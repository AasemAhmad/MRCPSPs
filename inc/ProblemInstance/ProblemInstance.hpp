#pragma once

#include "Job.hpp"
#include "Shared/Queue.hpp"

class ProblemInstance
{
  public:
    explicit ProblemInstance(const std::string &name);
    ~ProblemInstance() = default;
    ProblemInstance(const ProblemInstance &) = delete;
    ProblemInstance &operator=(const ProblemInstance &) = delete;

    void set_makespan_upperbound();
    void append_job(std::shared_ptr<Job> job);
    std::shared_ptr<Job> find_job(const std::string &job_id) const;
    void sort_jobs_by_id();
    void sort_jobs_by_release_time();

    Queue<Job> job_queue;
    std::vector<Resource> resources;
    std::string name;
    size_t makespan_upperbound = 0;
};
