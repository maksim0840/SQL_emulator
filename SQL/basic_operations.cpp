#include <fstream>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>

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

// Кастомный тип файла для выбрасывания исключений при ошибках
class FileStreamWithExceptions: public std::fstream {
public:
    void write_exc(const char* s, size_t n) {
        this->write(s, n);
        if (this->fail()) {
            throw std::ios_base::failure("Cant write to binary table file\n");
        }
    }
    void read_exc(char* s, size_t n) {
        this->read(s, n);
        if (this->fail()) {
            throw std::ios_base::failure("Cant read binary table file\n");
        }

    }
    void open_exc(const std::string& s, ios_base::openmode mode) {
        this->open(s, mode);
        if (this->fail()) {
            throw std::ios_base::failure("Cant open binary table file\n");
        }
    }
    void close_exc() {
        this->close();
        if (this->fail()) {
            throw std::ios_base::failure("Cant close binary table file\n");
        }

    }
};


void create_table(const std::string& talbe_name, const std::vector<ColumnLabel>& labels, const size_t labels_size) {
    const std::string table_path_name = "tables/" + talbe_name;

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
    for (int i = 0; i < labels_size; ++i) {
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
}


std::unordered_map<std::string, ColumnLabel> get_labels(const std::string& talbe_name) {
    const std::string table_path_name = "tables/" + talbe_name;
    size_t labels_size;
    std::unordered_map<std::string, ColumnLabel> labels;

    // Проверка на наличие такой таблицы
    if (!std::filesystem::exists(table_path_name)) {
        throw std::ios_base::failure("Table does not exist\n");
    }

    // Открытие таблицы
    FileStreamWithExceptions table;
    table.open_exc(table_path_name, std::ios_base::binary | std::ios_base::in);

    // Чтение служебной информации о столбцах таблицы
    table.read_exc(reinterpret_cast<char*>(&labels_size), sizeof(labels_size));
    for (int i = 0; i < labels_size; ++i) {
        ColumnLabel label;
        std::string name;
        std::vector<char> buffer;

        table.read_exc(reinterpret_cast<char*>(&label.name_size), sizeof(label.name_size));
        // Добавить строку, через копирование веткора
        buffer = std::vector<char>(label.name_size);
        table.read_exc(buffer.data(), label.name_size);
        name = std::string(buffer.begin(), buffer.end());
        label.name = name;
        table.read_exc(reinterpret_cast<char*>(&label.value_type), sizeof(label.value_type));
        table.read_exc(reinterpret_cast<char*>(&label.value_default_size), sizeof(label.value_default_size));
        // Добавить строку, через копирование веткора
        buffer = std::vector<char>(label.value_default_size);
        table.read_exc(buffer.data(), label.value_default_size);
        label.value_default = std::string(buffer.begin(), buffer.end());
        table.read_exc(reinterpret_cast<char*>(&label.is_unique), sizeof(label.is_unique));
        table.read_exc(reinterpret_cast<char*>(&label.is_autoincrement), sizeof(label.is_autoincrement));
        table.read_exc(reinterpret_cast<char*>(&label.is_key), sizeof(label.is_key));

        labels[name] = label;
    }

    table.close_exc();

    return labels;
}


int main() {
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

    return 0;
}