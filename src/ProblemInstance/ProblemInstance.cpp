#include "ProblemInstance/ProblemInstance.hpp"

ProblemInstance::ProblemInstance(const std::string &name) : name(name) {}

void ProblemInstance::set_makespan_upperbound()
{
    makespan_upperbound = 0;
    std::ranges::for_each(job_queue, [this](const auto &job) {
        auto max_value = std::ranges::max(job->modes, {}, &Mode::processing_time);
        makespan_upperbound += max_value.processing_time;
    });
}

void ProblemInstance::append_job(std::shared_ptr<Job> job) { job_queue.append_item(job); }

std::shared_ptr<Job> ProblemInstance::find_job(const std::string &job_id) const { return job_queue.find_item(job_id); }

void ProblemInstance::sort_jobs_by_id() { job_queue.sort_queue(Job::compare_by_job_id); }

void ProblemInstance::sort_jobs_by_release_time() { job_queue.sort_queue(Job::compare_by_release_time); }
