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
    case MODEL_STATUS::MODEL_SOL_OPTIMAL:
        solution_state = SolutionState::OPTIMAL;
        break;
    case MODEL_STATUS::MODEL_SOL_FEASIBLE:
        solution_state = SolutionState::FEASIBLE;
        break;
    case MODEL_STATUS::MODEL_SOL_INFEASIBLE:
        solution_state = SolutionState::INFEASIBLE;
        break;
    default:
        solution_state = SolutionState::UNKNOWN;
    }
    return solution_state;
}

std::string convert_solution_state_to_string(const SolutionState &solution_state)
{
    std::string solution_state_str;
    switch (solution_state)
    {
    case SolutionState::FEASIBLE:
        solution_state_str = "Feasible";
        break;
    case SolutionState::OPTIMAL:
        solution_state_str = "Optimal";
        break;
    case SolutionState::INFEASIBLE:
        solution_state_str = "Infeasible";
        break;
    case SolutionState::UNKNOWN:
        solution_state_str = "Unknown";
        break;
    }
    return solution_state_str;
}

JobAllocation::operator std::string() const
{
    std::ostringstream string_stream;
    string_stream << "Job ID: " << this->job_id << ", "
                  << "Start Time: " << this->start_time << ", "
                  << "Duration: " << this->duration << ", "
                  << "Mode ID: " << this->mode_id;
    return string_stream.str();
}

Solution::operator std::string() const
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
        job_allocations_string += static_cast<std::string>(job_allocation) + "\n";
    }

    return string_stream.str() + job_allocations_string;
}

void Solution::inverse_allocated_resouces(const ProblemInstance &problem_instance)
{
    this->job_allocations = sort_by_field(this->job_allocations, &JobAllocation::start_time);

    std::map<std::string, std::vector<size_t>> resource_availability;
    for (const auto &resource : problem_instance.resources)
    {
        resource_availability.insert({resource.id, std::vector<size_t>(resource.units, 0)});
    }

    for (auto &job_allocation : this->job_allocations)
    {
        std::string job_id = job_allocation.job_id;
        auto job = problem_instance.job_queue.find_item(job_id);
        PPK_ASSERT_ERROR(job != nullptr, "Invalid job_id %s", job_id.c_str());
        PPK_ASSERT_ERROR(job_allocation.mode_id > 0, "Invalid value %ld mode id must be greater than 0",
                         job_allocation.mode_id);
        size_t mode_index = job_allocation.mode_id - 1;
        PPK_ASSERT_ERROR(mode_index < job->modes.size(), "Invalid value %ld", mode_index);

        const Mode mode = job->modes.at(mode_index);
        bool allocated = false;
        size_t resouce_index = 0;
        for (const auto &[resource_id, resource] : problem_instance.resources)
        {
            std::vector<size_t> units;
            for (size_t i = 0; i < resource_availability.at(resource_id).size(); ++i)
            {
                if ((resource_availability.at(resource_id).at(i) <= job_allocation.start_time))
                {
                    units.push_back(i);
                    resource_availability.at(resource_id).at(i) = job_allocation.start_time + job_allocation.duration;
                    if (units.size() == mode.requested_resources.at(resouce_index).units)
                    {
                        break;
                    }
                }
            }
            job_allocation.units_map.insert({resouce_index, units});
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

    static ResultWriter instance_solution_writer(instance_solution_file, std::move(instance_solution_file_header));
    static ResultWriter statistics_writer(statistics_file, std::move(statistics_file_header));

    for (const auto &item : solution.job_allocations)
    {
        ResultWriter::Row solution_row = {{"Instance_ID", instance_id},
                                          {"Job_ID", item.job_id},
                                          {"Start_Time", std::to_string(item.start_time)},
                                          {"Processing_Time", std::to_string(item.duration)},
                                          {"Finish_Time", std::to_string(item.start_time + item.duration)},
                                          {"Mode_ID", std::to_string(item.mode_id)}};
        instance_solution_writer.write(std::move(solution_row));
    }

    ResultWriter::Row statistics_row = {{"Instance_ID", instance_id},
                                        {"Run_Time", std::to_string(solution.runtime)},
                                        {"Gap", std::to_string(solution.gap)},
                                        {"Makespan", std::to_string(solution.makespan)},
                                        {"Status", convert_solution_state_to_string(solution.solution_state)}};
    statistics_writer.write(std::move(statistics_row));
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
        rapidjson::Value units_map_value(rapidjson::kArrayType);

        for (const auto &allocation : job.units_map)
        {
            rapidjson::Value allocation_array_value(rapidjson::kArrayType);
            for (const auto &element : allocation.second)
            {
                allocation_array_value.PushBack(element, allocator);
            }
            units_map_value.PushBack(allocation_array_value, allocator);
        }
        job_allocation.AddMember("units_map", units_map_value, allocator);

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
