#pragma once

#include "ModelSolutionStatus.hpp"
#include "ProblemInstance/ProblemInstance.hpp"
#include "ResultWriter/ResultWriter.hpp"
#include "Settings.hpp"

enum class SolutionState
{
    OPTIMAL,
    FEASIBLE,
    INFEASIBLE,
    UNKNOWN
};

SolutionState convert(const MODEL_STATUS &ilp_status);
std::string convert_solution_state_to_string(const SolutionState &solution_state);

struct JobAllocation
{
    std::string job_id;
    size_t start_time;
    size_t duration;
    size_t mode_id;
    using ResouceID = size_t;
    using ResouceUnits = std::vector<size_t>;
    std::map<ResouceID, ResouceUnits> units_map;
    std::string get_job_allocation_as_string() const;
};

struct Solution
{
    Solution() = default;
    SolutionState solution_state = SolutionState::UNKNOWN;
    double gap = Settings::Solver::ILP_RELATIVE_GAP;
    double objective_bound = -1;
    size_t makespan = 0;
    double runtime = 0.0;
    double mem_usage = 0.0;
    std::vector<JobAllocation> job_allocations;
    void inverse_allocated_resouce_units(const ProblemInstance &problem_instance);
    std::string get_solution_as_string() const;

  private:
    std::vector<size_t> get_allocated_units_on_given_resource(
        const std::string &resource_id, size_t start_time, size_t requested_units, size_t duration,
        std::map<std::string, std::vector<size_t>, std::less<>> &resource_availability) const;
};

void write_solution_to_excel_file(const std::string &instance_solution_file, const std::string &statistics_file,
                                  const std::string &instance_id, const Solution &solution);

void write_job_allocations_to_json(const std::vector<JobAllocation> &job_allocations, const std::string &filename);
void write_resources_to_json(const std::vector<int> &resource_capacities, const std::string &filename);

void draw_gantt_chart_from_json(const std::vector<JobAllocation> &job_allocations,
                                const std::vector<Resource> &resource_capacities);
