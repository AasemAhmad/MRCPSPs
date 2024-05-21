#pragma once

#include "ProblemInstance/ProblemInstance.hpp"
#include "Solution/Solution.hpp"
#include <algorithm>
#include <vector>

class SolutionChecker
{
  public:
    explicit SolutionChecker(const ProblemInstance &problem_instance, const Solution &solution)
        : problem_instance(problem_instance), solution(solution)
    {}
    SolutionChecker(const SolutionChecker &) = delete;
    SolutionChecker &operator=(const SolutionChecker &) = delete;
    bool check_solution() const;
    std::vector<std::string> errorMessages() const { return this->mErrorMsg; }

  private:
    bool check_objective() const;
    bool check_job_selected_processing_time() const;
    bool check_resource_usage_over_time_period() const;
    bool check_resource_usage_over_intervals() const;
    bool check_resouce_usage_at_given_time(std::vector<JobAllocation>& allocations, size_t time) const;
    const ProblemInstance &problem_instance;
    const Solution &solution;
    mutable std::vector<std::string> mErrorMsg;
};
