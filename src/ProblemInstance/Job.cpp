#include "ProblemInstance/Job.hpp"

bool compare_by_job_id(const JobConstPtr &j1, const JobConstPtr &j2) { return j1->id < j2->id; }

bool compare_by_release_time(const JobConstPtr &j1, const JobConstPtr &j2) { return j1->id < j2->id; }