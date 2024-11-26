
#include "../setup/tables_info.cpp"
#include "../condition/condition.cpp"
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <variant>
#include <optional>

// Заменяет значение во всех строчках по условию Condition
void replace(const std::string& table_name, const std::string column_name) { 
 ///////////////////// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 ///// ???? <=> update table ?
}

void Sql::create_table(const std::string& table_name, const std::vector<ColumnLabel>& labels) {
    const std::string table_path_name = "../data/" + table_name;
    const size_t labels_size = labels.size();

    // Проверка на наличие такой таблицы
    if (std::filesystem::exists(table_path_name)) {
        throw std::ios_base::failure("Table alredy exists\n");
    }

    // Создание таблицы
    FileStreamWithExceptions table;
    table.open_exc(table_path_name, std::ios_base::binary | std::ios::app | std::ios_base::out);

    // Запись служебной информации о столбцах таблицы
    // *структуру целиком и сразу записать нельзя т.к. (string) - динамический, а (char*) только лишь указатель, а не сами байты*
    table.write_exc(reinterpret_cast<const char*>(&labels_size), sizeof(labels_size));
    for (size_t i = 0; i < labels_size; ++i) {
        table.write_exc(reinterpret_cast<const char*>(&(labels[i].name_size)), sizeof(labels[i].name_size));
        // Обращаемся по указателю на первый элемент c-style строки
        table.write_exc(labels[i].name.c_str(), labels[i].name_size);
        table.write_exc(reinterpret_cast<const char*>(&(labels[i].value_type)), sizeof(labels[i].value_type));
        table.write_exc(reinterpret_cast<const char*>(&(labels[i].value_max_size)), sizeof(labels[i].value_max_size));
        table.write_exc(reinterpret_cast<const char*>(&(labels[i].value_default_size)), sizeof(labels[i].value_default_size));
        // Проверяем хранящийся тип (std::monostate не записываем в файл)
        if (std::holds_alternative<std::string>(labels[i].value_default)) {
            table.write_exc(std::get<std::string>(labels[i].value_default).c_str(), labels[i].value_default_size);
        }
        else if (std::holds_alternative<int>(labels[i].value_default)) { // std::monostate не записываем в файл
            table.write_exc(reinterpret_cast<const char*>(&(std::get<int>(labels[i].value_default))), labels[i].value_default_size);
        } 
        else if (std::holds_alternative<bool>(labels[i].value_default)) { // std::monostate не записываем в файл
            table.write_exc(reinterpret_cast<const char*>(&(std::get<bool>(labels[i].value_default))), labels[i].value_default_size);
        } 
        table.write_exc(reinterpret_cast<const char*>(&(labels[i].is_unique)), sizeof(labels[i].is_unique));
        table.write_exc(reinterpret_cast<const char*>(&(labels[i].is_autoincrement)), sizeof(labels[i].is_autoincrement));
        table.write_exc(reinterpret_cast<const char*>(&(labels[i].is_key)), sizeof(labels[i].is_key));
        table.write_exc(reinterpret_cast<const char*>(&(labels[i].is_ordered)), sizeof(labels[i].is_ordered));
        table.write_exc(reinterpret_cast<const char*>(&(labels[i].is_unordered)), sizeof(labels[i].is_unordered));
    }

    table.close_exc();
    size_t last_read_pos = read_table_labels(table_name, &table_labels[table_name]);
    read_table_positions(table_name, last_read_pos, &table_positions[table_name]); // получить информацию о столбцах таблицы
}


void Sql::insert(const std::string& table_name, const std::vector<variants>& input_values) {
    const size_t input_size = input_values.size();

    /* БЕЗ ИНДЕКСОВ !!!!!!!!!!!!!!!!!!!!!*/
    /* сделать логику между

        deafult
        пустой
        не пустой
        autoincr
        unique
    */
   
    for (size_t i = 0; i < input_size; ++i) {
        // Логика для autoincrement
        if (table_labels[table_name][i].is_autoincrement) {
            if (table_labels[table_name][i].value_default_size != 0) { // если установленно
                throw;
            }
        }
        if (std::holds_alternative<std::monostate>(input_values[i])) { // если тип пустой
            if (table_labels[table_name][i].value_default_size == 0) {

            }
        }
        switch(table_labels[table_name][i].value_type) { // смотрим на тип в столбце
            case ValueType::INT32:

                break;
            case ValueType::BOOL:
                break;
            case ValueType::STRING:
                break;
            case ValueType::BYTES:
                break;
        }
    }

}

// устанавливает флаги в заголовок таблицы для столбца column_name
void Sql::create_index(const std::string& table_name, const std::string& column_name, const IndexType index_type) {
    const std::string table_path_name = "../data/" + table_name;
    // Проверка на наличие такой таблицы
    if (!std::filesystem::exists(table_path_name)) {
        throw std::ios_base::failure("Table does not exist\n");
    }
    
    // Открытие таблицы
    FileStreamWithExceptions table;
    table.open_exc(table_path_name, std::ios_base::binary | std::ios_base::in | std::ios_base::out); // для замены нужно открыть в режиме rw

    // Чтение служебной информации о столбцах таблицы
    size_t labels_size;
    table.read_exc(reinterpret_cast<char*>(&labels_size), sizeof(labels_size));

    for (size_t i = 0; i < labels_size; ++i) {
        std::string name;
        size_t buffer_size;
        std::vector<char> buffer;

        table.read_exc(reinterpret_cast<char*>(&buffer_size), sizeof(buffer_size)); // name_size
        buffer = std::vector<char>(buffer_size);
        table.read_exc(buffer.data(), buffer_size); // name
        name = std::string(buffer.begin(), buffer.end()); // добавить строку, через копирование веткора
        table.seekg(sizeof(Sql::ValueType), std::ios::cur); // value_type
        table.seekg(sizeof(size_t), std::ios::cur); // max_size
        table.read_exc(reinterpret_cast<char*>(&buffer_size), sizeof(buffer_size)); // default_size
        table.seekg(buffer_size, std::ios::cur); // value_default
        table.seekg(sizeof(bool), std::ios::cur); // is_unique
        table.seekg(sizeof(bool), std::ios::cur); // is_autoincrement
        table.seekg(sizeof(bool), std::ios::cur); // is_key
        if (name == table_name && index_type == IndexType::ORDERED) {
            bool ord = true;
            table.write_exc(reinterpret_cast<const char*>(&ord), sizeof(ord)); // is_ordered
            table.seekg(sizeof(bool), std::ios::cur); // is_unordered
            Sql::read_table_indexes(table_name, bytes_in_row, table_ordered_indexes, table_unordered_indexes)
            return;
        }
        else if (name == table_name && index_type == IndexType::UNORDERED) {
            bool ord = true;
            table.seekg(sizeof(bool), std::ios::cur); // is_ordered
            table.write_exc(reinterpret_cast<const char*>(&ord), sizeof(ord));// is_unordered
            Sql::read_table_indexes(table_name, bytes_in_row, table_ordered_indexes, table_unordered_indexes)
            return;
        }
        table.seekg(sizeof(bool), std::ios::cur); // is_ordered
        table.seekg(sizeof(bool), std::ios::cur); // is_unordered
    }

    throw "cant find column name in table"
}
    
    