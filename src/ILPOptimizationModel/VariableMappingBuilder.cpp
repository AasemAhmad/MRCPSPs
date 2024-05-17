#include "ILPOptimizationModel/VariableMappingBuilder.hpp"
#include "External/pempek_assert.hpp"
#include "ProblemInstance/ProblemInstance.hpp"
#include "Shared/Queue.hpp"
#include "Shared/Utils.hpp"
#include <cmath>
#include <format>

size_t calcualte_processing_time_upperbound(const std::shared_ptr<Job> &job)
{
    auto upper_bound_iterator = std::ranges::max_element(
        job->modes, [](const Mode &a, const Mode &b) { return a.processing_time < b.processing_time; });

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
    this->add_task_resources_allocation_variables();
}

size_t TimeIndexedModelVariableMapping::get_nb_variables() const
{
    return (c_max.size() + x.size() + p.size() + s.size() + y.size());
}

void TimeIndexedModelVariableMapping::add_objective_function_variables()
{
    const std::source_location loc = std::source_location::current();

    size_t idx = get_nb_variables();
    variables.emplace_back(DecisionVariableType::INT, 0, problem_instance.makespan_upperbound);
    var_desc.emplace_back(std::format("cMax_{}", idx));
    set_value(c_max, std::to_string(1), idx, loc);
    ++idx;
}

void TimeIndexedModelVariableMapping::add_task_processing_time_variables()
{
    const std::source_location loc = std::source_location::current();
    size_t idx = get_nb_variables();

    for (const auto &job : this->problem_instance.job_queue)
    {
        DecisionVariable var(DecisionVariableType::INT, 0,
                             static_cast<double>(calcualte_processing_time_upperbound(job)));
        variables.push_back(var);
        var_desc.emplace_back("p_{" + job->j_id + "}");
        set_value(p, job->j_id, idx, loc);
        ++idx;
    }
}

void TimeIndexedModelVariableMapping::add_task_startime_variable()
{
    const std::source_location loc = std::source_location::current();

    size_t idx = get_nb_variables();
    for (const auto &job : this->problem_instance.job_queue)
    {
        DecisionVariable var(DecisionVariableType::INT, 0, static_cast<double>(problem_instance.makespan_upperbound));
        variables.push_back(var);
        var_desc.emplace_back("s_{" + job->j_id + "}");
        set_value(s, {job->j_id}, idx, loc);
        ++idx;
    }
}

void TimeIndexedModelVariableMapping::add_task_startime_binary_variables()
{
    const std::source_location loc = std::source_location::current();

    size_t idx = get_nb_variables();

    for (const auto &job : this->problem_instance.job_queue)
    {
        for (size_t t = 0; t < problem_instance.makespan_upperbound; ++t)
        {
            size_t mode_id = 1;
            for ([[maybe_unused]] const auto &_ : job->modes)
            {
                PPK_ASSERT_ERROR(mode_id <= job->modes.size(), "Invalid value %ld", mode_id);
                DecisionVariable var(DecisionVariableType::BIN, 0.0, 1.0);
                variables.push_back(var);
                var_desc.emplace_back(std::format("x_{{{}#{}#{}}}", job->j_id, mode_id, t));
                set_value(x, {job->j_id, std::to_string(mode_id), std::to_string(t)}, idx, loc);
                ++idx;
                ++mode_id;
            }
        }
    }
}

void TimeIndexedModelVariableMapping::add_task_resources_allocation_variables()
{
    // TODO if necessary
}

TimeIndexedModelVariableMapping::map1to1 lookup(const std::vector<double> &solution,
                                                const TimeIndexedModelVariableMapping::map1to1 &mapping)
{
    const std::source_location loc = std::source_location::current();

    TimeIndexedModelVariableMapping::map1to1 my_map;
    for (const auto &[key, value] : mapping)
    {
        set_value(my_map, key, solution[value], loc);
    }
    return my_map;
}

TimeIndexedModelVariableMapping::map2to1 lookup(const std::vector<double> &solution,
                                                const TimeIndexedModelVariableMapping::map2to1 &mapping)
{
    const std::source_location loc = std::source_location::current();

    TimeIndexedModelVariableMapping::map2to1 my_map;
    for (const auto &[key, value] : mapping)
    {
        set_value(my_map, {std::get<0>(key), std::get<1>(key)}, solution[value], loc);
    }
    return my_map;
}

TimeIndexedModelVariableMapping::map1to1 lookup(const std::vector<double> &solution,
                                                const TimeIndexedModelVariableMapping::map3to1 &mapping)
{
    const std::source_location loc = std::source_location::current();

    TimeIndexedModelVariableMapping::map1to1 my_map;
    for (const auto &[key, value] : mapping)
    {
        if (solution[value] > 0)
        {
            set_value(my_map, std::get<0>(key), std::stoi(std::get<1>(key)), loc);
        }
    }
    return my_map;
}
