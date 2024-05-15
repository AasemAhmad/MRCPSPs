#include "ILPOptimizationModel/VariableMappingBuilder.hpp"
#include "External/pempek_assert.hpp"
#include "ProblemInstance/ProblemInstance.hpp"
#include "Shared/Queue.hpp"
#include "Shared/Utils.hpp"
#include <cmath>

size_t calcualte_processing_time_upperbound(const std::shared_ptr<Job> &job)
{
    auto upper_bound_iterator =
        std::max_element(job->modes.begin(), job->modes.end(),
                         [](const Mode &a, const Mode &b) { return a.processing_time > b.processing_time; });

    PPK_ASSERT_ERROR(upper_bound_iterator != job->modes.end(), "Error while calcualating the upper bound");

    return upper_bound_iterator->processing_time;
}

TimeIndexedModelVariableMapping::TimeIndexedModelVariableMapping(const ProblemInstance &problem_instance)
    : problem_instance(problem_instance)
{
    PPK_ASSERT_ERROR(problem_instance.makespan_upperbound > 0, "Makespan upperbound is 0");
    this->add_objective_function_variables();
    this->add_task_startime_variable();
    this->add_task_processing_time_variables();
    this->add_task_startime_binary_variables();
    this->add_task_computer_resources_allocation_variables();
}

size_t TimeIndexedModelVariableMapping::get_nb_variables() const
{
    return (c_max.size() + x.size() + p.size() + s.size() + y.size());
}

void TimeIndexedModelVariableMapping::add_objective_function_variables()
{
    size_t idx = get_nb_variables();
    variables.emplace_back(DecisionVariableType::INT, 0, problem_instance.makespan_upperbound);
    var_desc.emplace_back("cMax index = " + std::to_string(idx));
    set_value(c_max, std::to_string(1), idx++, __FUNCTION__);
}

void TimeIndexedModelVariableMapping::add_task_processing_time_variables()
{
    size_t idx = get_nb_variables();

    for (auto it = this->problem_instance.job_queue.cbegin(); it != this->problem_instance.job_queue.cend(); ++it)
    {
        DecisionVariable var(DecisionVariableType::INT, 0, calcualte_processing_time_upperbound(*it));
        variables.push_back(var);
        var_desc.emplace_back("p_{" + (*it)->j_id + "}");
        set_value(p, (*it)->j_id, idx++, __FUNCTION__);
    }
}

void TimeIndexedModelVariableMapping::add_task_startime_variable()
{
    size_t idx = get_nb_variables();
    for (auto it = this->problem_instance.job_queue.cbegin(); it != this->problem_instance.job_queue.cend(); ++it)
    {
        DecisionVariable var(DecisionVariableType::INT, 0, problem_instance.makespan_upperbound);
        variables.push_back(var);
        var_desc.emplace_back("s_{" + (*it)->j_id + "}");
        set_value(s, {(*it)->j_id}, idx++, __FUNCTION__);
    }
}

void TimeIndexedModelVariableMapping::add_task_startime_binary_variables()
{
    size_t idx = get_nb_variables();

    for (auto it = this->problem_instance.job_queue.cbegin(); it != this->problem_instance.job_queue.cend(); ++it)
    {
        for (size_t t = 0; t < problem_instance.makespan_upperbound; ++t)
        {
            size_t mode_id = 1;
            for (const auto &mode : (*it)->modes)
            {
                PPK_ASSERT_ERROR(mode_id <= (*it)->modes.size(), "Invalid value %ld", mode_id);
                DecisionVariable var(DecisionVariableType::BIN, 0.0, 1.0);
                variables.push_back(var);
                var_desc.emplace_back("x_{" + (*it)->j_id + "}#{" + std::to_string(mode_id) + "}#{" +
                                      std::to_string(t) + "}");
                set_value(x, {(*it)->j_id, std::to_string(mode_id), std::to_string(t)}, idx++, __FUNCTION__);
                ++mode_id;
            }
        }
    }
}

void TimeIndexedModelVariableMapping::add_task_computer_resources_allocation_variables()
{
    // TODO if necessary
}

TimeIndexedModelVariableMapping::map1to1 lookup(const std::vector<double> &solution,
                                                const TimeIndexedModelVariableMapping::map1to1 &mapping)
{
    TimeIndexedModelVariableMapping::map1to1 my_map;
    for (const auto &element : mapping)
    {
        set_value(my_map, element.first, solution[element.second], __FUNCTION__);
    }
    return my_map;
}

TimeIndexedModelVariableMapping::map2to1 lookup(const std::vector<double> &solution,
                                                const TimeIndexedModelVariableMapping::map2to1 &mapping)
{
    TimeIndexedModelVariableMapping::map2to1 my_map;
    for (const auto &element : mapping)
    {
        set_value(my_map, {std::get<0>(element.first), std::get<1>(element.first)}, solution[element.second],
                  __FUNCTION__);
    }
    return my_map;
}

TimeIndexedModelVariableMapping::map1to1 lookup(const std::vector<double> &solution,
                                                const TimeIndexedModelVariableMapping::map3to1 &mapping)
{
    TimeIndexedModelVariableMapping::map1to1 my_map;
    for (const auto &element : mapping)
    {
        if (solution[element.second] > 0)
        {
            set_value(my_map, std::get<0>(element.first), std::stoi(std::get<1>(element.first)), __FUNCTION__);
        }
    }
    return my_map;
}
