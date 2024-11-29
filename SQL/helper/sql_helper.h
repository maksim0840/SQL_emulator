#pragma once

#include "sql.h"

class Sql::Helper {
public:
    size_t count_bytes_in_row(const std::string& table_name, const bool check_real_data = true);
    size_t count_rows_in_table(const std::string& table_name);
    void write_variants_value(const variants value, const size_t value_size, FileStreamWithExceptions* table);
    variants read_variants_value(const ValueType value_type, const size_t value_size, const size_t value_max_size, FileStreamWithExceptions* table);
    size_t get_variants_size(const variants value);
    std::string convert_variants_for_output(const variants value, const ValueType value_type);
    std::vector<std::string> get_label_names(const std::string& table_name, const std::unordered_set<std::string>& columns = {}, std::vector<size_t>* paddings = nullptr);
    std::vector<std::vector<variants>> get_data_rows(const std::unordered_set<std::string>& columns, const std::string& table_name, std::vector<Sql::ValueType>* value_types = nullptr);
    std::vector<std::pair<size_t, variants>> get_checked_values_to_insert(const std::string& table_name, const std::unordered_map<std::string, variants>& row_values, FileStreamWithExceptions* table);
    bool check_unique(const variants cmp_val, const std::string& column_name, const std::string& table_name, FileStreamWithExceptions* table);
    
    Helper(Sql* sql);
    Sql* sql_;
};  
