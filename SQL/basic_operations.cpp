#include "fstream_with_exceptions.h"
#include "sql.h"
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <variant>

// Конструктор
Sql::Sql() {
    const std::string dir_name = "tables";
    const auto dir = std::filesystem::directory_iterator(dir_name);

    // Цикл по всем файлам дирректории "tables"
    for (const auto& entry : dir) {
        update_table_label(entry.path().filename()); // получить информацию о столбцах таблицы
    }
}


// Обновить информацию о столбцах в talbe_name таблице
void Sql::update_table_label(const std::string& talbe_name) {
    const std::string table_path_name = "tables/" + talbe_name;
    std::vector<ColumnLabel> columns_vec; // вектор из информации о каждой колонке таблицы
    size_t labels_size;

    // Проверка на наличие такой таблицы
    if (!std::filesystem::exists(table_path_name)) {
        throw std::ios_base::failure("Table does not exist\n");
    }

    // Открытие таблицы
    FileStreamWithExceptions table;
    table.open_exc(table_path_name, std::ios_base::binary | std::ios_base::in);

    // Чтение служебной информации о столбцах таблицы
    table.read_exc(reinterpret_cast<char*>(&labels_size), sizeof(labels_size));
    for (size_t i = 0; i < labels_size; ++i) {
        ColumnLabel label;
        std::vector<char> buffer;

        table.read_exc(reinterpret_cast<char*>(&label.name_size), sizeof(label.name_size));
        // Добавить строку, через копирование веткора
        buffer = std::vector<char>(label.name_size);
        table.read_exc(buffer.data(), label.name_size);
        label.name = std::string(buffer.begin(), buffer.end());
        table.read_exc(reinterpret_cast<char*>(&label.value_type), sizeof(label.value_type));
        table.read_exc(reinterpret_cast<char*>(&label.value_default_size), sizeof(label.value_default_size));
        // Добавить строку, через копирование веткора
        buffer = std::vector<char>(label.value_default_size);
        table.read_exc(buffer.data(), label.value_default_size);
        label.value_default = std::string(buffer.begin(), buffer.end());
        table.read_exc(reinterpret_cast<char*>(&label.is_unique), sizeof(label.is_unique));
        table.read_exc(reinterpret_cast<char*>(&label.is_autoincrement), sizeof(label.is_autoincrement));
        table.read_exc(reinterpret_cast<char*>(&label.is_key), sizeof(label.is_key));

        columns_vec.push_back(label); // добавляем 1 колонку
    }

    table_labels[talbe_name] = columns_vec;
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
        table.write_exc(reinterpret_cast<const char*>(&(labels[i].value_default_size)), sizeof(labels[i].value_default_size));
        // Обращаемся по указателю на первый элемент c-style строки
        table.write_exc(labels[i].value_default.c_str(), labels[i].value_default_size);
        table.write_exc(reinterpret_cast<const char*>(&(labels[i].is_unique)), sizeof(labels[i].is_unique));
        table.write_exc(reinterpret_cast<const char*>(&(labels[i].is_autoincrement)), sizeof(labels[i].is_autoincrement));
        table.write_exc(reinterpret_cast<const char*>(&(labels[i].is_key)), sizeof(labels[i].is_key));
    }

    table.close_exc();
    update_table_label(talbe_name); // сохраняем информацию о новой таблице
}


void Sql::insert(const std::string& talbe_name, const std::vector<variants>& input_values) {
    const size_t input_size = input_values.size();

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