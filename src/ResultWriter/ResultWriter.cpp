#include "ResultWriter/ResultWriter.hpp"
#include "External/pempek_assert.hpp"

ResultWriter::ResultWriter(const std::string &file_name, const std::vector<std::string> &header)
{
    workbook = workbook_new(file_name.c_str());
    worksheet = workbook_add_worksheet(workbook, NULL);

    worksheet_set_column(worksheet, 0, 255, 20, NULL);

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
    for (const auto &column : column_map)
    {
        worksheet_write_string(worksheet, current_row_index, column.first, column.second.c_str(), NULL);
    }
    ++current_row_index;
}

void ResultWriter::write(const ResultWriter::Row &row)
{
    for (const auto &pair : column_map)
    {
        size_t col_index = pair.first;
        const std::string &col_name = pair.second;
        PPK_ASSERT_ERROR(row.count(col_name) == 1, "Column %s does not exist", col_name.c_str());
        worksheet_write_string(worksheet, current_row_index, col_index, row.at(col_name).c_str(), NULL);
    }
    ++current_row_index;
}
