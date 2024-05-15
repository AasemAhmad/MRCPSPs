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
std::string convert_solution_state_to_string(const SolutionState& solution_state);

struct JobAllocation
{
    std::string job_id;
    size_t start_time;
    size_t duration;
    size_t mode_id;
    using ResouceID = size_t;
    using ResouceUnits = std::vector<size_t>;
    std::map<ResouceID, ResouceUnits> units_map;
    operator std::string() const;
};

struct Solution
{
    Solution() : solution_state(SolutionState::UNKNOWN), makespan(0.0), runtime(-1.0), mem_usage(-1.0) {}

    SolutionState solution_state;
    double gap = Settings::SolverSettings::ILP_RELATIVE_GAP;
    double objective_bound = -1;
    size_t makespan;
    double runtime;
    double mem_usage;
    std::vector<JobAllocation> job_allocations;
    void inverse_allocated_resouces(const ProblemInstance& problem_instance);
    operator std::string() const;
};

void write_solution_to_excel_file(const std::string &instance_solution_file, const std::string &statistics_file,
                                  const std::string &instance_id, const Solution &solution);

void write_job_allocations_to_json(const std::vector<JobAllocation> &job_allocations, const std::string &filename);
void write_resources_to_json(const std::vector<int> &resource_capacities, const std::string &filename);

void draw_gantt_chart_from_json(const std::vector<JobAllocation> &job_allocations,
                                const std::vector<Resource> &resource_capacities);
