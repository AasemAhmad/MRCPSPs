#include "ProblemInstance/ProblemInstance.hpp"
#include "ProblemInstance/Job.hpp"
#include "Shared/Utils.hpp"
#include <queue>
#include <unordered_set>

ProblemInstance::ProblemInstance(const std::string &name) : name(name) {}

void ProblemInstance::set_makespan_upperbound()
{
    makespan_upper_bound = 0;
    std::ranges::for_each(job_queue, [this](const JobConstPtr &job) {
        auto max_value = std::ranges::max(job->modes, std::ranges::less{}, &Mode::processing_time);
        makespan_upper_bound += max_value.processing_time;
    });
}
const std::vector<Resource> &ProblemInstance::get_resources() const { return this->resources; }
const std::string &ProblemInstance::get_name() const { return this->name; }

void ProblemInstance::append_job(const JobPtr &job) { job_queue.append_element(job); }

JobConstPtr ProblemInstance::find_job(const std::string &job_id) const { return job_queue.get_element(job_id); }

void ProblemInstance::sort_jobs_by_id() { job_queue.sort_queue(compare_by_job_id); }

void ProblemInstance::sort_jobs_by_release_time() { job_queue.sort_queue(compare_by_release_time); }

bool ProblemInstance::validate_problem_instance() const
{
    this->validate_dependencies();
    this->validate_job_modes();
    return true;
}

bool ProblemInstance::validate_dependencies() const
{
    return std::ranges::all_of(this->job_queue,
                               [this](const JobConstPtr &job) { return validate_job_dependencies(job); });
}

bool ProblemInstance::validate_job_dependencies(const JobConstPtr &job) const
{
    std::queue<std::string> reachable_job_queue;
    std::unordered_set<std::string, StringHash, std::equal_to<>> visited_jobs;
    for (const auto &successor_id : job->successors)
    {
        reachable_job_queue.emplace(successor_id);
    }

    while (!reachable_job_queue.empty())
    {
        const std::string job_id = reachable_job_queue.front();
        reachable_job_queue.pop();

        PPK_ASSERT_ERROR(job_id != job->id, "Invalid dependencies: cycle detected involving job '%s'.",
                         job->id.c_str());

        const auto &[iterator, emplaced] = visited_jobs.emplace(job_id);

        if (!emplaced)
        {
            continue;
        }

        const JobConstPtr &successor_job = this->find_job(job_id);
        PPK_ASSERT_ERROR(successor_job, "Job was not found");

        for (const auto &next_successor_id : successor_job->successors)
        {
            reachable_job_queue.emplace(next_successor_id);
        }
    }

    return true;
}

bool ProblemInstance::validate_job_modes() const
{
    for (const JobConstPtr &job : this->job_queue)
    {
        for (const auto &mode : job->modes)
        {
            size_t resource_index = 0;
            const auto &requested_resources = mode.requested_resources;

            for (const auto &nb_res : requested_resources)
            {
                PPK_ASSERT_ERROR(nb_res.units <= this->resources.at(resource_index).units,
                                 "Invalid number of requested resources");
                ++resource_index;
            }
        }
    }
    return true;
}
