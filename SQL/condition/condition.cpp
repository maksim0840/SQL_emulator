#include "condition.h"

// Проверяет соответствует ли переданная строка заданным эталонным значениям
bool Condition::check(const std::unordered_map<std::string, variants>& cmp_row) {
    for (const auto& [key, value] : manual_) {
        if (cmp_row.at(key) != value) {
            return false;
        }
    }
    return true;
}

// Конструктор
Condition::Condition(const std::vector<Sql::ColumnLabel>& table_labels, std::unordered_map<std::string, variants> manual) {
    // Сохраняет все названия столбцов таблицы
    std::unordered_set<std::string> columns_names;
    for (const auto& label : table_labels) {
        columns_names.insert(label.name);
    }
    auto not_exist = columns_names.end();

    // Проверяет, правда ли, что все имена столбцов из manual присутсвуют в table_name таблице
    for (const auto& [key, value] : manual) {
        if (columns_names.find(key) == not_exist) {
            throw "cant find column name in table";
        }
    }
    manual_ = manual;
}

