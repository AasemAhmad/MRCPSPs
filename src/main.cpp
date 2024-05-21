#include "External/ILPSolverModel/ILPSolverInterface.hpp"
#include "External/cxxopts.hpp"
#include "ILPOptimizationModel/ProblemSolverILP.hpp"
#include "InstanceGenerator/InstanceGenerator.hpp"
#include "InstanceReader/InstanceReader.hpp"
#include "ProblemInstance/ProblemInstance.hpp"
#include "Settings.hpp"
#include "Solution/SolutionChecker.hpp"
#include <Shared/Utils.hpp>
#include <filesystem>
#include <format>
#include <fstream>
#include <loguru.hpp>
#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include <set>
#include <vector>

static int g_argc;
static char **g_argv;

static const std::set<std::string, std::less<>> program_tasks_set = {"solver", "generator"};
static const std::set<std::string, std::less<>> verbosity_levels_set = {"debug", "info", "quiet", "silent"};

static void parse_command_line(std::string &program_task, std::string &program_task_conf, std::string &verbosity_level,
                               std::string &program_task_conf_path)

{

    cxxopts::Options options("MultiReourceProjectScheduler",
                             "Sheduling Algorithms for Multiple Resource-Constrainted Project");
    try
    {
        options.add_options()("program_task", "Programm task", cxxopts::value<std::string>(program_task))(
            "program_task_conf", "Programm task configuration", cxxopts::value<std::string>(program_task_conf))(
            "verbosity", "Verbosity level", cxxopts::value<std::string>(verbosity_level));

        auto result = options.parse(g_argc, g_argv);

        PPK_ASSERT_ERROR(program_tasks_set.find(program_task) != program_tasks_set.end(), "Invalid program task");
        PPK_ASSERT_ERROR(verbosity_levels_set.find(verbosity_level) != verbosity_levels_set.end(),
                         "Invalid verbosity level");
        PPK_ASSERT_ERROR(program_task_conf.ends_with(".json"), "Invalid config file format");

        std::filesystem::path path = std::filesystem::current_path().parent_path().parent_path() / "conf";
        program_task_conf_path = path.c_str() + std::string("/") + program_task_conf;
        PPK_ASSERT_ERROR(std::filesystem::exists(program_task_conf_path), "Config file does not exist");
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

    PPK_ASSERT_ERROR(json_doc_generator_options.HasMember("instances_directory_path"));
    LOG_F(INFO, "instances_directory_path =  %s", json_doc_generator_options["instances_directory_path"].GetString());
    Settings::INSTANCES_DIRECTORY_PATH = json_doc_generator_options["instances_directory_path"].GetString();

    PPK_ASSERT_ERROR(json_doc_generator_options.HasMember("nb_resources"));
    LOG_F(INFO, "nb_resources =  %d", json_doc_generator_options["nb_resources"].GetUint());
    Settings::Generator::NB_RESOURCES = json_doc_generator_options["nb_resources"].GetUint();

    PPK_ASSERT_ERROR(json_doc_generator_options.HasMember("min_resource_capacity"));
    LOG_F(INFO, "min_resource_capacity =  %d", json_doc_generator_options["min_resource_capacity"].GetUint());
    Settings::Generator::MIN_RESOURCE_CAPACITY = json_doc_generator_options["min_resource_capacity"].GetUint();

    PPK_ASSERT_ERROR(json_doc_generator_options.HasMember("max_resource_capacity"));
    LOG_F(INFO, "max_resource_capacity =  %d", json_doc_generator_options["max_resource_capacity"].GetUint());
    Settings::Generator::MAX_RESOURCE_CAPACITY = json_doc_generator_options["max_resource_capacity"].GetUint();

    PPK_ASSERT_ERROR(json_doc_generator_options.HasMember("nb_jobs"), "nb_jobs field is not found");
    LOG_F(INFO, "nb_jobs =  %d", json_doc_generator_options["nb_jobs"].GetUint());
    Settings::Generator::NB_JOBS = json_doc_generator_options["nb_jobs"].GetUint();

    PPK_ASSERT_ERROR(json_doc_generator_options.HasMember("min_nb_succ"), "min_nb_succ field is not found");
    LOG_F(INFO, "min_nb_succ =  %d", json_doc_generator_options["min_nb_succ"].GetUint());
    Settings::Generator::MIN_NB_SUCCESSORS = json_doc_generator_options["min_nb_succ"].GetUint();

    PPK_ASSERT_ERROR(json_doc_generator_options.HasMember("max_nb_succ"), "max_nb_succ field is not found");
    LOG_F(INFO, "max_nb_succ =  %d", json_doc_generator_options["max_nb_succ"].GetUint());
    Settings::Generator::MAX_NB_SUCCESSORS = json_doc_generator_options["max_nb_succ"].GetUint();

    PPK_ASSERT_ERROR(json_doc_generator_options.HasMember("max_nb_modes"));
    LOG_F(INFO, "max_nb_modes =  %d", json_doc_generator_options["max_nb_modes"].GetUint());
    Settings::Generator::MAX_NB_MODES = json_doc_generator_options["max_nb_modes"].GetUint();

    PPK_ASSERT_ERROR(json_doc_generator_options.HasMember("min_nb_modes"));
    LOG_F(INFO, "min_nb_modes =  %d", json_doc_generator_options["min_nb_modes"].GetUint());
    Settings::Generator::MIN_NB_MODES = json_doc_generator_options["min_nb_modes"].GetUint();

    PPK_ASSERT_ERROR(json_doc_generator_options.HasMember("max_execution_time"));
    LOG_F(INFO, "max_execution_time =  %d", json_doc_generator_options["max_execution_time"].GetUint());
    Settings::Generator::MAX_EXECUTION_TIME = json_doc_generator_options["max_execution_time"].GetUint();

    PPK_ASSERT_ERROR(json_doc_generator_options.HasMember("min_execution_time"));
    LOG_F(INFO, "min_execution_time =  %d", json_doc_generator_options["min_execution_time"].GetUint());
    Settings::Generator::MIN_EXECUTION_TIME = json_doc_generator_options["min_execution_time"].GetUint();

    PPK_ASSERT_ERROR(json_doc_generator_options.HasMember("max_nb_resouce_units_per_job"));
    LOG_F(INFO, "max_nb_resouce_units_per_job =  %d",
          json_doc_generator_options["max_nb_resouce_units_per_job"].GetUint());
    Settings::Generator::MAX_NB_RESOURCE_UNITS_PER_JOB =
        json_doc_generator_options["max_nb_resouce_units_per_job"].GetUint();

    PPK_ASSERT_ERROR(json_doc_generator_options.HasMember("min_nb_resource_units_per_job"));
    LOG_F(INFO, "min_nb_resource_units_per_job =  %d",
          json_doc_generator_options["min_nb_resource_units_per_job"].GetUint());
    Settings::Generator::MIN_NB_RESOURCE_UNITS_PER_JOB =
        json_doc_generator_options["min_nb_resource_units_per_job"].GetUint();

    PPK_ASSERT_ERROR(json_doc_generator_options.HasMember("first_instance_index"));
    LOG_F(INFO, "first_instance_index =  %d", json_doc_generator_options["first_instance_index"].GetUint());
    Settings::FIRST_INSTANCE_INDEX = json_doc_generator_options["first_instance_index"].GetUint();

    PPK_ASSERT_ERROR(json_doc_generator_options.HasMember("last_instance_index"));
    LOG_F(INFO, "last_instance_index =  %d", json_doc_generator_options["last_instance_index"].GetUint());
    Settings::LAST_INSTANCE_INDEX = json_doc_generator_options["last_instance_index"].GetUint();

    return true;
}

static bool parse_solver_option_parameters(const std::string &solver_options)
{
    rapidjson::Document json_doc_solver_options;
    json_doc_solver_options.Parse(solver_options.c_str());
    PPK_ASSERT_ERROR(json_doc_solver_options.IsObject(), "Invalid solver options: %s Not a JSON object",
                     solver_options.c_str());

    PPK_ASSERT_ERROR(json_doc_solver_options.HasMember("instance_file_name"));
    LOG_F(INFO, "instance_file_name =  %s", json_doc_solver_options["instance_file_name"].GetString());
    Settings::INSTANCE_NAME = json_doc_solver_options["instance_file_name"].GetString();

    PPK_ASSERT_ERROR(json_doc_solver_options.HasMember("instances_directory_path"));
    LOG_F(INFO, "instances_directory_path =  %s", json_doc_solver_options["instances_directory_path"].GetString());
    Settings::INSTANCES_DIRECTORY_PATH = json_doc_solver_options["instances_directory_path"].GetString();

    PPK_ASSERT_ERROR(json_doc_solver_options.HasMember("first_instance_index"));
    LOG_F(INFO, "first_instance_index =  %d", json_doc_solver_options["first_instance_index"].GetUint());
    Settings::FIRST_INSTANCE_INDEX = json_doc_solver_options["first_instance_index"].GetUint();

    PPK_ASSERT_ERROR(json_doc_solver_options.HasMember("last_instance_index"));
    LOG_F(INFO, "last_instance_index =  %d", json_doc_solver_options["last_instance_index"].GetUint());
    Settings::LAST_INSTANCE_INDEX = json_doc_solver_options["last_instance_index"].GetUint();

    PPK_ASSERT_ERROR(json_doc_solver_options.HasMember("results_directory"));
    LOG_F(INFO, "results_directory =  %s", json_doc_solver_options["results_directory"].GetString());
    Settings::Solver::RESULTS_DIRECTORY = json_doc_solver_options["results_directory"].GetString();

    PPK_ASSERT_ERROR(json_doc_solver_options.HasMember("nb_of_thread"));
    LOG_F(INFO, "nb_of_thread =  %d", json_doc_solver_options["nb_of_thread"].GetUint());
    Settings::Solver::NB_THREADS = json_doc_solver_options["nb_of_thread"].GetUint();

    PPK_ASSERT_ERROR(json_doc_solver_options.HasMember("use_gurobi") ||
                     json_doc_solver_options.HasMember("use_cplex") || json_doc_solver_options.HasMember("use_cp"));

    if (json_doc_solver_options.HasMember("use_gurobi"))
    {
        LOG_F(INFO, "use_gurobi =  %s", json_doc_solver_options["use_gurobi"].GetBool() ? "true" : "false");
        Settings::Solver::USE_GUROBI = json_doc_solver_options["use_gurobi"].GetBool();
    }

    if (json_doc_solver_options.HasMember("use_cplex"))
    {
        LOG_F(INFO, "use_cplex =  %s", json_doc_solver_options["use_cplex"].GetBool() ? "true" : "false");
        Settings::Solver::USE_CPLEX = json_doc_solver_options["use_cplex"].GetBool();
    }

    if (json_doc_solver_options.HasMember("use_cp"))
    {
        LOG_F(INFO, "use_cp =  %s", json_doc_solver_options["use_cp"].GetBool() ? "true" : "false");
        Settings::Solver::USE_CP = json_doc_solver_options["use_cp"].GetBool();
    }

    if (json_doc_solver_options.HasMember("check_solution"))
    {
        LOG_F(INFO, "check_solution =  %s", json_doc_solver_options["check_solution"].GetBool() ? "true" : "false");
        Settings::Solver::CHECK_SOLUTION = json_doc_solver_options["check_solution"].GetBool();
    }

    if (json_doc_solver_options.HasMember("draw_gantt_chart"))
    {
        LOG_F(INFO, "draw_gantt_chart =  %s", json_doc_solver_options["draw_gantt_chart"].GetBool() ? "true" : "false");
        Settings::Solver::DRAW_GANTT_CHART = json_doc_solver_options["draw_gantt_chart"].GetBool();
    }

    PPK_ASSERT_ERROR(json_doc_solver_options.HasMember("max_runtime"));
    LOG_F(INFO, "max_runtime =  %d", json_doc_solver_options["max_runtime"].GetUint());
    Settings::Solver::MAX_RUNTIME = json_doc_solver_options["max_runtime"].GetUint();

    PPK_ASSERT_ERROR(json_doc_solver_options.HasMember("init_ilp_solution"));
    LOG_F(INFO, "init_ilp_solution =  %s", json_doc_solver_options["init_ilp_solution"].GetBool() ? "ture" : "false");
    Settings::Solver::INIT_ILP_SOLUTION = json_doc_solver_options["init_ilp_solution"].GetBool();

    PPK_ASSERT_ERROR(json_doc_solver_options.HasMember("ilp_relative_gap"));
    LOG_F(INFO, "ilp_relative_gap =  %f", json_doc_solver_options["ilp_relative_gap"].GetDouble());
    Settings::Solver::ILP_RELATIVE_GAP = json_doc_solver_options["ilp_relative_gap"].GetDouble();

    return true;
}

static void run_generator(const std::string &programm_task_options)
{
    // FIXME
    std::filesystem::create_directory("../../Instances");

    if (parse_generator_option_parameters(programm_task_options))
    {

        for (size_t index = Settings::FIRST_INSTANCE_INDEX; index <= Settings::LAST_INSTANCE_INDEX; ++index)
        {
            std::filesystem::create_directory(Settings::INSTANCES_DIRECTORY_PATH);
            std::string fileName = std::format("{}instance_{}.json", Settings::INSTANCES_DIRECTORY_PATH, index);
            LOG_F(INFO, "file name = %s", fileName.c_str());
            InstanceGenerator generator(fileName);
            generator.generate();
        }
    }
}

static bool run_solution_checker(const ProblemInstance &problem_instance, const Solution &solution)
{
    SolutionChecker checker(problem_instance, solution);
    PPK_ASSERT_ERROR(checker.check_solution(), "Solution Checker Faild");
    return true;
}

static void write_results(const ProblemInstance &problem_instance, const std::string &short_instance_name,
                          Solution &solution)
{
    std::string solution_file = std::format("{}instances_solution.xlsx", Settings::Solver::RESULTS_DIRECTORY);
    std::string statistic_file = std::format("{}statistics.xlsx", Settings::Solver::RESULTS_DIRECTORY);

    LOG_F(INFO, "solution file = %s", solution_file.c_str());
    write_solution_to_excel_file(solution_file, statistic_file, short_instance_name, solution);

    LOG_F(INFO, "%s", solution.get_solution_as_string().c_str());
    if (Settings::Solver::DRAW_GANTT_CHART)
    {
        solution.inverse_allocated_resouce_units(problem_instance);
        draw_gantt_chart_from_json(solution.job_allocations, problem_instance.resources);
    }
}

static void run_solver(const std::string &programm_task_options)
{
    parse_solver_option_parameters(programm_task_options);
    std::filesystem::create_directories(Settings::Solver::RESULTS_DIRECTORY);

    for (size_t index = Settings::FIRST_INSTANCE_INDEX; index <= Settings::LAST_INSTANCE_INDEX; ++index)
    {
        const std::string short_instance_name = std::format("{}_{}.json", Settings::INSTANCE_NAME, index);

        ProblemInstance problem_instance(std::format("{}{}", Settings::INSTANCES_DIRECTORY_PATH, short_instance_name));

        InstanceReader reader(problem_instance.name);
        reader.read(problem_instance);
        Solution solution;
        if (Settings::Solver::USE_GUROBI || Settings::Solver::USE_CPLEX)
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
            // TODO
        }

        if (solution.solution_state == SolutionState::FEASIBLE || solution.solution_state == SolutionState::OPTIMAL)
        {
            if (Settings::Solver::CHECK_SOLUTION)
            {
                PPK_ASSERT_ERROR(run_solution_checker(problem_instance, solution), "Wrong Solution");
            }
            write_results(problem_instance, short_instance_name, solution);
        }
    }
}

static void load_and_run(const std::string &program_task, const std::string &program_task_conf_path)
{
    try
    {
        std::ifstream conf_file(program_task_conf_path);
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
    std::string program_task_conf_path;
    g_argc = argc;
    g_argv = argv;
    parse_command_line(program_task, program_task_conf, verbosity_level, program_task_conf_path);
    set_logger_verbosity(verbosity_level);
    load_and_run(program_task, program_task_conf_path);
    return 0;
}