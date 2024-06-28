#pragma once

#include "ProblemInstance/ProblemInstance.hpp"
#include "Settings.hpp"
#include <map>
#include <string>

enum class MODEL_STATUS
{
    MODEL_SOL_OPTIMAL,
    MODEL_SOL_FEASIBLE,
    MODEL_SOL_INFEASIBLE,
    MODEL_SOL_UNBOUNDED,
    MODEL_SOL_BOUNDED,
    MODEL_SOL_INFEASIBLE_OR_UNBOUNDED,
    MODEL_SOL_UNKNOWN,
    MODEL_SOL_ERROR
};

enum class SolutionState
{
    OPTIMAL,
    FEASIBLE,
    INFEASIBLE,
    UNBOUNDED,
    INFEASIBLE_OR_UNBOUNDED,
    BOUNDED,
    ERROR,
    UNKNOWN
};

SolutionState convert(const MODEL_STATUS &ilp_status);

std::string solution_state_as_string(const SolutionState &solution_state);

struct JobAllocation
{
    std::string job_id;
    size_t start_time;
    size_t duration;
    size_t mode_id;
    using ResourceID = size_t;
    using ResourceUnits = std::vector<size_t>;
    std::map<ResourceID, ResourceUnits> units_map;
    std::string get_job_allocation_as_string() const;
};

struct Solution
{
    SolutionState solution_state = SolutionState::UNKNOWN;
    double gap = Settings::Solver::ILP_RELATIVE_GAP;
    double objective_bound = -1;
    size_t makespan = 0;
    double runtime = 0.0;
    double mem_usage = 0.0;
    std::vector<JobAllocation> job_allocations;
    void inverse_allocated_resource_units(const ProblemInstance &problem_instance);
    std::string get_solution_as_string() const;

  private:
    std::vector<size_t> get_allocated_units_on_given_resource(
        const std::string &resource_id, size_t start_time, size_t requested_units, size_t duration,
        std::map<std::string, std::vector<size_t>, std::less<>> &resource_availability) const;
};

void write_results_to_excel_file(const std::string &instance_solution_file, const std::string &statistics_file,
                                 const std::string &instance_id, const Solution &solution);

void write_solution(const std::string &instance_solution_file, const std::string &instance_id,
                    const Solution &solution);

void write_statistics(const std::string &statistics_file, const std::string &instance_id, const Solution &solution);

void write_job_allocations_to_json(const std::vector<JobAllocation> &job_allocations, const std::string &filename);
void write_resources_to_json(const std::vector<int> &resource_capacities, const std::string &filename);

void draw_gantt_chart_from_json(const std::vector<JobAllocation> &job_allocations,
                                const std::vector<Resource> &resource_capacities);
