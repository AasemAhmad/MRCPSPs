#pragma once

#include <memory>
#include <string>
#include <vector>

struct Job;

using JobPtr = std::shared_ptr<Job>;
using JobConstPtr = std::shared_ptr<const Job>;

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
    std::string id;
    size_t release_time;
    std::vector<Mode> modes;
    std::vector<std::string> successors;
};

bool compare_by_job_id(const JobConstPtr &j1, const JobConstPtr &j2);
bool compare_by_release_time(const JobConstPtr &j1, const JobConstPtr &j2);
