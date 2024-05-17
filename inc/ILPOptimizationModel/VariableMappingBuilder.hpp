#pragma once

#include "External/ILPSolverModel/ILPSolverModel.hpp"
#include "ProblemInstance/ProblemInstance.hpp"
#include <map>

class TimeIndexedModelVariableMapping
{
  public:
    explicit TimeIndexedModelVariableMapping(const ProblemInstance &problem_instance);
    TimeIndexedModelVariableMapping(const TimeIndexedModelVariableMapping &) = delete;
    TimeIndexedModelVariableMapping &operator=(const TimeIndexedModelVariableMapping &) = delete;

    size_t get_nb_variables() const;

    using map1to1 = std::map<std::string, size_t, std::less<>>;
    using map2to1 = std::map<std::tuple<std::string, std::string>, size_t>;
    using map3to1 = std::map<std::tuple<std::string, std::string, std::string>, size_t>;

    map1to1 c_max;
    map3to1 x;
    map1to1 p;
    map1to1 s;
    map2to1 y;

    std::vector<DecisionVariable> variables;
    std::vector<std::string> var_desc;

  private:
    const ProblemInstance &problem_instance;
    void add_objective_function_variables();
    void add_task_startime_binary_variables();
    void add_task_processing_time_variables();
    void add_task_startime_variable();
    void add_task_resources_allocation_variables();
};

TimeIndexedModelVariableMapping::map1to1 lookup(const std::vector<double> &solution,
                                                const TimeIndexedModelVariableMapping::map1to1 &mapping);
TimeIndexedModelVariableMapping::map2to1 lookup(const std::vector<double> &solution,
                                                const TimeIndexedModelVariableMapping::map2to1 &mapping);
TimeIndexedModelVariableMapping::map1to1 lookup(const std::vector<double> &solution,
                                                const TimeIndexedModelVariableMapping::map3to1 &mapping);
