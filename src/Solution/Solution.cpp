#include "Solution/Solution.hpp"
#include "External/pempek_assert.hpp"
#include "loguru.hpp"
#include <Python.h>
#include <Shared/Utils.hpp>
#include <filesystem>
#include <fstream>
#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <sstream>
#include <string>
#include <vector>

SolutionState convert(const MODEL_STATUS &ilp_status)
{
    SolutionState solution_state;
    switch (ilp_status)
    {
        using enum MODEL_STATUS;
        using enum SolutionState;
    case MODEL_SOL_OPTIMAL:
        solution_state = OPTIMAL;
        break;
    case MODEL_SOL_FEASIBLE:
        solution_state = FEASIBLE;
        break;
    case MODEL_SOL_INFEASIBLE:
        solution_state = INFEASIBLE;
        break;
    default:
        solution_state = UNKNOWN;
    }
    return solution_state;
}

std::string convert_solution_state_to_string(const SolutionState &solution_state)
{
    std::string solution_state_str;
    switch (solution_state)
    {
        using enum SolutionState;
    case FEASIBLE:
        solution_state_str = "Feasible";
        break;
    case OPTIMAL:
        solution_state_str = "Optimal";
        break;
    case INFEASIBLE:
        solution_state_str = "Infeasible";
        break;
    case UNKNOWN:
        solution_state_str = "Unknown";
        break;
    }
    return solution_state_str;
}

std::string JobAllocation::get_job_allocation_as_string() const
{
    std::ostringstream string_stream;
    string_stream << "Job ID: " << this->job_id << ", "
                  << "Start Time: " << this->start_time << ", "
                  << "Duration: " << this->duration << ", "
                  << "Mode ID: " << this->mode_id;
    return string_stream.str();
}

std::string Solution::get_solution_as_string() const
{
    std::ostringstream string_stream;
    string_stream << "Solution:\n"
                  << "Run Time: " << this->runtime << ", "
                  << "Gap: " << this->gap << ", "
                  << "Makespan: " << this->makespan << ", "
                  << "State: " << convert_solution_state_to_string(this->solution_state) << "\n";

    std::string job_allocations_string;
    for (const auto &job_allocation : job_allocations)
    {
        job_allocations_string += (job_allocation.get_job_allocation_as_string() + "\n");
    }

    return string_stream.str() + job_allocations_string;
}

std::vector<size_t> Solution::get_allocated_units_on_given_resource(
    const std::string &resource_id, size_t start_time, size_t requested_units, size_t duration,
    std::map<std::string, std::vector<size_t>, std::less<>> &resource_availability) const
{
    PPK_ASSERT_ERROR(requested_units > 0, "requested resource units must be greated than 0");
    PPK_ASSERT_ERROR(duration > 0, "duration must be greater than 0");

    std::vector<size_t> units;
    units.reserve(requested_units);

    auto &res_availability = resource_availability.at(resource_id);
    for (size_t r_unit = 0; r_unit < res_availability.size(); ++r_unit)
    {
        if (res_availability[r_unit] <= start_time)
        {
            units.emplace_back(r_unit);
            res_availability[r_unit] = start_time + duration;

            if (units.size() == requested_units)
            {
                break;
            }
        }
    }
    PPK_ASSERT_ERROR(units.size() == requested_units,
                     "Failed to allocated requested resources at resourc %s, allocated = %ld, requested = %ld",
                     resource_id.c_str(), units.size(), requested_units);
    return units;
}

void Solution::inverse_allocated_resouce_units(const ProblemInstance &problem_instance)
{
    PPK_ASSERT_ERROR(this->job_allocations.size() == problem_instance.job_queue.nb_items(),
                     "at least one job was not allocated %ld, %ld", this->job_allocations.size(),
                     problem_instance.job_queue.nb_items());
    this->job_allocations = sort_by_field(this->job_allocations, &JobAllocation::start_time);

    std::map<std::string, std::vector<size_t>, std::less<>> resource_availability;

    for (const auto &resource : problem_instance.resources)
    {
        auto [iterator, emplaced_resource_units] = resource_availability.try_emplace(resource.id, resource.units, 0);
        PPK_ASSERT_ERROR(emplaced_resource_units, "Failed to emplace resource uints");
    }

    for (auto &job_allocation : this->job_allocations)
    {
        std::string job_id = job_allocation.job_id;
        auto job = problem_instance.job_queue.find_item(job_id);
        PPK_ASSERT_ERROR(job != nullptr, "Invalid job_id %s", job_id.c_str());
        PPK_ASSERT_ERROR(job_allocation.mode_id > 0, "Invalid value %ld",
                         job_allocation.mode_id);
        size_t mode_index = job_allocation.mode_id - 1;
        PPK_ASSERT_ERROR(mode_index < job->modes.size(), "Invalid value %ld", mode_index);

        const Mode &mode = job->modes.at(mode_index);
        size_t resouce_index = 0;

        for (const auto &[resource_id, resource] : problem_instance.resources)
        {
            size_t requested_units = mode.requested_resources.at(resouce_index).units;
            if (size_t duration = job_allocation.duration; requested_units > 0 && duration > 0)
            {
                std::vector<size_t> units = this->get_allocated_units_on_given_resource(
                    resource_id, job_allocation.start_time, requested_units, duration, resource_availability);

                auto [iterator, emplaced_units] = job_allocation.units_map.try_emplace(resouce_index, std::move(units));
                PPK_ASSERT_ERROR(emplaced_units, "resource units were already inserted");
            }
            ++resouce_index;
        }
    }
}

void write_solution_to_excel_file(const std::string &instance_solution_file, const std::string &statistics_file,
                                  const std::string &instance_id, const Solution &solution)
{
    static const std::vector<std::string> instance_solution_file_header = {
        "Instance_ID", "Job_ID", "Start_Time", "Processing_Time", "Finish_Time", "Mode_ID"};

    static const std::vector<std::string> statistics_file_header = {"Instance_ID", "Run_Time", "Gap", "Makespan",
                                                                    "Status"};

    static ResultWriter instance_solution_writer(instance_solution_file, instance_solution_file_header);
    static ResultWriter statistics_writer(statistics_file, statistics_file_header);

    for (const auto &item : solution.job_allocations)
    {
        ResultWriter::Row solution_row = {{"Instance_ID", instance_id},
                                          {"Job_ID", item.job_id},
                                          {"Start_Time", std::to_string(item.start_time)},
                                          {"Processing_Time", std::to_string(item.duration)},
                                          {"Finish_Time", std::to_string(item.start_time + item.duration)},
                                          {"Mode_ID", std::to_string(item.mode_id)}};
        instance_solution_writer.write(solution_row);
    }

    ResultWriter::Row statistics_row = {{"Instance_ID", instance_id},
                                        {"Run_Time", std::to_string(solution.runtime)},
                                        {"Gap", std::to_string(solution.gap)},
                                        {"Makespan", std::to_string(solution.makespan)},
                                        {"Status", convert_solution_state_to_string(solution.solution_state)}};
    statistics_writer.write(statistics_row);
}

void write_job_allocations_to_json(const std::vector<JobAllocation> &job_allocations, const std::string &filename)
{
    rapidjson::Document doc(rapidjson::kObjectType);
    auto &allocator = doc.GetAllocator();
    rapidjson::Value jobs(rapidjson::kArrayType);
    for (const auto &job : job_allocations)
    {
        rapidjson::Value job_allocation(rapidjson::kObjectType);
        job_allocation.AddMember("job_id", rapidjson::StringRef(job.job_id.c_str()), allocator);
        job_allocation.AddMember("start_time", job.start_time, allocator);
        job_allocation.AddMember("duration", job.duration, allocator);
        job_allocation.AddMember("mode_id", job.mode_id, allocator);

        rapidjson::Value units_allocation_maping(rapidjson::kArrayType);
        rapidjson::Value resource_id_mapping(rapidjson::kArrayType);

        for (const auto &[resource_id, units] : job.units_map)
        {
            rapidjson::Value uints_allocation_array_value(rapidjson::kArrayType);
            resource_id_mapping.PushBack(resource_id, allocator);
            for (const auto &element : units)
            {
                uints_allocation_array_value.PushBack(element, allocator);
            }
            units_allocation_maping.PushBack(uints_allocation_array_value, allocator);
        }
        job_allocation.AddMember("resource_ids", resource_id_mapping, allocator);
        job_allocation.AddMember("units_map", units_allocation_maping, allocator);
        jobs.PushBack(job_allocation, allocator);
    }
    doc.AddMember("jobs", jobs, allocator);

    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);
    std::ofstream ofs(filename);
    PPK_ASSERT_ERROR(ofs.is_open(), "file cannot be oppened");
    ofs << buffer.GetString() << std::endl;
    ofs.close();
}

void write_resources_to_json(const std::vector<Resource> &resources, const std::string &filename)
{
    rapidjson::Document doc(rapidjson::kObjectType);
    auto &allocator = doc.GetAllocator();
    rapidjson::Value resources_value(rapidjson::kArrayType);
    for (const auto &r : resources)
    {
        rapidjson::Value resource(rapidjson::kObjectType);
        resource.AddMember("id", rapidjson::StringRef(r.id.c_str()), allocator);
        resource.AddMember("units", r.units, allocator);
        resources_value.PushBack(resource, allocator);
    }
    doc.AddMember("resources", resources_value, allocator);

    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);
    std::ofstream ofs(filename);
    PPK_ASSERT_ERROR(ofs.is_open(), "file cannot be oppened");
    ofs << buffer.GetString() << std::endl;
    ofs.close();
}

void draw_gantt_chart_from_json(const std::vector<JobAllocation> &job_allocations,
                                const std::vector<Resource> &resources)
{
    Py_Initialize();
    write_job_allocations_to_json(job_allocations, "jobs.json");
    write_resources_to_json(resources, "resources.json");
    std::string command = "python3 ../../src/PythonModules/interface_json.py jobs.json resources.json";
    int result = std::system(command.c_str());
    PPK_ASSERT_ERROR(result != -1, "Error executing Python script");
    Py_Finalize();
}
