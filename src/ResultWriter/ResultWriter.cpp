#include "ResultWriter/ResultWriter.hpp"
#include "External/pempek_assert.hpp"

ResultWriter::ResultWriter(const std::string &file_name, const std::vector<std::string> &header)
{
    workbook = workbook_new(file_name.c_str());
    worksheet = workbook_add_worksheet(workbook, nullptr);

    worksheet_set_column(worksheet, 0, 255, 20, nullptr);

    size_t col_index = 0;
    for (const auto &item : header)
    {
        column_map[col_index] = item;
        ++col_index;
    }

    write_header_to_excel();
}

ResultWriter::~ResultWriter() { workbook_close(workbook); }

void ResultWriter::write_header_to_excel()
{
    for (const auto &[column_index, column_name] : column_map)
    {
        worksheet_write_string(worksheet, static_cast<unsigned short>(current_row_index),
                               static_cast<unsigned short>(column_index), column_name.c_str(), nullptr);
    }
    ++current_row_index;
}

void ResultWriter::write(const ResultWriter::Row &&row)
{
    for (const auto &[column_index, column_name] : column_map)
    {
        size_t col_index = column_index;
        const std::string &col_name = column_name;
        PPK_ASSERT_ERROR(row.count(col_name) == 1, "Column %s does not exist", col_name.c_str());
        worksheet_write_string(worksheet, static_cast<unsigned short>(current_row_index),
                               static_cast<unsigned short>(col_index), row.at(col_name).c_str(), nullptr);
    }
    ++current_row_index;
}
