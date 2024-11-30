#pragma once
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <variant>
#include <optional>
#include <unordered_set>

#include "fstream_with_exceptions.h"
#include "sql_exception.h"

template <typename Key, typename Value>
using umap = std::unordered_map<Key, Value>;
template <typename Key, typename Value>
using map = std::map<Key, Value>;

using variants = std::variant<std::monostate, int, bool, std::string>;
using OrderedIndex = umap<std::string, umap<std::string, map<variants, std::vector<size_t>>>>;
using UnorderedIndex = umap<std::string, umap<std::string, umap<variants, std::vector<size_t>>>>;

class Parser; // для парсера
class Tests; // для тестов


class Sql {
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
        size_t value_default_size; // фактический размер deafault значения (0 => null)
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

    Sql(const std::string& data_dir);
    ~Sql();

    void create_table(const std::string& talbe_name, const std::vector<ColumnLabel>& labels);
    void create_index(const std::string& table_name, const std::string& column_name, const IndexType index_type);
    void insert(const std::string& talbe_name, const std::unordered_map<std::string, variants>& row_values);
    void select(const std::unordered_set<std::string>& columns, const std::string& table_name);
    void delete_table(const std::string& table_name);

private:
    friend class Parser;
    friend class Tests;
    
    class Helper; // вложенный класс для вспомогательных функций, обеспечивающих работу основных
    Helper* helper_; // указатель на объект класса Helper для обращения к его методам
    std::string data_dir_;

    #pragma pack(push, 1) // отключить выравнивание
    struct RowsPositions {
        size_t start; // указывает на начало секции с данными
        std::optional<size_t> last; // указывает на начало последней строки (nullopt - нет строк вообще)
    };
    #pragma pack(pop)

    // Информация о таблице
    std::unordered_map<std::string, std::vector<ColumnLabel>> table_labels; // храним заголовки и служебную информацию о всех столбцах из каждой таблицы
    std::unordered_map<std::string, RowsPositions> table_positions; // храним позиции начальной и конечной строки таблицы
    // Индексы таблиц: обращение имеет вид index[имя таблицы][имя столбца][значение] = вектор позиций начала строк с этим значением в таблице
    OrderedIndex* table_ordered_indexes; 
    UnorderedIndex* table_unordered_indexes;

    // Обновление информации о таблице (используется в основном чтобы вспомнить параметры таблиц при создании класса в конструкторе)
    size_t read_table_labels(const std::string& table_name, std::vector<ColumnLabel>* labels); // обновляет информацию о столбцах таблицы
    void read_table_positions(const std::string& table_name, const size_t end_labels_pos, RowsPositions* positions); // обновляет инфомрацию о позициях первой и последней строки
    void read_table_indexes(const std::string& table_name, OrderedIndex* ordered_indexes, UnorderedIndex* unordered_indexes); // обновляет индексы
};