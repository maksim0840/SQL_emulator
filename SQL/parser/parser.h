#pragma once

#include "sql_helper.h"

class Parser {
private:
	std::string input; // переданная команда
	size_t pos; // текущая исследуемая позиция в команде
    size_t sz; // размер команды
    Sql db;

    bool is_sep();
    void skip_spaces();
    bool skip_comma(); // тот же skip_spaces, но если есть ещё и запятая то её тоже сикпает (возвращает true если запятая была)
	std::string to_lower(const std::string& str);

    std::string expect_name(bool throw_exceptions = true); // читает слово, но не допускает наличие доп символов (цифры и нижнее подчёркивание)
    std::string expect_extended_name(bool throw_exceptions = true); // expect_name но разрешает доп символы
    int expect_value(bool throw_exceptions = true); // читает число до любого не цифрового символа
    std::string expect_command(); // expect_name() c приведением к lower_case
    bool expect_keyword(const std::string& keyword, bool throw_exceptions = true); // <=> expect_command с проверкой на конкретную строку
    void expect_ending(); // поиск ';'

    // Прочитать значения разных типов в строку
    int read_int32_str();
    bool read_bool_str();
    std::string read_string_str(const size_t max_size);
    std::string read_bytes_str(const size_t max_size, bool can_be_smaller = false);

    // Для create table
    void expect_label_attributes(Sql::ColumnLabel* label);
    void expect_label_name(Sql::ColumnLabel* label);
    int expect_label_size();
    void expect_label_type(Sql::ColumnLabel* label);
    void expect_label_default(Sql::ColumnLabel* label);
    std::vector<Sql::ColumnLabel> expect_label();
    void determine_presets_by_attributes(std::vector<Sql::ColumnLabel>* labels); // проверяет корректность использования атрибутов и добавляет некоторые автоматические определния
    void check_for_dublicated_columns(const std::vector<Sql::ColumnLabel>& labels);

    // Для create index
    Sql::IndexType expect_index_type();

    // Для insert
    enum class InsertingType {
        BY_ORDER,
        BY_ASSIGNMENT
    };
    struct InsertingValueInfo {
        std::string name_ = "";
        variants value_;
        Sql::ValueType type_;
    };
    void expect_order_value(std::vector<InsertingValueInfo>* row_values);
    void expect_assignment_value(std::vector<InsertingValueInfo>* row_values);
    InsertingType determine_inserting_type(); // определить как задаются значения (по очереди (,,,,,) или прямым назначением (a=, b=, c=))
    InsertingType expect_row_values(std::vector<InsertingValueInfo>* row_values);
    std::unordered_map<std::string, variants> prepare_row_order_values(const std::vector<InsertingValueInfo>& old_row_values, const std::vector<Sql::ColumnLabel>& labels); // проверка введенных значений
    std::unordered_map<std::string, variants> prepare_row_assignment_values(const std::vector<InsertingValueInfo>& old_row_values, const std::vector<Sql::ColumnLabel>& labels); // проверка введенных значений
    
    // Для select
    std::unordered_set<std::string> expect_columns_names();

public:
	void execute(const std::string& str);
};
