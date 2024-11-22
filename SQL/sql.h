#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <variant>

using variants = std::variant<std::monostate, int, bool, std::string>;

class Sql {
private:
    // Доступные типы значений столбцов
    enum class ValueType{
        INT32,
        BOOL,
        STRING,
        BYTES
    };

    // Структура для описания служебной информации столбцов
    #pragma pack(push, 1) // отключить выравнивание
    struct ColumnLabel {
        size_t name_size;
        std::string name;
        ValueType value_type;
        size_t value_default_size;
        std::string value_default;
        // attributes
        bool is_unique;
        bool is_autoincrement;
        bool is_key;
    };
    #pragma pack(pop)

    // храним заголовки и служебную информацию о всех столбцах из каждой таблицы
    std::unordered_map<std::string, std::vector<ColumnLabel>> table_labels;

    // обновить заголовки и служебную информацию для talbe_name таблицы
    void update_table_label(const std::string& talbe_name);

public:

    Sql();
    //~Sql();

    void create_table(const std::string& talbe_name, const std::vector<ColumnLabel>& labels);

    void insert(const std::string& talbe_name, const std::vector<variants>& input_values);
};
