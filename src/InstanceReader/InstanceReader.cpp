#include "InstanceReader/InstanceReader.hpp"
#include "External/pempek_assert.hpp"
#include "ProblemInstance/Job.hpp"
#include "loguru.hpp"
#include <format>
#include <rapidjson/document.h>
#include <string>

void InstanceReader::read(ProblemInstance &problem_instance)
{
    PPK_ASSERT_ERROR(this->instance_file.is_open(), "Failed to open the file");
    std::string content((std::istreambuf_iterator<char>(instance_file)), (std::istreambuf_iterator<char>()));
    instance_file.close();
    PPK_ASSERT_ERROR(!content.empty(), "Content read from the file is empty or invalid ");

    rapidjson::Document doc;
    doc.Parse(content.c_str());

    PPK_ASSERT_ERROR(doc.IsObject(), "Invalid content: %s Not a JSON object", content.c_str());

    const rapidjson::Value &res_units_array = doc["res_units"];
    problem_instance.resources.reserve(res_units_array.Size());
    for (rapidjson::SizeType i = 0; i < res_units_array.Size(); ++i)
    {
        Resource resource;
        resource.id = std::format("r_{}", i);
        resource.units = res_units_array[i].GetUint();
        problem_instance.resources.push_back(std::move(resource));
    }
    const rapidjson::Value &jobs_array = doc["jobs"];
    for (rapidjson::SizeType i = 0; i < jobs_array.Size(); ++i)
    {
        auto job = std::make_shared<Job>();
        job->id = jobs_array[i]["id"].GetString();

        // Parse modes
        const rapidjson::Value &modes_array = jobs_array[i]["modes"];
        for (rapidjson::SizeType j = 0; j < modes_array.Size(); ++j)
        {
            Mode mode;
            for (rapidjson::SizeType k = 0; k < modes_array[j].Size(); ++k)
            {
                Resource r;
                r.id = std::format("r_{}", k);
                r.units = modes_array[j][k].GetUint();
                mode.requested_resources.push_back(std::move(r));
            }
            job->modes.push_back(std::move(mode));
        }

        const rapidjson::Value &processing_time_array = jobs_array[i]["processing_time"];
        for (rapidjson::SizeType j = 0; j < processing_time_array.Size(); ++j)
        {
            job->modes.at(j).processing_time = processing_time_array[j].GetUint();
        }

        const rapidjson::Value &successor_array = jobs_array[i]["succ"];
        job->successors.reserve(successor_array.Size());
        for (rapidjson::SizeType j = 0; j < successor_array.Size(); ++j)
        {
            job->successors.emplace_back(successor_array[j].GetString());
        }
        problem_instance.job_queue.append_element(job);
    }

    problem_instance.set_makespan_upperbound();
}
