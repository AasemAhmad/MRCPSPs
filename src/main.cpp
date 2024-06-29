#include "Algorithms/CPSolver/CPSolver.hpp"
#include "Algorithms/ILPOptimizationModel/ProblemSolverILP.hpp"
#include "External/ILPSolverModel/ILPSolverInterface.hpp"
#include "External/cxxopts.hpp"
#include "InstanceGenerator/InstanceGenerator.hpp"
#include "InstanceReader/InstanceReader.hpp"
#include "ProblemInstance/ProblemInstance.hpp"
#include "Settings.hpp"
#include "Shared/Utils.hpp"
#include "Solution/SolutionChecker.hpp"
#include <Shared/Utils.hpp>
#include <filesystem>
#include <format>
#include <fstream>
#include <loguru.hpp>
#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include <unordered_set>
#include <vector>

static const std::unordered_set<std::string, StringHash, std::equal_to<>> program_tasks_set = {"solver", "generator"};
static const std::unordered_set<std::string, StringHash, std::equal_to<>> verbosity_levels_set = {"debug", "info",
                                                                                                  "quiet", "silent"};

static void parse_command_line(std::string &program_task, std::string &program_task_conf, std::string &verbosity_level,
                               int argc, char **argv)

{
    cxxopts::Options options("MultiResourceProjectScheduler",
                             "Scheduling Algorithms for Multiple Resource-Constrained Project");
    try
    {
        options.add_options()("program_task", "Program task", cxxopts::value<std::string>(program_task));
        options.add_options()("program_task_conf", "Program task configuration",
                              cxxopts::value<std::string>(program_task_conf));
        options.add_options()("verbosity", "Verbosity level", cxxopts::value<std::string>(verbosity_level));

        auto result = options.parse(argc, argv);

        PPK_ASSERT_ERROR(program_tasks_set.find(program_task) != program_tasks_set.end(), "Invalid program task");
        PPK_ASSERT_ERROR(verbosity_levels_set.find(verbosity_level) != verbosity_levels_set.end(),
                         "Invalid verbosity level");
        PPK_ASSERT_ERROR(program_task_conf.ends_with(".json"), "Invalid config file format");

        std::filesystem::path path = std::filesystem::current_path().parent_path().parent_path() / "conf";
        PPK_ASSERT_ERROR(std::filesystem::exists(path) && std::filesystem::is_directory(path),
                         "Path does not exist or is not a directory");

        program_task_conf = std::format("{}/{}", path.c_str(), program_task_conf);

        PPK_ASSERT_ERROR(std::filesystem::exists(program_task_conf), "Config file does not exist");
    } catch (const cxxopts::exceptions::exception &e)
    {
        LOG_F(ERROR, "Error parsing options: %s", e.what());
        exit(1);
    }
}

static void set_logger_verbosity(const std::string &verbosity_level)
{
    // TODO
}

static bool parse_generator_option_parameters(const std::string &generator_options)
{
    rapidjson::Document json_doc_generator_options;
    json_doc_generator_options.Parse(generator_options.c_str());

    PPK_ASSERT_ERROR(json_doc_generator_options.IsObject(), "Invalid generator options: %s Not a JSON object",
                     generator_options.c_str());

    Settings::INSTANCES_DIRECTORY_PATH =
        parse_scalar<std::string>(json_doc_generator_options, "instances_directory_path");
    Settings::Generator::NB_RESOURCES = parse_scalar<size_t>(json_doc_generator_options, "nb_resources");
    Settings::Generator::MIN_RESOURCE_CAPACITY =
        parse_scalar<size_t>(json_doc_generator_options, "min_resource_capacity");
    Settings::Generator::MAX_RESOURCE_CAPACITY =
        parse_scalar<size_t>(json_doc_generator_options, "max_resource_capacity");
    Settings::Generator::NB_JOBS = parse_scalar<size_t>(json_doc_generator_options, "nb_jobs");
    Settings::Generator::MIN_NB_SUCCESSORS = parse_scalar<size_t>(json_doc_generator_options, "min_nb_succ");
    Settings::Generator::MAX_NB_SUCCESSORS = parse_scalar<size_t>(json_doc_generator_options, "max_nb_succ");
    Settings::Generator::MAX_NB_MODES = parse_scalar<size_t>(json_doc_generator_options, "max_nb_modes");
    Settings::Generator::MIN_NB_MODES = parse_scalar<size_t>(json_doc_generator_options, "min_nb_modes");
    Settings::Generator::MAX_EXECUTION_TIME = parse_scalar<size_t>(json_doc_generator_options, "max_execution_time");
    Settings::Generator::MIN_EXECUTION_TIME = parse_scalar<size_t>(json_doc_generator_options, "min_execution_time");
    Settings::Generator::MAX_NB_RESOURCE_UNITS_PER_JOB =
        parse_scalar<size_t>(json_doc_generator_options, "max_nb_resource_units_per_job");
    Settings::Generator::MIN_NB_RESOURCE_UNITS_PER_JOB =
        parse_scalar<size_t>(json_doc_generator_options, "min_nb_resource_units_per_job");
    Settings::FIRST_INSTANCE_INDEX = parse_scalar<size_t>(json_doc_generator_options, "first_instance_index");
    Settings::LAST_INSTANCE_INDEX = parse_scalar<size_t>(json_doc_generator_options, "last_instance_index");

    return true;
}

static bool parse_solver_option_parameters(const std::string &solver_options)
{
    rapidjson::Document json_doc_solver_options;
    json_doc_solver_options.Parse(solver_options.c_str());
    PPK_ASSERT_ERROR(json_doc_solver_options.IsObject(), "Invalid solver options: %s Not a JSON object",
                     solver_options.c_str());

    Settings::INSTANCE_NAME = parse_scalar<std::string>(json_doc_solver_options, "instance_file_name");
    Settings::INSTANCES_DIRECTORY_PATH = parse_scalar<std::string>(json_doc_solver_options, "instances_directory_path");
    Settings::FIRST_INSTANCE_INDEX = parse_scalar<size_t>(json_doc_solver_options, "first_instance_index");
    Settings::LAST_INSTANCE_INDEX = parse_scalar<size_t>(json_doc_solver_options, "last_instance_index");
    Settings::Solver::RESULTS_DIRECTORY = parse_scalar<std::string>(json_doc_solver_options, "results_directory");
    Settings::Solver::NB_THREADS = parse_scalar<size_t>(json_doc_solver_options, "nb_of_thread");
    Settings::Solver::MAX_RUNTIME = parse_scalar<double>(json_doc_solver_options, "max_runtime");
    Settings::Solver::INIT_ILP_SOLUTION = parse_scalar<bool>(json_doc_solver_options, "init_ilp_solution");
    Settings::Solver::ILP_RELATIVE_GAP = parse_scalar<double>(json_doc_solver_options, "ilp_relative_gap");

    if (json_doc_solver_options.HasMember("use_gurobi"))
    {
        Settings::Solver::USE_GUROBI = parse_scalar<bool>(json_doc_solver_options, "use_gurobi");
    }

    if (json_doc_solver_options.HasMember("use_gurobi"))
    {
        Settings::Solver::USE_CP = parse_scalar<bool>(json_doc_solver_options, "use_cp");
    }

    PPK_ASSERT_ERROR(Settings::Solver::USE_GUROBI || Settings::Solver::USE_CP, "Either Gurobi or CP must be selected");

    if (json_doc_solver_options.HasMember("check_solution"))
    {

        Settings::Solver::CHECK_SOLUTION = parse_scalar<bool>(json_doc_solver_options, "check_solution");
    }

    if (json_doc_solver_options.HasMember("draw_gantt_chart"))
    {
        Settings::Solver::DRAW_GANTT_CHART = parse_scalar<bool>(json_doc_solver_options, "draw_gantt_chart");
    }

    return true;
}

static void run_generator(const std::string &program_task_options)
{
    if (parse_generator_option_parameters(program_task_options))
    {
        std::filesystem::path path = Settings::INSTANCES_DIRECTORY_PATH;

        if (!std::filesystem::exists(path))
        {
            PPK_ASSERT_ERROR(std::filesystem::create_directories(path), "Failed to create directories %s",
                             path.c_str());
        }

        for (size_t index = Settings::FIRST_INSTANCE_INDEX; index <= Settings::LAST_INSTANCE_INDEX; ++index)
        {
            std::string file_name = std::format("{}instance_{}.json", Settings::INSTANCES_DIRECTORY_PATH, index);
            LOG_F(INFO, "file name = %s", file_name.c_str());
            InstanceGenerator generator(file_name);
            generator.generate();
        }
    }
}

static bool run_solution_checker(const ProblemInstance &problem_instance, const Solution &solution)
{
    SolutionChecker checker(problem_instance, solution);
    PPK_ASSERT_ERROR(checker.check_solution(), "Solution Checker failed!");
    return true;
}

static void write_results(const ProblemInstance &problem_instance, const std::string &short_instance_name,
                          Solution &solution)
{
    std::string solution_file = std::format("{}instances_solution.xlsx", Settings::Solver::RESULTS_DIRECTORY);
    std::string statistic_file = std::format("{}statistics.xlsx", Settings::Solver::RESULTS_DIRECTORY);

    write_results_to_excel_file(solution_file, statistic_file, short_instance_name, solution);

    if (Settings::Solver::DRAW_GANTT_CHART)
    {
        solution.inverse_allocated_resource_units(problem_instance);
        draw_gantt_chart_from_json(solution.job_allocations, problem_instance.get_resources());
    }
}

static void run_solver(const std::string &program_task_options)
{
    parse_solver_option_parameters(program_task_options);

    std::filesystem::create_directories(Settings::Solver::RESULTS_DIRECTORY);

    for (size_t index = Settings::FIRST_INSTANCE_INDEX; index <= Settings::LAST_INSTANCE_INDEX; ++index)
    {
        const std::string short_instance_name = std::format("{}_{}.json", Settings::INSTANCE_NAME, index);

        ProblemInstance problem_instance(std::format("{}{}", Settings::INSTANCES_DIRECTORY_PATH, short_instance_name));

        InstanceReader reader(problem_instance.get_name());
        reader.read(problem_instance);
        PPK_ASSERT_ERROR(problem_instance.validate_problem_instance(), "Invalid problem instance");
        Solution solution;
        if (Settings::Solver::USE_GUROBI)
        {
            Solution initSol;
            if (Settings::Solver::INIT_ILP_SOLUTION)
            {
                // TODO
            }
            ProblemSolverILP ilpSolver(problem_instance);
            solution = ilpSolver.solve(initSol);

        } else if (Settings::Solver::USE_CP)
        {
            CPSolver solver(problem_instance);
            solution = solver.solve();
        }

        if (solution.solution_state == SolutionState::FEASIBLE || solution.solution_state == SolutionState::OPTIMAL)
        {
            LOG_F(INFO, "solution = %s", solution.get_solution_as_string().c_str());

            if (Settings::Solver::CHECK_SOLUTION)
            {
                PPK_ASSERT_ERROR(run_solution_checker(problem_instance, solution), "Wrong Solution");
            }
            write_results(problem_instance, short_instance_name, solution);
        } else
        {
            LOG_F(INFO, "no solution");
            exit(1);
        }
    }
}

static void load_and_run(const std::string &program_task, const std::string &program_task_conf)
{
    try
    {
        std::ifstream conf_file(program_task_conf);
        PPK_ASSERT_ERROR(conf_file.is_open(), "Failed to open the config file");
        std::string content((std::istreambuf_iterator<char>(conf_file)), (std::istreambuf_iterator<char>()));
        conf_file.close();
        PPK_ASSERT_ERROR(!content.empty(), "Content is empty or invalid");

        if (program_task == "generator")
        {
            run_generator(content);
        } else if (program_task == "solver")
        {
            run_solver(content);
        }
    } catch (const std::exception &e)
    {
        LOG_F(ERROR, "Exception: %s", e.what());
        exit(1);
    }
}

int main(int argc, char *argv[])
{
    std::string program_task;
    std::string program_task_conf;
    std::string verbosity_level;
    parse_command_line(program_task, program_task_conf, verbosity_level, argc, argv);
    set_logger_verbosity(verbosity_level);
    load_and_run(program_task, program_task_conf);
    return 0;
}