#pragma once

namespace Settings
{
// clang-format off
    #define DEFAULT_INSTANCES_DIRECTORY_PATH "../../Instances/Set10/"
    #define DEFAULT_GENERATE_INSTANCES       true
    #define DEFAULT_FIRST_INSTANCE_INDEX     0
    #define DEFAULT_LAST_INSTANCE_INDEX      10
    #define DEFAULT_INSTANCE_NAME            "instance"

    namespace Generator
    {
        #define DEFAULT_NB_RESOURCES                  1
        #define DEFAULT_MAX_RESOURCE_CAPACITY         10
        #define DEFAULT_MIN_RESOURCE_CAPACITY         1
        #define DEFAULT_NB_JOBS                       5
        #define DEFAULT_MIN_NB_SUCCESSORS             1
        #define DEFAULT_MAX_NB_SUCCESSORS             3
        #define DEFAULT_MAX_NB_MODES                  3
        #define DEFAULT_MIN_NB_MODES                  2
        #define DEFAULT_MAX_EXECUTION_TIME            5
        #define DEFAULT_MIN_EXECUTION_TIME            3
        #define DEFAULT_MAX_NB_RESOURCE_UNITS_PER_JOB 4
        #define DEFAULT_MIN_NB_RESOURCE_UNITS_PER_JOB 1
    } // namespace GeneratorSettings

    namespace Solver
    {
        #define DEFAULT_RESULTS_DIRECTORY "../../Result/"
        #define DEFAULT_VERBOSE           false
        #define DEFAULT_NB_THREADS        10
        #define DEFAULT_MAX_RUNTIME       30.0
        #define DEFAULT_USE_GUROBI        true
        #define DEFAULT_USE_CPLEX         false
        #define DEFAULT_USE_CP            false
        #define DEFAULT_CHECK_SOLUTION    true
        #define DEFAULT_DRAW_GANTT_CHART  false;

        #define DEFAULT_INIT_ILP_SOLUTION false
        #define DEFAULT_ILP_RELATIVE_GAP  0.0
        #define DEFAULT_INIT_SOLUTION     false
    } // namespace SolverSettings
} // namespace Settings
