#include "ProblemInstance/Job.hpp"

bool compare_by_job_id(const std::shared_ptr<Job> &j1, const std::shared_ptr<Job> &j2) { return j1->j_id < j2->j_id; }

bool compare_by_release_time(const std::shared_ptr<Job> &j1, const std::shared_ptr<Job> &j2)
{
    return j1->j_id < j2->j_id;
}