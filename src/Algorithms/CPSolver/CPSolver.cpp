
#include "Algorithms/CPSolver/CPSolver.hpp"
#include "Settings.hpp"
#include "Shared/Exceptions.hpp"
#include "Shared/Utils.hpp"
#include "Solution/Solution.hpp"
#include <intervalset.hpp>
#include <loguru.hpp>
#include <math.h>
#include <memory>
#include <tuple>
#include <vector>

CPSolver::CPSolver(const ProblemInstance &problem_instance) : problem_instance(problem_instance) {}

Solution CPSolver::solve()
{
    Solution solution;
    IloEnv env;
    try
    {
        model = IloModel(env);
        tasks = IloIntervalVarArray(env, problem_instance.job_queue.nb_elements());
        modes = IloIntervalVarArray2(env, problem_instance.job_queue.nb_elements());
        ends = IloIntExprArray(env);
        starts = IloIntExprArray(env);

        size_t nb_resources = problem_instance.resources.size();

        processes = IloCumulFunctionExprArray(env, nb_resources);
        capacities = IloIntArray(env, nb_resources);

        initialize_resource_arrays();
        init_task_and_mode_arrays();
        add_capacity_constraints();
        add_precedence_constraints();
        add_objective_and_solve(solution);

    } catch (IloException &ex)
    {
        throw CustomException(std::source_location::current(), ex.getMessage());
    } catch (...)
    {
        throw_with_nested(CustomException(std::source_location::current(), "Error during solving the CP model!"));
    }
    env.end();
    return solution;
}

void CPSolver::initialize_resource_arrays()
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

void CPSolver::init_task_and_mode_arrays()
{
    IloEnv env = model.getEnv();

    size_t job_index = 0;
    for (const auto &job : problem_instance.job_queue)
    {
        IloIntervalVar task = add_job_start_time_constraints(job);

        tasks[job_index] = task;

        add_job_modes_constraints(job, job_index);

        model.add(IloAlternative(env, task, modes[job_index]));
        ends.add(IloEndOf(task));
        starts.add(IloStartOf(task));

        const auto &[it, emplaced] = id_index_map.try_emplace(job->id, job_index);
        PPK_ASSERT_ERROR(emplaced, "key with the given id was already emplaced");
        ++job_index;
    }
}

IloIntervalVar CPSolver::add_job_start_time_constraints(const JobPtr &job) const
{
    IloEnv env = model.getEnv();
    IloIntervalVar task(env);
    task.setStartMin(job->release_time);
    return task;
}

void CPSolver::add_job_modes_constraints(const JobPtr &job, size_t job_index)
{
    IloEnv env = model.getEnv();
    modes[job_index] = IloIntervalVarArray(env);

    IloIntervalVarArray job_modes(env);
    for (const auto &mode : job->modes)
    {
        IloInt proc_time = mode.processing_time;
        IloIntervalVar alt(env, proc_time);
        alt.setOptional();
        modes[job_index].add(alt);
    }
    model.add(IloAlternative(env, tasks[job_index], modes[job_index]));

    job_modes = modes[job_index];

    size_t mode_index = 0;
    for (const auto &mode : job->modes)
    {
        size_t res_index = 0;
        for (const auto &resource : mode.requested_resources)
        {
            IloInt res_units = resource.units;
            processes[res_index] += IloPulse(job_modes[mode_index], res_units);
            ++res_index;
        }
        ++mode_index;
    }
}

void CPSolver::add_precedence_constraints()
{
    IloEnv env = model.getEnv();

    for (const JobConstPtr &job : this->problem_instance.job_queue)
    {
        size_t job_index = id_index_map[job->id];
        for (const auto &succ : job->successors)
        {
            size_t succ_idex = id_index_map.at(succ);

            model.add(IloEndBeforeStart(env, tasks[job_index], tasks[succ_idex]));
        }
    }
}

void CPSolver::add_capacity_constraints()
{
    for (size_t i = 0; i < processes.getSize(); ++i)
    {
        model.add(processes[i] <= capacities[i]);
    }
}

void CPSolver::add_objective_and_solve(Solution &solution)
{
    IloEnv env = model.getEnv();
    IloObjective objective = IloMinimize(env, IloMax(ends));
    model.add(objective);

    IloCP cp(model);
    cp.setParameter(IloCP::TimeLimit, Settings::Solver::MAX_RUNTIME);
    cp.setParameter(IloCP::Workers, 1);

    cp.setParameter(IloCP::LogVerbosity, IloCP::Quiet);

    cp.setOut(env.getNullStream());
    cp.setWarning(env.getNullStream());
    cp.setError(env.getNullStream());

    if (cp.solve())
    {
        LOG_F(INFO, "model is solved");
        set_solution(cp, solution);
    } else
    {
        LOG_F(INFO, "CP is not solvable");
    }
    cp.end();
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
