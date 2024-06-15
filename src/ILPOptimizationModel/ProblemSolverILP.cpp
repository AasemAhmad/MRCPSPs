#include "ILPOptimizationModel/ProblemSolverILP.hpp"
#include "External/ILPSolverModel/ILPSolverInterface.hpp"
#include "Settings.hpp"
#include <chrono>
#include <cmath>
#include <memory>

ProblemSolverILP::ProblemSolverILP(const ProblemInstance &problem_instance) : variable_mapping_ilp(problem_instance)
{
    ConstraintModelBuilder constraint_model_builder(problem_instance);
    construct(constraint_model_builder);
}

Solution ProblemSolverILP::solve(Solution &init_solution, double rel_gap, double time_limit) const
{

    Solution solution;
    SolutionILP solution_ilp;
    std::unique_ptr<Solver> solver = nullptr;
    PPK_ASSERT_ERROR(Settings::Solver::USE_GUROBI || Settings::Solver::USE_CPLEX, "Solver was not selected");

  
    if (Settings::Solver::USE_GUROBI)
    {
        solver = std::make_unique<GurobiSolver>();
        solver->initialize_local_environments(1u);
    } else if (Settings::Solver::USE_CPLEX)
    {
        solver = std::make_unique<CplexSolver>();
    }

    PPK_ASSERT_ERROR(solver, "Failed to create solver");

    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

    solution_ilp = solver->solve_ilp(init_solution, ilp_model, Settings::Solver::VERBOSE, rel_gap, time_limit,
                                     Settings::Solver::NB_THREADS, 0);

    using chrono_clk = std::chrono::high_resolution_clock;
    solution.runtime = duration_cast<std::chrono::duration<double>>(chrono_clk::now() - start).count();
    solution.solution_state = convert(solution_ilp.status);

    if (solution_ilp.status == MODEL_STATUS::MODEL_SOL_OPTIMAL ||
        solution_ilp.status == MODEL_STATUS::MODEL_SOL_FEASIBLE)
    {
        try
        {
            solution.makespan = static_cast<size_t>(std::round(solution_ilp.criterion));
            auto job_durations = lookup(solution_ilp.solution, variable_mapping_ilp.p);
            auto job_start_times = lookup(solution_ilp.solution, variable_mapping_ilp.s);
            auto job_modes = lookup(solution_ilp.solution, variable_mapping_ilp.x);

            for (const auto &[key, value] : job_start_times)
            {
                JobAllocation job_allocation;
                std::string job_id = key;
                job_allocation.job_id = job_id;
                job_allocation.start_time = job_start_times.at(job_id);
                job_allocation.duration = job_durations.at(job_id);
                job_allocation.mode_id = job_modes.at(job_id);

                solution.job_allocations.emplace_back(std::move(job_allocation));
            }
        } catch (...)
        {
            // TODO
            // PPK_ASSERT_ERROR
        }
    } else if (solution_ilp.status != MODEL_STATUS::MODEL_SOL_UNKNOWN)
    {}
    return solution;
}

void ProblemSolverILP::construct(ConstraintModelBuilder &constraint_model_builder)
{
    constraint_model_builder.add_job_processing_time_constraints(variable_mapping_ilp.x, variable_mapping_ilp.p);

    constraint_model_builder.add_job_start_time_constraints(variable_mapping_ilp.s, variable_mapping_ilp.x,
                                                            variable_mapping_ilp.p, variable_mapping_ilp.c_max);

    constraint_model_builder.add_precedence_constraints(variable_mapping_ilp.s, variable_mapping_ilp.p);

    constraint_model_builder.add_renewable_resource_constraints(variable_mapping_ilp.x);

    ilp_model.vector_c.resize(variable_mapping_ilp.get_nb_variables(), 0.0);
    ilp_model.vector_c[0] = 1;

    ilp_model.vector_x = move(variable_mapping_ilp.variables);
    ilp_model.varDesc = move(variable_mapping_ilp.var_desc);
    constraint_model_builder.move_constraints_to_model(ilp_model);

    if (Settings::Solver::VERBOSE == true)
    {
        ilp_model.log_statistics();
    }
}
