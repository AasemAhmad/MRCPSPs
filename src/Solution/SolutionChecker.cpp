#include "Solution/SolutionChecker.hpp"
#include "External/pempek_assert.hpp"
#include "Shared/Utils.hpp"
#include "loguru.hpp"
#include <set>

bool SolutionChecker::check_solution() const
{
    PPK_ASSERT_ERROR(this->check_objective(), "Objective value is incorrect");
    PPK_ASSERT_ERROR(this->check_job_selected_processing_time(), "Jobs processing times are not correctly selected");
    PPK_ASSERT_ERROR(this->check_resource_usage_over_time_period(), "Capacity constraints are not correct");
    PPK_ASSERT_ERROR(this->check_resource_usage_over_intervals(), "Capacity constraints are not correct");
    return true;
}

bool SolutionChecker::check_objective() const
{
    size_t makespan = 0;

    for (const auto &job_allocation : this->solution.job_allocations)
    {
        makespan = std::max(makespan, job_allocation.start_time + job_allocation.duration);
    }
    return (makespan == this->solution.makespan);
}

bool SolutionChecker::check_job_selected_processing_time() const
{
    for (const auto &job_allocation : this->solution.job_allocations)
    {
        PPK_ASSERT_ERROR(job_allocation.mode_id > 0, "mode id must be greater than 0");
        size_t mode_index = job_allocation.mode_id - 1;
        std::string job_id = job_allocation.job_id;
        auto job = this->problem_instance.job_queue.get_element(job_id);
        PPK_ASSERT_ERROR(job != nullptr, "Job cannot be found");
        PPK_ASSERT_ERROR(mode_index < job->modes.size(), "Invalid Value %ld", mode_index);
        PPK_ASSERT_ERROR(job->modes.at(mode_index).processing_time == job_allocation.duration,
                         "selected processing time does not match with the selected mode");
    }
    return true;
}

bool SolutionChecker::check_resource_usage_at_given_time(std::vector<JobAllocation> &allocations, size_t time) const
{
    std::vector<size_t> consumed_capacity(this->problem_instance.resources.size(), 0);

    PPK_ASSERT_ERROR(allocations.size() > 0, "job allocation is empty");
    for (auto it = allocations.begin(); it != allocations.end();)
    {
        if ((it->start_time <= time) && (it->start_time + it->duration > time))
        {
            const auto &job = this->problem_instance.job_queue.get_element(it->job_id);
            PPK_ASSERT_ERROR(job != nullptr, "Job was not found");
            PPK_ASSERT_ERROR(it->mode_id > 0, "Invalid value %ld", it->mode_id);
            size_t mode_index = it->mode_id - 1;
            PPK_ASSERT_ERROR(mode_index < job->modes.size(), "Invalid value %ld", mode_index);
            for (size_t i = 0; i < this->problem_instance.resources.size(); ++i)
            {
                consumed_capacity[i] += job->modes.at(mode_index).requested_resources.at(i).units;
                PPK_ASSERT_ERROR(consumed_capacity[i] <= this->problem_instance.resources.at(i).units,
                                 "capacity constraint is invalid at resource %ld", i);
            }
        } else if (it->start_time + it->duration == time)
        {
            it = allocations.erase(it);
            continue;
        }
        ++it;
    }
    return true;
}

bool SolutionChecker::check_resource_usage_over_time_period() const
{
    PPK_ASSERT_ERROR(this->solution.job_allocations.size() == this->problem_instance.job_queue.nb_items(),
                     "at least one job is not allocated %ld, %ld", this->solution.job_allocations.size(),
                     this->problem_instance.job_queue.nb_items());

    std::vector<JobAllocation> allocations = sort_by_field(this->solution.job_allocations, &JobAllocation::start_time);

    for (size_t time = 0; time < this->solution.makespan; ++time)
    {
        if (!check_resource_usage_at_given_time(allocations, time))
        {
            return false;
        }
    }
    return true;
}

bool SolutionChecker::check_resource_usage_over_intervals() const
{
    PPK_ASSERT_ERROR(this->solution.job_allocations.size() == this->problem_instance.job_queue.nb_items(),
                     "at least one job is not allocated %ld, %ld", this->solution.job_allocations.size(),
                     this->problem_instance.job_queue.nb_items());

    std::vector<JobAllocation> allocations = sort_by_field(this->solution.job_allocations, &JobAllocation::start_time);

    std::set<size_t, std::less<>> resource_allocation_changed_time;
    for (const auto &allocation : this->solution.job_allocations)
    {
        resource_allocation_changed_time.emplace(allocation.start_time);
        resource_allocation_changed_time.emplace(allocation.start_time + allocation.duration);
    }

    for (const auto &time : resource_allocation_changed_time)
    {
        if (!check_resource_usage_at_given_time(allocations, time))
        {
            return false;
        }
    }
    return true;
}