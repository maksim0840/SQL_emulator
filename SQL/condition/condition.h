#pragma once

#include "sql.h"

class Condition {
private:
    // Хранит мапу эталонных значений строки {имя столбца : значение}
    std::unordered_map<std::string, variants> manual_;

public:
    // Проверяет соответствует ли переданная строка заданным эталонным значениям
    bool check(const std::unordered_map<std::string, variants>& cmp_row);

    Condition(const std::vector<Sql::ColumnLabel>& table_labels, std::unordered_map<std::string, variants> manual);
    ~Condition();
    
};