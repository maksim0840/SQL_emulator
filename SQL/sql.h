#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <variant>
#include <optional>

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
        size_t value_max_size; // максимальный размер типа
        size_t value_default_size; // размер deafault значения (0 => null)
        std::string value_default;
        // attributes
        bool is_unique;
        bool is_autoincrement;
        bool is_key;
    };
    struct RowsPositions {
        size_t start; // указывает на начало секции со строками в таблице
        std::optional<size_t> last; // указывает на начало последней строки (nullopt - нет строк вообще)
    };
    #pragma pack(pop)

    // Информация о таблице
    std::unordered_map<std::string, std::vector<ColumnLabel>> table_labels; // храним заголовки и служебную информацию о всех столбцах из каждой таблицы
    std::unordered_map<std::string, RowsPositions> table_rows; // храним позиции начальной и конечной строк таблицы из каждой таблицы

    void update_table_info(const std::string& talbe_name); // обновляет информацию о таблице

public:

    Sql();
    //~Sql();

    void create_table(const std::string& talbe_name, const std::vector<ColumnLabel>& labels);

    void insert(const std::string& talbe_name, const std::vector<variants>& input_values);
};