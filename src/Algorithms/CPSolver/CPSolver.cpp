
#include "Algorithms/CPSolver/CPSolver.hpp"
#include "Settings.hpp"
#include "Shared/Exceptions.hpp"
#include "Shared/Utils.hpp"
#include "Solution/Solution.hpp"
#include "loguru.hpp"
#include <memory>
#include <vector>

CPSolver::CPSolver(const ProblemInstance &problem_instance) : problem_instance(problem_instance) {}

Solution CPSolver::solve()
{
    Solution solution;
    IloEnv env;
    try
    {
        IloModel model(env);
        tasks = IloIntervalVarArray(env, problem_instance.job_queue.nb_elements());
        modes = IloIntervalVarArray2(env, problem_instance.job_queue.nb_elements());
        ends = IloIntExprArray(env);
        starts = IloIntExprArray(env);

        size_t nb_resources = problem_instance.resources.size();

        processes = IloCumulFunctionExprArray(env, nb_resources);
        capacities = IloIntArray(env, nb_resources);

        init_resource_arrays(model);
        add_job_start_time_constraints(model);
        add_job_modes_constraints(model);
        add_capacity_constraints(model);
        add_precedence_constraints(model);
        add_objective(model);

        IloCP cp(model);

        if (bool solved = solve_cp_model(cp, model, solution); solved)
        {
            set_solution(cp, solution);
        } else
        {
            LOG_F(INFO, "CP Model is not solvable within the time limit of %g", Settings::Solver::MAX_RUNTIME);
        }
        cp.end();

    } catch (IloException &ex)
    {
        env.end();
        throw CustomException(std::source_location::current(), ex.getMessage());
    } catch (...)
    {
        env.end();
        throw_with_nested(CustomException(std::source_location::current(), "Error during solving the CP model!"));
    }

    env.end();
    return solution;
}

void CPSolver::init_resource_arrays(IloModel &model)
{
    IloEnv env = model.getEnv();

    size_t resource_index = 0;
    for (const auto &resource : problem_instance.resources)
    {
        processes[resource_index] = IloCumulFunctionExpr(env);
        capacities[resource_index] = resource.units;
        ++resource_index;
    }
}

void CPSolver::add_job_start_time_constraints(IloModel &model)
{
    IloEnv env = model.getEnv();

    size_t job_index = 0;
    for (const JobConstPtr &job : problem_instance.job_queue)
    {
        IloIntervalVar task(env);
        task.setStartMin(job->release_time);
        tasks[job_index] = task;
        starts.add(IloStartOf(tasks[job_index]));
        ends.add(IloEndOf(tasks[job_index]));
        ++job_index;
    }
}

void CPSolver::add_job_modes_constraints(IloModel &model)
{
    IloEnv env = model.getEnv();

    size_t job_index = 0;
    for (const JobConstPtr &job : problem_instance.job_queue)
    {
        modes[job_index] = IloIntervalVarArray(env);

        for (const auto &mode : job->modes)
        {
            IloInt proc_time = mode.processing_time;
            IloIntervalVar alt(env, proc_time);
            alt.setOptional();
            modes[job_index].add(alt);
        }

        size_t mode_index = 0;
        for (const auto &mode : job->modes)
        {
            size_t res_index = 0;
            for (const auto &resource : mode.requested_resources)
            {
                IloInt res_units = resource.units;
                processes[res_index] += IloPulse(modes[job_index][mode_index], res_units);
                ++res_index;
            }
            ++mode_index;
        }

        model.add(IloAlternative(env, tasks[job_index], modes[job_index]));
        ++job_index;
    }
}

void CPSolver::add_precedence_constraints(IloModel &model)
{
    IloEnv env = model.getEnv();

    size_t job_index = 0;
    for (const JobConstPtr &job : this->problem_instance.job_queue)
    {
        for (const auto &succ : job->successors)
        {
            const auto &succ_job = problem_instance.find_job(succ);
            PPK_ASSERT_ERROR(succ_job, "successor job %s was not found", succ.c_str());
            auto it = std::ranges::find(this->problem_instance.job_queue, succ_job);
            PPK_ASSERT_ERROR(it != this->problem_instance.job_queue.end(), " Invalid Iterator");
            size_t succ_index = std::distance(this->problem_instance.job_queue.begin(), it);
            model.add(IloEndBeforeStart(env, tasks[job_index], tasks[succ_index]));
        }
        ++job_index;
    }
}

void CPSolver::add_capacity_constraints(IloModel &model)
{
    for (size_t i = 0; i < processes.getSize(); ++i)
    {
        model.add(processes[i] <= capacities[i]);
    }
}

void CPSolver::add_objective(IloModel &model)
{
    IloEnv env = model.getEnv();

    IloObjective objective = IloMinimize(env, IloMax(ends));
    model.add(objective);
}

bool CPSolver::solve_cp_model(IloCP &cp, IloModel &model, Solution &solution)
{
    IloEnv env = model.getEnv();

    cp.setParameter(IloCP::TimeLimit, Settings::Solver::MAX_RUNTIME);
    cp.setParameter(IloCP::Workers, 1);

    cp.setParameter(IloCP::LogVerbosity, IloCP::Quiet);

    cp.setOut(env.getNullStream());
    cp.setWarning(env.getNullStream());
    cp.setError(env.getNullStream());

    return cp.solve();
}

void CPSolver::set_solution(IloCP &cp, Solution &solution)
{
    switch (cp.getStatus())
    {
        using enum MODEL_STATUS;
    case IloAlgorithm::Optimal:
        solution.solution_state = convert(MODEL_SOL_OPTIMAL);
        break;
    case IloAlgorithm::Feasible:
        solution.solution_state = convert(MODEL_SOL_FEASIBLE);
        break;
    case IloAlgorithm::Infeasible:
        solution.solution_state = convert(MODEL_SOL_INFEASIBLE);
        break;
    case IloAlgorithm::Unknown:
        solution.solution_state = convert(MODEL_SOL_UNKNOWN);
        break;
    case IloAlgorithm::Unbounded:
        solution.solution_state = convert(MODEL_SOL_UNBOUNDED);
        break;
    }

    if (cp.getStatus() == IloAlgorithm::Optimal || cp.getStatus() == IloAlgorithm::Feasible)
    {
        set_solution_helper(cp, solution);
    }
}

void CPSolver::set_solution_helper(IloCP &cp, Solution &solution)
{
    solution.makespan = cp.getObjValue();
    solution.gap = cp.getObjGap();
    solution.objective_bound = cp.getObjBound();
    solution.runtime = cp.getInfo(IloCP::TotalTime);

    size_t job_index = 0;
    for (const auto &job : problem_instance.job_queue)
    {
        JobAllocation job_allocation;
        job_allocation.job_id = job->id;
        job_allocation.start_time = cp.getStartMax(tasks[job_index]);
        job_allocation.duration = static_cast<size_t>(cp.getValue(ends[job_index]) - cp.getValue(starts[job_index]));

        for (size_t mode_index = 0; mode_index < modes[job_index].getSize(); ++mode_index)
        {
            if (cp.isPresent(modes[job_index][mode_index]))
            {
                job_allocation.mode_id = mode_index + 1;
                break;
            }
        }

        solution.job_allocations.emplace_back(std::move(job_allocation));
        ++job_index;
    }

    LOG_F(INFO, "Solution details updated successfully");
}
