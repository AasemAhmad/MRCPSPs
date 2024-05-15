/***********************************************
 * Author: Libur Bukata
 * Modified by: Aasem Ahmad
 ***********************************************/

#include "External/ILPSolverModel/ILPSolverInterface.hpp"
#include <ilcplex/ilocplex.h>

void generate_problem_cplex(Solution &init_solution, const ILPSolverModel &ilp_model, IloModel &model,
                            IloNumVarArray &x, IloRangeArray &con)
{
    IloEnv env = model.getEnv();
    IloObjective obj = (ilp_model.obj == ObjectiveFunction::MINIMIZE ? IloMinimize(env) : IloMaximize(env));
    size_t var_index = 0;

    for (const auto &var : ilp_model.vector_x)
    {
        switch (var.type)
        {
        case DecisionVariableType::FLT:
            x.add(IloNumVar(env, var.lower_bound, var.upper_bound, IloNumVar::Float));
            break;
        case DecisionVariableType::BIN:
            x.add(IloNumVar(env, var.lower_bound, var.upper_bound, IloNumVar::Bool));
            break;
        default:
            x.add(IloNumVar(env, var.lower_bound, var.upper_bound, IloNumVar::Int));
        }
        obj.setLinearCoef(x[var_index], ilp_model.vector_c[var_index]);
        x[var_index].setName(ilp_model.varDesc[var_index].c_str());
        ++var_index;
    }

    size_t constraint_index = 0;
    for (const auto &op : ilp_model.vector_op)
    {
        switch (op)
        {
        case Operator::LESS_EQUAL:
            con.add(IloRange(env, -IloInfinity, ilp_model.vector_b[constraint_index]));
            break;
        case Operator::EQUAL:
            con.add(IloRange(env, ilp_model.vector_b[constraint_index], ilp_model.vector_b[constraint_index]));
            break;
        case Operator::GREATER_EQUAL:
            con.add(IloRange(env, ilp_model.vector_b[constraint_index], IloInfinity));
        }

        for (const auto &element : ilp_model.matrix_A[constraint_index])
        {
            con[constraint_index].setLinearCoef(x[element.first], element.second);
        }

        con[constraint_index].setName(ilp_model.conDesc[constraint_index].c_str());
        ++constraint_index;
    }

    model.add(obj);
    model.add(con);
}

std::string CplexSolver::get_solver_identification() const
{
    IloEnv env;
    IloCplex cplex(env);
    std::string identification = "Cplex " + std::string(cplex.getVersion());
    env.end();
    return identification;
}

void CplexSolver::initialize_local_environments(int nb_threads) const
{
    // TODO
}

SolutionILP CplexSolver::solve_ilp(Solution &init_solution, const ILPSolverModel &ilp_model, bool verbose, double gap,
                                   double time_limit, int nb_threads, int) const
{
    IloEnv env;
    SolutionILP solution_ilp;

    try
    {
        IloModel model(env);
        IloObjective obj(env);
        IloNumVarArray var(env);
        IloRangeArray con(env);
        generate_problem_cplex(init_solution, ilp_model, model, var, con);

        IloCplex cplex(model);
        cplex.setParam(IloCplex::Param::Threads, nb_threads);
        if (!verbose)
            cplex.setOut(env.getNullStream());
        if (gap != 0.0)
            cplex.setParam(IloCplex::Param::MIP::Tolerances::MIPGap, gap);
        if (time_limit != 0.0)
            cplex.setParam(IloCplex::Param::TimeLimit, time_limit);

        cplex.solve();

        switch (cplex.getStatus())
        {
        case IloAlgorithm::Optimal:
            solution_ilp.status = MODEL_STATUS::MODEL_SOL_OPTIMAL;
            break;
        case IloAlgorithm::Feasible:
            solution_ilp.status = MODEL_STATUS::MODEL_SOL_FEASIBLE;
            break;
        case IloAlgorithm::Infeasible:
            solution_ilp.status = MODEL_STATUS::MODEL_SOL_INFEASIBLE;
            break;
        case IloAlgorithm::Unbounded:
            solution_ilp.status = MODEL_STATUS::MODEL_SOL_UNBOUNDED;
            break;
        default:
            solution_ilp.status = MODEL_STATUS::MODEL_SOL_UNKNOWN;
        }

        if (solution_ilp.status == MODEL_STATUS::MODEL_SOL_OPTIMAL ||
            solution_ilp.status == MODEL_STATUS::MODEL_SOL_FEASIBLE)
        {
            IloNumArray sol(env);
            cplex.getValues(sol, var);
            solution_ilp.criterion = cplex.getObjValue();
            for (long v = 0; v < sol.getSize(); ++v)
                solution_ilp.solution.push_back(sol[v]);
        }

        solution_ilp.bound = cplex.getBestObjValue();
        env.end();

    } catch (IloException &e)
    {
        env.end();
        // FIXME
    } catch (...)
    {
        env.end();
        // FIXME
    }

    return solution_ilp;
}