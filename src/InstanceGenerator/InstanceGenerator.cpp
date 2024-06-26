
#include "InstanceGenerator/InstanceGenerator.hpp"
#include "External/pempek_assert.hpp"
#include "Settings.hpp"
#include "Shared/Utils.hpp"
#include "loguru.hpp"
#include <queue>
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>

void InstanceGenerator::generate()
{
    PPK_ASSERT_ERROR(this->out_file.is_open(), "Failed to open the file");
    generated_data_to_json_file();
}

void InstanceGenerator::generated_data_to_json_file()
{
    rapidjson::Document document;
    document.SetObject();

    InstanceGenerator::Dependencies dependencies = this->generate_dependencies();

    InstanceGenerator::ResourceUnits resources_data = this->generate_resource_units();
    rapidjson::Value resources(rapidjson::kArrayType);
    for (int cap : resources_data)
    {
        resources.PushBack(cap, document.GetAllocator());
    }
    document.AddMember("res_units", resources, document.GetAllocator());

    rapidjson::Value jobs(rapidjson::kArrayType);
    for (size_t i = 0; i < Settings::Generator::NB_JOBS; ++i)
    {
        InstanceGenerator::JobModes job_modes = this->generate_job_modes(resources_data);
        rapidjson::Value job(rapidjson::kObjectType);

        rapidjson::Value id_value;
        id_value.SetString(std::to_string(i).c_str(), document.GetAllocator());
        job.AddMember("id", id_value, document.GetAllocator());

        rapidjson::Value mode_value(rapidjson::kArrayType);
        for (const auto &row : job_modes)
        {
            rapidjson::Value row_array_value(rapidjson::kArrayType);
            for (const auto &element : row)
            {
                row_array_value.PushBack(element, document.GetAllocator());
            }
            mode_value.PushBack(row_array_value, document.GetAllocator());
        }
        job.AddMember("modes", mode_value, document.GetAllocator());

        rapidjson::Value successors(rapidjson::kArrayType);
        rapidjson::Value succ_id;
        for (const auto &item : dependencies[i + 1])
        {
            succ_id.SetString(std::to_string(item).c_str(), document.GetAllocator());
            successors.PushBack(succ_id, document.GetAllocator());
        }
        job.AddMember("succ", successors, document.GetAllocator());

        ProcessingTimes processing_time = generate_processing_times(job_modes);
        rapidjson::Value processing_time_value(rapidjson::kArrayType);
        for (size_t val : processing_time)
        {
            processing_time_value.PushBack(val, document.GetAllocator());
        }
        job.AddMember("processing_time", processing_time_value, document.GetAllocator());

        jobs.PushBack(job, document.GetAllocator());
    }

    document.AddMember("jobs", jobs, document.GetAllocator());

    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    document.Accept(writer);

    PPK_ASSERT_ERROR(out_file.is_open(), "Failed to open the file");
    out_file << buffer.GetString() << std::endl;
    out_file.close();

    LOG_F(INFO, "JSON file written successfully");
}

InstanceGenerator::ResourceUnits InstanceGenerator::generate_resource_units() const
{
    std::vector<size_t> resource_units(Settings::Generator::NB_RESOURCES);

    for (size_t i = 0; i < Settings::Generator::NB_RESOURCES; ++i)
    {
        resource_units[i] =
            random_range(Settings::Generator::MIN_RESOURCE_CAPACITY, Settings::Generator::MAX_RESOURCE_CAPACITY);
    }

    return resource_units;
}

InstanceGenerator::Dependencies InstanceGenerator::generate_dependencies() const
{
    InstanceGenerator::Dependencies dependencies;
    size_t nb_nodes = Settings::Generator::NB_JOBS;

    PPK_ASSERT_ERROR(nb_nodes >= 1, "number of jobs must be strictly positive");

    size_t dummy = 0;
    size_t node_index = 0;
    std::queue<size_t> q;
    q.emplace(dummy);

    size_t nb_created_nodes = 1;

    while (!q.empty() && nb_created_nodes < nb_nodes)
    {
        size_t current = q.front();
        q.pop();

        size_t nb_child_nodes =
            random_range(Settings::Generator::MIN_NB_SUCCESSORS, Settings::Generator::MAX_NB_SUCCESSORS);

        nb_child_nodes = std::min(nb_child_nodes, nb_nodes - nb_created_nodes);

        for (int i = 0; i < nb_child_nodes; ++i)
        {
            ++node_index;
            dependencies[current].emplace_back(node_index);
            q.emplace(node_index);
            ++nb_created_nodes;
        }
    }
    return dependencies;
}

InstanceGenerator::ProcessingTimes
InstanceGenerator::generate_processing_times(const InstanceGenerator::JobModes &job_modes) const
{
    LOG_F(INFO, "job modes size = %ld", job_modes.size());
    InstanceGenerator::ProcessingTimes processing_times(job_modes.size());
    std::vector<size_t> resources_per_mode;

    std::ranges::transform(job_modes, std::back_inserter(resources_per_mode),
                           [](const std::vector<size_t> &row) { return std::accumulate(row.begin(), row.end(), 0); });

    size_t optimal_execution_time_value =
        random_range(Settings::Generator::MIN_EXECUTION_TIME, Settings::Generator::MAX_EXECUTION_TIME / 2);

    auto max_nb_nodes = std::ranges::max_element(resources_per_mode);

    LOG_F(INFO, "max_nb_nodes = %ld", *max_nb_nodes);

    for (size_t i = 0; i < resources_per_mode.size(); ++i)
    {
        double normalized_value = static_cast<double>(resources_per_mode[i]) / static_cast<double>(*max_nb_nodes);
        processing_times[i] =
            Settings::Generator::MAX_EXECUTION_TIME -
            static_cast<size_t>(
                std::ceil(normalized_value *
                          static_cast<double>(Settings::Generator::MAX_EXECUTION_TIME - optimal_execution_time_value)));
    }
    return processing_times;
}

InstanceGenerator::JobModes
InstanceGenerator::generate_job_modes(const InstanceGenerator::ResourceUnits &resource_units) const
{
    std::size_t nb_modes = random_range(Settings::Generator::MIN_NB_MODES, Settings::Generator::MAX_NB_MODES);

    const size_t nb_resources = Settings::Generator::NB_RESOURCES;

    InstanceGenerator::JobModes job_modes;
    job_modes.reserve(nb_modes);

    for (size_t i = 0; i < nb_modes;)
    {
        std::vector<size_t> mode(nb_resources);
        for (size_t j = 0; j < nb_resources; ++j)
        {
            size_t max_nb_units = std::min(resource_units[j], Settings::Generator::MAX_NB_RESOURCE_UNITS_PER_JOB);
            PPK_ASSERT_ERROR(max_nb_units >= Settings::Generator::MIN_NB_RESOURCE_UNITS_PER_JOB,
                             "Invalid Config Parameter");
            mode[j] = random_range(Settings::Generator::MIN_NB_RESOURCE_UNITS_PER_JOB, max_nb_units);
        }

        if (const bool no_resource_units_requested = std::ranges::all_of(mode, [](size_t value) { return value == 0; }))
        {
            continue;
        }
        job_modes.emplace_back(std::move(mode));
        ++i;
    }

    return job_modes;
}