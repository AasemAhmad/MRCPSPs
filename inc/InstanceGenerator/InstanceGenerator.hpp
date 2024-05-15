#pragma once

#include <fstream>
#include <vector>
#include <map>
#include "External/pempek_assert.hpp"

class InstanceGenerator
{
  public:
    InstanceGenerator(const std::string &file_name) : out_file(file_name)
    {
        PPK_ASSERT_ERROR(out_file.is_open(), "Failed to open the file");
    }
    ~InstanceGenerator() { out_file.close(); }

    InstanceGenerator(const InstanceGenerator &) = delete;
    InstanceGenerator &operator=(const InstanceGenerator &) = delete;
    void generate();

    using ResourceUnits = std::vector<size_t>;
    using JobModes = std::vector<std::vector<size_t>>;
    using ProcessingTimes = std::vector<size_t>;
    using Dependecies = std::map<size_t, std::vector<size_t>>;

  private:
    ResourceUnits generate_resource_units() const;
    JobModes generate_job_modes(const InstanceGenerator::ResourceUnits &resouce_units) const;
    ProcessingTimes generate_processing_times(const InstanceGenerator::JobModes &job_modes) const;
    Dependecies generate_dependencies() const;
    void generated_data_to_json_file();
    std::ofstream out_file;
};
