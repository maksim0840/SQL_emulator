#pragma once

#include "../executing/basic_operations.cpp"
#include <cctype>

class Parser {
private:
	std::string input; // переданная команда
	size_t pos; // текущая исследуемая позиция в команде
    size_t sz; // размер команды
    Sql db;

    bool is_sep();
    void skip_spaces();
    bool skip_comma(); // skip_spaces но если есть ещё и запятая то её тоже сикпает (возвращает true если запятая была)
	std::string to_lower(const std::string& str);

    std::string expect_name(bool throw_exceptions); // читает слово, но не допускает наличие доп символов (цифры и нижнее подчёркивание)
    std::string expect_extended_name(bool throw_exceptions); // expect_name но разрешает доп символы
    //std::vector<std::string> expect_names_before_keyword(const std::string& keyword);
    int expect_value(bool throw_exceptions); // читает число до любого не цифрового символа
    std::string expect_command(); // expect_name() c lower_case
    bool expect_keyword(const std::string& keyword, bool throw_exceptions); // <=> expect_command с проверкой на конкретную строку
    void expect_ending(); // поиск ';'

    // Прочитать значения разных типов в строку
    int read_int32_str();
    bool read_bool_str();
    std::string read_string_str(const size_t max_size);
    std::string read_bytes_str(const size_t max_size);

    // для create table
    void expect_label_attributes(Sql::ColumnLabel* label);
    void expect_label_name(Sql::ColumnLabel* label);
    int expect_label_size();
    void expect_label_type(Sql::ColumnLabel* label);
    void expect_label_default(Sql::ColumnLabel* label);
    std::vector<Sql::ColumnLabel> expect_label();
    void check_for_correct_attributes(std::vector<Sql::ColumnLabel>* labels); // проверяется совместимость аттрибутов и default значения (+ если есть аттрибут key, то выставляется ещё и unique)
    void determine_indexes_by_attributes(std::vector<Sql::ColumnLabel>* labels); // выставляем ordered index если присутствует аттрибут key
    void check_for_dublicated_columns(const std::vector<Sql::ColumnLabel>& labels);

    // для create index
    Sql::IndexType expect_index_type();

public:
	void execute(const std::string& str);
};
