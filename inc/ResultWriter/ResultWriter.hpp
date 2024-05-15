#pragma once

#include <map>
#include <string>
#include <vector>
#include <xlsxwriter.h>

class ResultWriter
{
  public:
    ResultWriter(const std::string &file_name, const std::vector<std::string> &header);
    ResultWriter(const ResultWriter &) = delete;
    ResultWriter &operator==(const ResultWriter &) = delete;
    ~ResultWriter();
    using ColumnName = std::string;
    using ColumnIndex = size_t;
    using ColumnValue = std::string;
    using Row = std::map<ColumnName, ColumnValue>;
    void write(const Row &row);

  private:
    void write_header_to_excel();
    using map1ton = std::map<std::string, std::vector<std::string>>;
    std::map<ColumnIndex, ColumnName> column_map;
    lxw_workbook *workbook;
    lxw_worksheet *worksheet;
    size_t current_row_index = 0;
};
