/***********************************************
 * Author: Libur Bukata
 * Modified by: Aasem Ahmad
 ***********************************************/

#pragma once

#include "ILPSolverModel.hpp"
#include "Shared/Constants.hpp"
#include "Solution/Solution.hpp"
#include <iostream>
#include <string>
#include <vector>

struct SolutionILP
{
    double criterion;
    double bound;
    std::vector<double> solution;
    MODEL_STATUS status;
};

struct Solver
{
    virtual void initialize_local_environments(int nb_threads) const = 0;
    virtual std::string get_solver_identification() const = 0;
    virtual SolutionILP solve_ilp(Solution &init_solution, const ILPSolverModel &ilp_model, bool verbose,
                                  double gap = 0.0, double time_limit = 0.0, int nb_threads = 1,
                                  int thread_id = 0) const = 0;
};

struct GurobiSolver : public Solver
{
    void initialize_local_environments(int nb_threads) const override;
    std::string get_solver_identification() const override;
    SolutionILP solve_ilp(Solution &init_solution, const ILPSolverModel &ilp_model, bool verbose, double gap = 0.0,
                          double time_limit = 0.0, int nb_threads = 1, int thread_id = 0) const override;
};

struct CplexSolver : public Solver
{
    void initialize_local_environments(int nb_threads) const override;
    std::string get_solver_identification() const override;
    SolutionILP solve_ilp(Solution &init_solution, const ILPSolverModel &ilp_model, bool verbose, double gap = 0.0,
                          double time_limit = 0.0, int nb_threads = 1, int thread_id = 0) const override;
};