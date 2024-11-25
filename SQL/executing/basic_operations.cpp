
#include "../setup/tables_info.cpp"
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <variant>
#include <optional>


void Sql::create_table(const std::string& table_name, const std::vector<ColumnLabel>& labels) {
    const std::string table_path_name = "tables/" + table_name;
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
        // Проверяем хранящийся тип
        if (std::holds_alternative<std::string>(labels[i].value_default)) {
            table.write_exc(std::get<std::string>(labels[i].value_default).c_str(), labels[i].value_default_size);
        }
        else if (!std::holds_alternative<std::monostate>(labels[i].value_default)) { // std::monostate не записываем в файл
            table.write_exc(reinterpret_cast<const char*>(&(labels[i].value_default)), labels[i].value_default_size);
        } 
        table.write_exc(reinterpret_cast<const char*>(&(labels[i].is_unique)), sizeof(labels[i].is_unique));
        table.write_exc(reinterpret_cast<const char*>(&(labels[i].is_autoincrement)), sizeof(labels[i].is_autoincrement));
        table.write_exc(reinterpret_cast<const char*>(&(labels[i].is_key)), sizeof(labels[i].is_key));
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
