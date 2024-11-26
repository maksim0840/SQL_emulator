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
template <typename Key, typename Value>
using map = std::map<Key, Value>;

using variants = std::variant<std::monostate, int, bool, std::string>;
using OrderedIndex = umap<std::string, umap<std::string, map<variants, std::vector<size_t>>>>;
using UnorderedIndex = umap<std::string, umap<std::string, umap<variants, std::vector<size_t>>>>;

class Parser;
class Tests;


class Sql { // полностью приватный (запросы через парсер)
public:

    // Доступные типы значений столбцов
    enum class ValueType{
        INT32,
        BOOL,
        STRING,
        BYTES
    };

    // Доступные типы индексов
    enum class IndexType{
        ORDERED,
        UNORDERED
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
        // indexes
        bool is_ordered;
        bool is_unordered;
    };
    #pragma pack(pop)

    Sql();
    ~Sql();

    void create_table(const std::string& talbe_name, const std::vector<ColumnLabel>& labels);
    void create_index(const std::string& table_name, const std::string& column_name, const IndexType index_type);
    void insert(const std::string& talbe_name, const std::unordered_map<std::string, variants> row_values);
    
private:
    friend class Parser;
    friend class Tests;

    #pragma pack(push, 1) // отключить выравнивание
    struct RowsPositions {
        size_t start; // указывает на начало секции со строками в таблице
        std::optional<size_t> last; // указывает на начало последней строки (nullopt - нет строк вообще)
    };
    #pragma pack(pop)

    // Информация о таблице
    std::unordered_map<std::string, std::vector<ColumnLabel>> table_labels; // храним заголовки и служебную информацию о всех столбцах из каждой таблицы
    std::unordered_map<std::string, RowsPositions> table_positions; // храним позиции начальной и конечной строк таблицы из каждой таблицы
    // Индексы таблиц (обращение вида) index[имя таблицы][имя столбца][значение] = вектор позиций строк с этим значением в таблице>
    OrderedIndex* table_ordered_indexes; 
    UnorderedIndex* table_unordered_indexes;

    size_t read_table_labels(const std::string& table_name, std::vector<ColumnLabel>* labels);
    void read_table_positions(const std::string& table_name, const size_t end_labels_pos, RowsPositions* positions); // обновляет инфомрацию о позициях первой и последней строки
    void read_table_indexes(const std::string& table_name, OrderedIndex* ordered_indexes, UnorderedIndex* unordered_indexes); // обновляет индексы

    // Вспомогательные
    size_t count_bytes_in_row(const std::string table_name);
    void write_variants_value(const variants value, const size_t value_size, FileStreamWithExceptions* table);
    variants read_variants_value(const ValueType value_type, const size_t value_size, const size_t value_max_size, FileStreamWithExceptions* table); // читает из файла значение переменной std::variants
    bool check_unique(const variants cmp_val, const std::string& column_name, const std::string& table_name, FileStreamWithExceptions* table); // проверяет на уникальность значения cmp_val в столбце column_name
    
};