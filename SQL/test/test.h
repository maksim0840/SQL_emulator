#pragma once

#include "../parser/parser.cpp"

class Tests {
public:
    static bool check_labels_in_table(const std::vector<Sql::ColumnLabel>& labels_for_cmp, const std::string& table_name);
};