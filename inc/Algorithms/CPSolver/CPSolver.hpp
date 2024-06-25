#pragma once

#include "ProblemInstance/ProblemInstance.hpp"
#include "Solution/Solution.hpp"
#include <ilcp/cp.h>

class CPSolver
{
  public:
    explicit CPSolver(const ProblemInstance &);
    CPSolver(const CPSolver &) = delete;
    CPSolver &operator=(const CPSolver &) = delete;

    Solution solve();

  private:
    void init_resource_arrays(IloModel &);
    void add_job_start_time_constraints(IloModel &);
    void add_job_modes_constraints(IloModel &);
    void add_precedence_constraints(IloModel &);
    void add_capacity_constraints(IloModel &);
    bool solve_cp_model(IloCP &cp, IloModel &, Solution &solution);
    void set_solution(IloCP &cp, Solution &solution);
    void set_solution_helper(IloCP &cp, Solution &solution);
    void add_objective(IloModel &);

    const ProblemInstance &problem_instance;
    IloCumulFunctionExprArray processes;
    IloIntArray capacities;
    IloIntervalVarArray tasks;
    IloIntervalVarArray2 modes;
    IloIntExprArray ends;
    IloIntExprArray starts;
};
