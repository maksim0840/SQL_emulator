#include "../core/sql.h"
#include <string>
#include <filesystem>
#include <vector>
#include <stdexcept>
#include <variant>
#include <optional>
#include <iostream>

// Конструктор
Sql::Sql() {
    table_ordered_indexes = new OrderedIndex();
    table_unordered_indexes = new UnorderedIndex();
    const std::string dir_name = "../data";
    const auto dir = std::filesystem::directory_iterator(dir_name);

    // Цикл по всем файлам дирректории "data"
    for (const auto& entry : dir) {
        const std::string table_name = entry.path().filename();
        size_t last_read_pos = read_table_labels(table_name, &table_labels[table_name]);
        read_table_positions(table_name, last_read_pos, &table_positions[table_name]); // получить информацию о столбцах таблицы
        read_table_indexes(table_name, table_ordered_indexes, table_unordered_indexes); // сохранить индексы в кучу
    }
}

// Деструктор
Sql::~Sql() {
    delete table_ordered_indexes;
    delete table_unordered_indexes;
}

variants Sql::read_variants_value(const ValueType value_type, const size_t value_size, const size_t value_max_size, FileStreamWithExceptions* table) {
    if (value_size > value_max_size) {
        throw "value size overflow";
    }

    variants var;
    if (value_size == 0) {
        var = std::monostate{};
    }
    else if (value_type == ValueType::INT32) {
        int value;
        table->read_exc(reinterpret_cast<char*>(&value), value_size);
        var = value;
    }
    else if (value_type == ValueType::BOOL) {
        bool value;
        table->read_exc(reinterpret_cast<char*>(&value), value_size);
        var = value;
    }
    else {
        std::vector<char> buffer(value_size);
        table->read_exc(buffer.data(), value_size); // добавить строку, через копирование веткора
        var = std::string(buffer.begin(), buffer.end());
    }

    // дополнительно отступить, незаполненное пространство
    table->seekg(value_max_size - value_size, std::ios::cur);
    return var;
}

size_t Sql::read_table_labels(const std::string& table_name, std::vector<ColumnLabel>* labels) {

    const std::string table_path_name = "../data/" + table_name;
    // Проверка на наличие такой таблицы
    if (!std::filesystem::exists(table_path_name)) {
        throw std::ios_base::failure("Table does not exist\n");
    }
    
    // Открытие таблицы
    FileStreamWithExceptions table;
    table.open_exc(table_path_name, std::ios_base::binary | std::ios_base::in);

    // Чтение служебной информации о столбцах таблицы
    size_t labels_size;
    table.read_exc(reinterpret_cast<char*>(&labels_size), sizeof(labels_size));
    for (size_t i = 0; i < labels_size; ++i) {
        ColumnLabel label;
        std::vector<char> buffer;
        table.read_exc(reinterpret_cast<char*>(&label.name_size), sizeof(label.name_size));

        buffer = std::vector<char>(label.name_size);
        table.read_exc(buffer.data(), label.name_size);
        label.name = std::string(buffer.begin(), buffer.end()); // добавить строку, через копирование веткора
        table.read_exc(reinterpret_cast<char*>(&label.value_type), sizeof(label.value_type));
        table.read_exc(reinterpret_cast<char*>(&label.value_max_size), sizeof(label.value_max_size));
        table.read_exc(reinterpret_cast<char*>(&label.value_default_size), sizeof(label.value_default_size));
        label.value_default = read_variants_value(label.value_type, label.value_default_size, label.value_default_size, &table); // читаем default value в зависимости от его типа
        table.read_exc(reinterpret_cast<char*>(&label.is_unique), sizeof(label.is_unique));
        table.read_exc(reinterpret_cast<char*>(&label.is_autoincrement), sizeof(label.is_autoincrement));
        table.read_exc(reinterpret_cast<char*>(&label.is_key), sizeof(label.is_key));
        table.read_exc(reinterpret_cast<char*>(&label.is_ordered), sizeof(label.is_ordered));
        table.read_exc(reinterpret_cast<char*>(&label.is_unordered), sizeof(label.is_unordered));

        labels->push_back(label); // добавляем описание прочитанной колонки
    }

    size_t end_labels_pos = table.tellg(); // сохраняем текущую позицию
    table.close_exc(); 
    return end_labels_pos;
}


// Обновить информацию о структуре таблицы (информация о столбцах и позиции крайних строк)
void Sql::read_table_positions(const std::string& table_name, const size_t end_labels_pos, RowsPositions* positions) {

    size_t bytes_in_row = 0; // количество байт в строке в секции с данными
    for (auto const& label : table_labels[table_name]) {
        bytes_in_row += sizeof(label.value_max_size) + label.value_max_size;
    }

    // Проверка на наличие такой таблицы
    const std::string table_path_name = "../data/" + table_name;
    if (!std::filesystem::exists(table_path_name)) {
        throw std::ios_base::failure("Table does not exist\n");
    }
    
    // Открытие таблицы
    FileStreamWithExceptions table;
    table.open_exc(table_path_name, std::ios_base::binary | std::ios_base::in);

    positions->start = end_labels_pos; // сохраняем кол-во байт до первой строки

    table.seekg(0, std::ios::end); // перемещаем указатель в конец файла
    table.seekg(-1*bytes_in_row, std::ios::cur); // возвращаемся в начало последней строки
    positions->last = table.tellg(); // сохранями текущую позицию указателя

    if (positions->last < positions->start) { // в таблице нет строк
        positions->last = std::nullopt;
    }

    table.close_exc();
}

// Обновить информацию о структуре таблицы (информация о столбцах и позиции крайних строк)
void Sql::read_table_indexes(const std::string& table_name, OrderedIndex* ordered_indexes, UnorderedIndex* unordered_indexes) {
    if (table_positions[table_name].last == std::nullopt) { // если данных нет
        return;
    }

    const std::string table_path_name = "../data/" + table_name;
    // Проверка на наличие такой таблицы
    if (!std::filesystem::exists(table_path_name)) {
        throw std::ios_base::failure("Table does not exist\n");
    }
    
    // Открытие таблицы
    FileStreamWithExceptions table;
    table.open_exc(table_path_name, std::ios_base::binary | std::ios_base::in);

    table.seekg(table_positions[table_name].start, std::ios::beg);// переместим на начало данных

    size_t bytes_in_row = count_bytes_in_row(table_name);
    size_t rows_size = (table_positions[table_name].last.value() - table_positions[table_name].start + 1) / bytes_in_row + 1; // количетсво строк
    
    size_t columns_size = table_labels[table_name].size();
    for (size_t cur_row = 0; cur_row < rows_size; ++cur_row) { // по количеству строк
        size_t row_position = table.tellg(); // позиция начала текущей строки

        for (size_t i = 0; i < columns_size; ++i) { // по количеству столбцов
            const auto& label = table_labels[table_name][i];
            size_t value_size;
            table.read_exc(reinterpret_cast<char*>(&value_size), sizeof(value_size));
            variants var = read_variants_value(label.value_type, value_size, label.value_max_size, &table); // читаем значение переменной
            if (label.is_ordered) { // если стоит ordered индексацияы
                (*ordered_indexes)[table_name][label.name][var].push_back(row_position);
            }
            if (label.is_unordered) { // если стоит unordered индексация
                (*unordered_indexes)[table_name][label.name][var].push_back(row_position);
            }
        }
    }
    
    table.close_exc(); 
}

size_t Sql::count_bytes_in_row(const std::string table_name) {
    if (table_positions[table_name].last == std::nullopt) { // если данных нет
        return 0;
    }

    size_t bytes_in_row = 0; // количество байт в строке в секции с данными
    for (auto const& label : table_labels[table_name]) {
        bytes_in_row += sizeof(label.value_max_size) + label.value_max_size;
    }

    return bytes_in_row;
}