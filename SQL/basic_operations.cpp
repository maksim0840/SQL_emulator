#include "fstream_with_exceptions.h"
#include "sql.h"
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <variant>
#include <optional>

// Конструктор
Sql::Sql() {
    const std::string dir_name = "tables";
    const auto dir = std::filesystem::directory_iterator(dir_name);

    // Цикл по всем файлам дирректории "tables"
    for (const auto& entry : dir) {
        update_table_info(entry.path().filename()); // получить информацию о столбцах таблицы
    }
}


// Обновить информацию о структуре таблицы (информация о столбцах и позиции крайних строк)
void Sql::update_table_info(const std::string& talbe_name) {
    const std::string table_path_name = "tables/" + talbe_name;
    std::vector<ColumnLabel> columns_vec; // вектор из информации о каждой колонке таблицы
    size_t labels_size;
    size_t bytes_passed = 0; // количество считанных байт
    size_t bytes_in_row = 0; // количество байт в строке в секции с данными

    // Проверка на наличие такой таблицы
    if (!std::filesystem::exists(table_path_name)) {
        throw std::ios_base::failure("Table does not exist\n");
    }
    std::streampos;
    // Открытие таблицы
    FileStreamWithExceptions table;
    table.open_exc(table_path_name, std::ios_base::binary | std::ios_base::in);

    // Чтение служебной информации о столбцах таблицы
    bytes_passed += table.read_exc(reinterpret_cast<char*>(&labels_size), sizeof(labels_size));
    for (size_t i = 0; i < labels_size; ++i) {
        ColumnLabel label;
        std::vector<char> buffer;

        bytes_passed += table.read_exc(reinterpret_cast<char*>(&label.name_size), sizeof(label.name_size));
        buffer = std::vector<char>(label.name_size);
        bytes_passed += table.read_exc(buffer.data(), label.name_size); // добавить строку, через копирование веткора
        label.name = std::string(buffer.begin(), buffer.end());
        bytes_passed += table.read_exc(reinterpret_cast<char*>(&label.value_type), sizeof(label.value_type));
        bytes_passed += table.read_exc(reinterpret_cast<char*>(&label.value_max_size), sizeof(label.value_max_size));
        bytes_passed += table.read_exc(reinterpret_cast<char*>(&label.value_default_size), sizeof(label.value_default_size));
        buffer = std::vector<char>(label.value_default_size);
        bytes_passed += table.read_exc(buffer.data(), label.value_default_size); // добавить строку, через копирование веткора
        label.value_default = std::string(buffer.begin(), buffer.end());
        bytes_passed += table.read_exc(reinterpret_cast<char*>(&label.is_unique), sizeof(label.is_unique));
        bytes_passed += table.read_exc(reinterpret_cast<char*>(&label.is_autoincrement), sizeof(label.is_autoincrement));
        bytes_passed += table.read_exc(reinterpret_cast<char*>(&label.is_key), sizeof(label.is_key));

        columns_vec.push_back(label); // добавляем описание прочитанной колонки
        bytes_in_row += sizeof(label.value_max_size) + label.value_max_size;
    }

    table_labels[talbe_name] = columns_vec; // сохраняем описание колонок
    table_rows[talbe_name].start = bytes_passed; // сохраняем кол-во байт до первой строки

    table.seekg(0, std::ios::end); // перемещаем указатель в конец файла
    table.seekg(-1*bytes_in_row, std::ios::cur); // возвращаемся в начало последней строки
    table_rows[talbe_name].last = table.tellg(); // сохранями текущую позицию указателя

    if (table_rows[talbe_name].last < table_rows[talbe_name].start) { // в таблице нет строк
        table_rows[talbe_name].last = std::nullopt;
    }

    table.close_exc(); 
}


void Sql::create_table(const std::string& talbe_name, const std::vector<ColumnLabel>& labels) {
    const std::string table_path_name = "tables/" + talbe_name;
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
        table.write_exc(reinterpret_cast<const char*>(&(labels[i].value_default_size)), sizeof(labels[i].value_max_size));
        table.write_exc(reinterpret_cast<const char*>(&(labels[i].value_default_size)), sizeof(labels[i].value_default_size));
        // Обращаемся по указателю на первый элемент c-style строки
        table.write_exc(labels[i].value_default.c_str(), labels[i].value_default_size);
        table.write_exc(reinterpret_cast<const char*>(&(labels[i].is_unique)), sizeof(labels[i].is_unique));
        table.write_exc(reinterpret_cast<const char*>(&(labels[i].is_autoincrement)), sizeof(labels[i].is_autoincrement));
        table.write_exc(reinterpret_cast<const char*>(&(labels[i].is_key)), sizeof(labels[i].is_key));
    }

    table.close_exc();
    update_table_info(talbe_name); // сохраняем информацию о новой таблице
}


void Sql::insert(const std::string& talbe_name, const std::vector<variants>& input_values) {
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
        if (table_labels[talbe_name][i].is_autoincrement) {
            if (table_labels[talbe_name][i].value_default_size != 0) { // если установленно
                throw;
            }
        }
        if (std::holds_alternative<std::monostate>(input_values[i])) { // если тип пустой
            if (table_labels[talbe_name][i].value_default_size == 0)
        }
        switch(table_labels[talbe_name][i].value_type) { // смотрим на тип в столбце
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

int main() {
    // создание таблицы
    Sql sql;

    /*
    ColumnLabel label1 = {2, "id", ValueType::INT32, 4, std::to_string(17), true, true, false};
    ColumnLabel label2 = {4, "city", ValueType::STRING, 6, "Moscow", false, true, false};
    ColumnLabel label3 = {7, "country", ValueType::STRING, 6, "Russia", false, true, false};
    std::vector<ColumnLabel> vec;
    vec.push_back(label1);
    vec.push_back(label2);
    vec.push_back(label3);

    create_table("xyz", vec, vec.size());
    std::unordered_map<std::string, ColumnLabel> m = get_labels("xyz");
    std::cout << m["id"].name_size << ' ' << m["id"].name << ' ' << (int)m["id"].value_type << ' ' << m["id"].value_default_size << ' ' << m["id"].value_default << ' ' << m["id"].is_unique << ' ' << m["id"].is_autoincrement << ' ' << m["id"].is_key << '\n';
    std::cout << m["city"].name_size << ' ' << m["city"].name << ' ' << (int)m["city"].value_type << ' ' << m["city"].value_default_size << ' ' << m["city"].value_default << ' ' << m["city"].is_unique << ' ' << m["city"].is_autoincrement << ' ' << m["city"].is_key << '\n';
    std::cout << m["country"].name_size << ' ' << m["country"].name << ' ' << (int)m["country"].value_type << ' ' << m["country"].value_default_size << ' ' << m["country"].value_default << ' ' << m["country"].is_unique << ' ' << m["country"].is_autoincrement << ' ' << m["country"].is_key << '\n';
    */

    /*
    // значния хранятся в виде
    std::vector<variants> vec_inserted_values; // либо значение нашего типа либо nullopt
    vec_inserted_values.push_back(true);
    vec_inserted_values.push_back(20);
    vec_inserted_values.push_back("okkk");
    vec_inserted_values.push_back(std::monostate{}); // нулевое значение
    
    insert("xyz", inserted_values, 4);
    std::vector<std::optional<ValueType>>;
    */

    return 0;
}