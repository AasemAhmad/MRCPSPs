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

    using map1to1 = std::map<std::string, size_t>;

  private:
    void initialize_resource_arrays();
    void init_task_and_mode_arrays();
    IloIntervalVar add_job_start_time_constraints(const JobPtr &job) const;
    void add_job_modes_constraints(const JobPtr &job, size_t job_index);
    void add_precedence_constraints();
    void add_capacity_constraints();
    void add_objective_and_solve(Solution &solution);
    void set_solution(IloCP &cp, Solution &solution);
    void set_solution_helper(IloCP &cp, Solution &solution);

    const ProblemInstance &problem_instance;
    map1to1 id_index_map;

    IloModel model;
    IloCumulFunctionExprArray processes;
    IloIntArray capacities;
    IloIntervalVarArray tasks;
    IloIntervalVarArray2 modes;
    IloIntExprArray ends;
    IloIntExprArray starts;
};
