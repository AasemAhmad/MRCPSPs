#include "ProblemInstance/Job.hpp"

Job::Job(std::string job_id, double release_time, const std::vector<Mode> &modes)
    : j_id(job_id), release_time(release_time), modes(modes)
{}

std::string Job::get_id() const { return j_id; }

bool Job::compare_by_job_id(const std::shared_ptr<Job> &j1, const std::shared_ptr<Job> &j2)
{
    return j1->j_id < j2->j_id;
}

bool Job::compare_by_release_time(const std::shared_ptr<Job> &j1, const std::shared_ptr<Job> &j2)
{
    return j1->j_id < j2->j_id;
}