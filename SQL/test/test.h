#pragma once

#include "parser.h"

class Tests {
public:
    static bool check_labels_in_table(const std::vector<Sql::ColumnLabel>& labels_for_cmp, const std::string& table_name);
    static bool check_index_in_table(const Sql::IndexType index_type, const std::string& column_name, const std::string& table_name);
    static bool check_rows_in_table(const std::vector<std::vector<variants>>& rows_cmp, const std::string& table_name);
};