#pragma once

#include <memory>
#include <string>
#include <vector>

struct Resource
{
    std::string id;
    size_t units;
};

struct Mode
{
    std::vector<Resource> requested_resources;
    size_t processing_time;
};

struct Job
{
    Job() = default;
    std::string get_id() const;
    static bool compare_by_job_id(const std::shared_ptr<Job> &j1, const std::shared_ptr<Job> &j2);
    static bool compare_by_release_time(const std::shared_ptr<Job> &j1, const std::shared_ptr<Job> &j2);

    std::string j_id;
    size_t release_time;
    std::vector<Mode> modes;
    std::vector<std::string> successors;
};
