#pragma once

#include "DefaultSettings.hpp"
#include <string>

namespace Settings
{
    extern std::string INSTANCES_DIRECTORY_PATH;
    extern bool GENERATE_INSTANCES;
    extern size_t FIRST_INSTANCE_INDEX;
    extern size_t LAST_INSTANCE_INDEX;
    extern std::string INSTANCE_NAME;

    namespace Generator
    {
        extern size_t NB_RESOURCES;
        extern size_t MAX_RESOURCE_CAPACITY;
        extern size_t MIN_RESOURCE_CAPACITY;
        extern size_t NB_JOBS;
        extern size_t MIN_NB_SUCCESSORS;
        extern size_t MAX_NB_SUCCESSORS;
        extern size_t MAX_NB_MODES;
        extern size_t MIN_NB_MODES;
        extern size_t MAX_EXECUTION_TIME;
        extern size_t MIN_EXECUTION_TIME;
        extern size_t MAX_NB_RESOURCE_UNITS_PER_JOB;
        extern size_t MIN_NB_RESOURCE_UNITS_PER_JOB;
    } // namespace Generator

    namespace Solver
    {
        extern std::string RESULTS_DIRECTORY;
        extern bool VERBOSE;
        extern size_t NB_THREADS;
        extern double MAX_RUNTIME;
        extern bool USE_GUROBI;
        extern bool USE_CPLEX;
        extern bool USE_CP;
        extern bool CHECK_SOLUTION;
        extern bool DRAW_GANTT_CHART;

        extern bool INIT_ILP_SOLUTION;
        extern double ILP_RELATIVE_GAP;
        extern bool INIT_SOLUTION;
    } // namespace Solver
} // namespace Settings
