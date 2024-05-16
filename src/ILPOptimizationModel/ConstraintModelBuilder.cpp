#include "ILPOptimizationModel/ConstraintModelBuilder.hpp"
#include "Shared/Utils.hpp"
#include <loguru.hpp>

void ConstraintModelBuilder::reset()
{
    constraint_matrix.clear();
    constraint_operator.clear();
    constraint_bound.clear();
    constraint_description.clear();
}

size_t ConstraintModelBuilder::move_constraints_to_model(ILPSolverModel &ilp_model)
{
    size_t nb_moved_constraints = constraint_matrix.get_nb_rows();

    for (auto &row : constraint_matrix)
    {
        ilp_model.matrix_A.add_row(std::move(row));
    }

    std::ranges::move(constraint_operator, std::back_inserter(ilp_model.vector_op));
    std::ranges::move(constraint_bound, std::back_inserter(ilp_model.vector_b));
    std::ranges::move(this->constraint_description, std::back_inserter(ilp_model.conDesc));

    reset();

    return nb_moved_constraints;
}

void ConstraintModelBuilder::add_job_proccessing_time_constraints(const TimeIndexedModelVariableMapping::map3to1 &x,
                                                                  const TimeIndexedModelVariableMapping::map1to1 &p)
{
    LOG_F(INFO, "%s started", __FUNCTION__);

    for (const auto &job : this->problem_instance.job_queue)
    {
        double b = 0.0;
        Operator op = Operator::EQUAL;
        SparseMatrix<double>::Row row;
        size_t mode_id = 1;
        for (const auto &mode : job->modes)
        {
            for (size_t t = 0; t < problem_instance.makespan_upperbound; ++t)
            {
                PPK_ASSERT_ERROR(mode_id <= job->modes.size(), "Invalid value %ld", mode_id);
                row.emplace_back(get_value(x, {job->j_id, std::to_string(mode_id), std::to_string(t)}, __FUNCTION__),
                                 mode.processing_time);
            }
            ++mode_id;
        }

        row.emplace_back(get_value(p, job->j_id, __FUNCTION__), -1.0);
        add_constraint(row, op, b, "JobProccessingTimeConstraints");
    }

    for (const auto &job : this->problem_instance.job_queue)
    {
        double b = 1.0;
        Operator op = Operator::EQUAL;
        SparseMatrix<double>::Row row;
        size_t mode_id = 1;
        for (const auto &mode : job->modes)
        {
            for (size_t t = 0; t < problem_instance.makespan_upperbound; ++t)
            {
                PPK_ASSERT_ERROR(mode_id <= job->modes.size(), "Invalid value %ld", mode_id);
                row.emplace_back(get_value(x, {job->j_id, std::to_string(mode_id), std::to_string(t)}, __FUNCTION__),
                                 1);
            }
            ++mode_id;
        }
        add_constraint(row, op, b, "JobStartTimeForSelectedModeConstraint");
    }
}

void ConstraintModelBuilder::add_job_start_time_constraints(const TimeIndexedModelVariableMapping::map1to1 &s,
                                                            const TimeIndexedModelVariableMapping::map3to1 &x,
                                                            const TimeIndexedModelVariableMapping::map1to1 &p,
                                                            const TimeIndexedModelVariableMapping::map1to1 &cMax)
{
    LOG_F(INFO, "%s started", __FUNCTION__);

    for (const auto &job : this->problem_instance.job_queue)
    {
        double b = 0.0;
        Operator op = Operator::LESS_EQUAL;
        SparseMatrix<double>::Row row = {{get_value(s, job->j_id, __FUNCTION__), 1.0},
                                         {get_value(p, job->j_id, __FUNCTION__), 1.0},
                                         {get_value(cMax, std::to_string(1), __FUNCTION__), -1.0}};
        add_constraint(row, op, b, "JobStartTimeConstraint");
    }

    for (const auto &job : this->problem_instance.job_queue)
    {
        double b = 0.0;
        Operator op = Operator::EQUAL;
        SparseMatrix<double>::Row row;
        for (size_t t = 0; t < problem_instance.makespan_upperbound; ++t)
        {
            size_t mode_id = 1;
            for (const auto &mode : job->modes)
            {
                PPK_ASSERT_ERROR(mode_id <= job->modes.size(), "Invalid value %ld", mode_id);
                row.emplace_back(get_value(x, {job->j_id, std::to_string(mode_id), std::to_string(t)}, __FUNCTION__),
                                 t);
                ++mode_id;
            }
        }
        row.emplace_back(get_value(s, job->j_id, __FUNCTION__), -1.0);
        add_constraint(row, op, b, "JobStartTimeConstraint");
    }

    LOG_F(INFO, "%s finished successfully", __FUNCTION__);
}

void ConstraintModelBuilder::add_precedence_constraints(const TimeIndexedModelVariableMapping::map1to1 &s,
                                                        const TimeIndexedModelVariableMapping::map1to1 &p)
{
    LOG_F(INFO, "%s started", __FUNCTION__);

    for (const auto &job : this->problem_instance.job_queue)
    {
        for (const auto &succ : job->successors)
        {
            double b = 0.0;
            Operator op = Operator::LESS_EQUAL;
            SparseMatrix<double>::Row row = {{get_value(s, job->j_id, __FUNCTION__), 1.0},
                                             {get_value(s, succ, __FUNCTION__), -1.0},
                                             {get_value(p, job->j_id, __FUNCTION__), 1.0}};
            add_constraint(row, op, b, "addprecedenceconstraints");
        }
    }

    LOG_F(INFO, "%s finished successfully", __FUNCTION__);
}

void ConstraintModelBuilder::add_renewable_resource_constraints(const TimeIndexedModelVariableMapping::map3to1 &x)
{
    LOG_F(INFO, "%s started", __FUNCTION__);

    size_t nb_resources = problem_instance.resources.size();

    for (size_t t = 0; t < problem_instance.makespan_upperbound; ++t)
    {
        for (size_t k = 0; k < nb_resources; ++k)
        {
            double b = this->problem_instance.resources.at(k).units;
            Operator op = Operator::LESS_EQUAL;
            SparseMatrix<double>::Row row;

            for (const auto &job : problem_instance.job_queue)
            {
                size_t mode_id = 1;
                for (const auto &mode : job->modes)
                {
                    PPK_ASSERT_ERROR(mode_id <= job->modes.size(), "Invalid Value %ld", mode_id);
                    int s_start = std::max(static_cast<int>(t) - static_cast<int>(mode.processing_time) + 1, 0);
                    PPK_ASSERT_ERROR(s_start >= 0, "invalid value %d", s_start);
                    for (size_t s = s_start; s <= t; ++s)
                    {
                        // FIXME
                        row.emplace_back(
                            get_value(x, {job->j_id, std::to_string(mode_id), std::to_string(s)}, __FUNCTION__),
                            mode.requested_resources.at(k).units);
                    }
                    ++mode_id;
                }
            }

            add_constraint(row, op, b, "RenewableResourceConstraint");
        }
    }

    LOG_F(INFO, "%s finished successfully", __FUNCTION__);
}

void ConstraintModelBuilder::add_constraint(SparseMatrix<double>::Row &row, Operator op, const double &b,
                                            const std::string &conDesc)
{
    this->constraint_matrix.add_row(std::move(row));
    this->constraint_operator.push_back(op);
    this->constraint_bound.push_back(b);
    this->constraint_description.push_back(conDesc);
}