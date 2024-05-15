#pragma once

#include "External/ILPSolverModel/ILPSolverModel.hpp"
#include "ILPOptimizationModel/VariableMappingBuilder.hpp"
#include "ProblemInstance/ProblemInstance.hpp"
#include <utility>
#include <vector>

class ConstraintModelBuilder
{
  public:
    ConstraintModelBuilder(const ProblemInstance &problem_instance)
        : constraints_counter(0), problem_instance(problem_instance)
    {}
    ConstraintModelBuilder(const ConstraintModelBuilder &) = delete;
    ConstraintModelBuilder &operator=(ConstraintModelBuilder &&) = delete;

    void reset();

    size_t move_constraints_to_model(ILPSolverModel &ilp_model);

    void add_job_proccessing_time_constraints(const TimeIndexedModelVariableMapping::map3to1 &x,
                                              const TimeIndexedModelVariableMapping::map1to1 &p);

    void add_job_start_time_constraints(const TimeIndexedModelVariableMapping::map1to1 &s,
                                        const TimeIndexedModelVariableMapping::map3to1 &x,
                                        const TimeIndexedModelVariableMapping::map1to1 &p,
                                        const TimeIndexedModelVariableMapping::map1to1 &cMax);

    void add_precedence_constraints(const TimeIndexedModelVariableMapping::map1to1 &s,
                                    const TimeIndexedModelVariableMapping::map1to1 &p);

    void add_renewable_resource_constraint(const TimeIndexedModelVariableMapping::map3to1 &x);

    void add_constraint(SparseMatrix<double>::Row &row, Operator op, const double &b, const std::string &conDesc);

  private:
    size_t constraints_counter;
    const ProblemInstance &problem_instance;
    SparseMatrix<double> constraint_matrix;
    std::vector<Operator> constraint_operator;
    std::vector<double> constraint_bound;
    std::vector<std::string> constraint_description;
};
