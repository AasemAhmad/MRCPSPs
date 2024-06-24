#pragma once

#include "Job.hpp"
#include "Shared/Queue.hpp"

class ProblemInstance
{
  public:
    explicit ProblemInstance(const std::string &name);
    ProblemInstance(const ProblemInstance &) = delete;
    ProblemInstance &operator=(const ProblemInstance &) = delete;

    void set_makespan_upperbound();
    void append_job(const JobPtr &job);
    JobConstPtr find_job(const std::string &job_id) const;
    const std::vector<Resource> &get_resources() const;
    const std::string &get_name() const;
    void sort_jobs_by_id();
    void sort_jobs_by_release_time();
    bool validate_problem_instance() const;

  private:
    bool validate_dependencies() const;
    bool validate_job_modes() const;
    bool validate_job_dependencies(const JobConstPtr &job) const;

    std::string name;
    size_t makespan_upper_bound = 0;
    std::vector<Resource> resources;
    Queue<Job> job_queue;

    friend class InstanceReader;
    friend class Solution;
    friend class SolutionChecker;
    friend class ConstraintModelBuilder;
    friend class VariableModelBuilder;
    friend class TimeIndexedModelVariableMapping;
    friend class CPSolver;
};
