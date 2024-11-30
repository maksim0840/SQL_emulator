#include <filesystem>
#include "sql_helper.h"

// Конструктор
Sql::Sql(const std::string& data_dir) {
    table_ordered_indexes = new OrderedIndex();
    table_unordered_indexes = new UnorderedIndex();
    helper_ = new Sql::Helper(this);
    data_dir_ = data_dir;

    const auto dir = std::filesystem::directory_iterator(data_dir_);

    // Цикл по всем файлам директории "data"
    for (const auto& entry : dir) {
        const std::string table_name = entry.path().filename();
        size_t last_read_pos = read_table_labels(table_name, &table_labels[table_name]);
        read_table_positions(table_name, last_read_pos, &table_positions[table_name]);
        read_table_indexes(table_name, table_ordered_indexes, table_unordered_indexes);
    }
}

// Деструктор
Sql::~Sql() {
    delete table_ordered_indexes;
    delete table_unordered_indexes;
    delete helper_;
}


// Получить информацию о столбцах
size_t Sql::read_table_labels(const std::string& table_name, std::vector<ColumnLabel>* labels) {

    // Проверка на наличие такой таблицы
    const std::string table_path_name = data_dir_ + "/" + table_name;
    if (!std::filesystem::exists(table_path_name)) {
        throw SqlException("Table does not exist");
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
        label.value_default = helper_->read_variants_value(label.value_type, label.value_default_size, label.value_default_size, &table); // читаем default value в зависимости от его типа
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


// Получить информацию о позициях строк
void Sql::read_table_positions(const std::string& table_name, const size_t end_labels_pos, RowsPositions* positions) {

    // Проверка на наличие такой таблицы
    const std::string table_path_name = data_dir_ + "/" + table_name;
    if (!std::filesystem::exists(table_path_name)) {
        throw SqlException("Table does not exist");
    }
    
    // Открытие таблицы
    FileStreamWithExceptions table;
    table.open_exc(table_path_name, std::ios_base::binary | std::ios_base::in);

    positions->start = end_labels_pos; // сохраняем кол-во байт до первой строки

    size_t bytes_in_row = helper_->count_bytes_in_row(table_name, false);
    table.seekg(0, std::ios::end); // перемещаем указатель в конец файла
    
    if (static_cast<size_t>(table.tellg()) <= positions->start) { // в таблице нет строк
        positions->last = std::nullopt;
        return;
    }

    table.seekg(-1*bytes_in_row, std::ios::cur); // возвращаемся в начало последней строки
    positions->last = table.tellg(); // сохранями текущую позицию указателя

    table.close_exc();
}

// Сохранить значения столбцов с включёнными индексами в кучу
void Sql::read_table_indexes(const std::string& table_name, OrderedIndex* ordered_indexes, UnorderedIndex* unordered_indexes) {
    if (table_positions[table_name].last == std::nullopt) { // если данных нет
        return;
    }

    // Проверка на наличие такой таблицы
    const std::string table_path_name = data_dir_ + "/" + table_name;
    if (!std::filesystem::exists(table_path_name)) {
        throw SqlException("Table does not exist");
    }
    
    // Открытие таблицы
    FileStreamWithExceptions table;
    table.open_exc(table_path_name, std::ios_base::binary | std::ios_base::in);

    table.seekg(table_positions[table_name].start, std::ios::beg);// переместим на начало данных

    size_t rows_in_table = helper_->count_rows_in_table(table_name);
    for (size_t row = 0; row < rows_in_table; ++row) { // по количеству строк
        size_t row_position = table.tellg(); // позиция начала текущей строки
        
        for (const auto& label : table_labels[table_name]) {
            size_t value_size;
            table.read_exc(reinterpret_cast<char*>(&value_size), sizeof(value_size));
            variants var =  helper_->read_variants_value(label.value_type, value_size, label.value_max_size, &table); // читаем значение переменной

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
