#pragma once

#include "fstream_with_exceptions.h"
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <variant>
#include <optional>


template <typename Key, typename Value>
using umap = std::unordered_map<Key, Value>;

using variants = std::variant<std::monostate, int, bool, std::string>;
using index_t = umap<std::string, umap<std::string, std::map<variants, std::vector<size_t>>>>;

class Parser;
class Tests;

class Sql {
private:
    friend class Parser;
    friend class Tests;

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
        variants value_default;
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
    std::unordered_map<std::string, RowsPositions> table_positions; // храним позиции начальной и конечной строк таблицы из каждой таблицы
    index_t* table_indexes; // Индексы таблиц (обращение вида) index[имя таблицы][имя столбца][значение] = вектор позиций строк с этим значением в таблице>

    variants read_variants_value(const size_t value_size, const ValueType value_type, FileStreamWithExceptions* table); // читает из файла значение переменной std::variants
    size_t read_table_labels(const std::string& table_name, std::vector<ColumnLabel>* labels);
    /* read_table_values */ 
    size_t read_table_positions(const std::string& table_name, const size_t end_labels_pos, RowsPositions* positions); // обновляет инфомрацию о позициях первой и последней строки
    void read_table_indexes(const std::string& table_name, const size_t bytes_in_row, index_t* indexes); // обновляет индексы

public:
    Sql();
    ~Sql();

    void create_table(const std::string& talbe_name, const std::vector<ColumnLabel>& labels);

    void insert(const std::string& talbe_name, const std::vector<variants>& input_values);
};