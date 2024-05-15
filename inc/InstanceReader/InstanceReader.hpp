#pragma once

#include "ProblemInstance/ProblemInstance.hpp"
#include <fstream>

class InstanceReader
{
  public:
    InstanceReader(const std::string &instance_file_name) : instance_file(instance_file_name)
    {
        PPK_ASSERT_ERROR(instance_file.is_open(), "Failed to open the file");
    }
    ~InstanceReader() { instance_file.close(); }
    InstanceReader(const InstanceReader &) = delete;
    InstanceReader &operator=(const InstanceReader &) = delete;
    void read(const std::string &programm_task_options, ProblemInstance &problem_instance);

  private:
    std::ifstream instance_file;
};
