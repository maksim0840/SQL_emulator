#include <iostream>
#include <filesystem>
#include "sql_helper.h"
#include "condition.h"

void Sql::create_table(const std::string& table_name, const std::vector<ColumnLabel>& labels) {

    // Проверка на наличие такой таблицы
    const std::string table_path_name = data_dir_ + "/" + table_name;
    if (std::filesystem::exists(table_path_name)) {
        throw SqlException("Table alredy exists");
    }

    // Создание таблицы
    FileStreamWithExceptions table;
    table.open_exc(table_path_name, std::ios_base::binary | std::ios::app | std::ios_base::out);

    // Запись служебной информации о столбцах таблицы
    // *структуру целиком и сразу записать нельзя т.к. (string) - динамический, а (char*) только лишь указатель, а не сами байты*
    const size_t labels_size = labels.size();
    table.write_exc(reinterpret_cast<const char*>(&labels_size), sizeof(labels_size));
    for (size_t i = 0; i < labels_size; ++i) {
        table.write_exc(reinterpret_cast<const char*>(&(labels[i].name_size)), sizeof(labels[i].name_size));
        // Обращаемся по указателю на первый элемент c-style строки
        table.write_exc(labels[i].name.c_str(), labels[i].name_size);
        table.write_exc(reinterpret_cast<const char*>(&(labels[i].value_type)), sizeof(labels[i].value_type));
        table.write_exc(reinterpret_cast<const char*>(&(labels[i].value_max_size)), sizeof(labels[i].value_max_size));
        table.write_exc(reinterpret_cast<const char*>(&(labels[i].value_default_size)), sizeof(labels[i].value_default_size));
        helper_->write_variants_value(labels[i].value_default, labels[i].value_default_size, &table);
        table.write_exc(reinterpret_cast<const char*>(&(labels[i].is_unique)), sizeof(labels[i].is_unique));
        table.write_exc(reinterpret_cast<const char*>(&(labels[i].is_autoincrement)), sizeof(labels[i].is_autoincrement));
        table.write_exc(reinterpret_cast<const char*>(&(labels[i].is_key)), sizeof(labels[i].is_key));
        table.write_exc(reinterpret_cast<const char*>(&(labels[i].is_ordered)), sizeof(labels[i].is_ordered));
        table.write_exc(reinterpret_cast<const char*>(&(labels[i].is_unordered)), sizeof(labels[i].is_unordered));
    }

    table.close_exc();
    size_t last_read_pos = read_table_labels(table_name, &table_labels[table_name]); // сохранить инфомарцию о заголовках таблицы
    read_table_positions(table_name, last_read_pos, &table_positions[table_name]); // сохранить информацию о начале строк
}

// Устанавливает флаги в заголовке таблицы для столбца column_name
void Sql::create_index(const std::string& table_name, const std::string& column_name, const IndexType index_type) {
    
    // Проверка на наличие такой таблицы
    const std::string table_path_name = data_dir_ + "/" + table_name;
    if (!std::filesystem::exists(table_path_name)) {
        throw SqlException("Table does not exist");
    }
    
    // Открытие таблицы
    FileStreamWithExceptions table;
    table.open_exc(table_path_name, std::ios_base::binary | std::ios_base::in | std::ios_base::out); // для замены нужно открыть в режиме rw

    // Чтение служебной информации о столбцах таблицы
    size_t labels_size;
    table.read_exc(reinterpret_cast<char*>(&labels_size), sizeof(labels_size));

    // Пропускаем значения пока не дойдём до флагов индексов
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

        // Устанавливаем флаги
        if (name == column_name && index_type == IndexType::ORDERED) {
            bool ord = true;
            table.write_exc(reinterpret_cast<const char*>(&ord), sizeof(ord)); // is_ordered
            table.seekg(sizeof(bool), std::ios::cur); // is_unordered
            Sql::read_table_indexes(table_name, table_ordered_indexes, table_unordered_indexes);
            return;
        }
        else if (name == column_name && index_type == IndexType::UNORDERED) {
            bool ord = true;
            table.seekg(sizeof(bool), std::ios::cur); // is_ordered
            table.write_exc(reinterpret_cast<const char*>(&ord), sizeof(ord));// is_unordered
            Sql::read_table_indexes(table_name, table_ordered_indexes, table_unordered_indexes);
            return;
        }
        table.seekg(sizeof(bool), std::ios::cur); // is_ordered
        table.seekg(sizeof(bool), std::ios::cur); // is_unordered
    }
    table.close_exc();

    throw SqlException("cant find column name in table");
}

void Sql::insert(const std::string& table_name, const std::unordered_map<std::string, variants>& row_values) {
    
    // Проверка на наличие такой таблицы
    const std::string table_path_name = data_dir_ + "/" + table_name;
    if (!std::filesystem::exists(table_path_name)) {
        throw SqlException("Table does not exist");
    }

    // Нахождение стартовой позиции
    size_t insert_pos = table_positions[table_name].start; // начало секции с данными
    if (table_positions[table_name].last != std::nullopt) { // есть записи
        insert_pos = table_positions[table_name].last.value() + helper_->count_bytes_in_row(table_name); // конец файла
    }

    // Открытие таблицы
    FileStreamWithExceptions table;
    table.open_exc(table_path_name, std::ios_base::binary | std::ios_base::out | std::ios_base::in | std::ios_base::app); // режим записи и добавления назад
   
   // Проверяем данные которые хотим вставить
   std::vector<std::pair<size_t, variants>> checked_values = helper_->get_checked_values_to_insert(table_name, row_values, &table);

    // Вставляем значения в файл таблицы
    size_t cur = 0;
    for (const auto& label : table_labels[table_name]) {
        // Записываем размер
        table.write_exc(reinterpret_cast<const char*>(&(checked_values[cur].first)), sizeof(checked_values[cur].first));
        // Записываем значение
        helper_->write_variants_value(checked_values[cur].second, label.value_max_size, &table);
        // Дополняем индексы
        if (label.is_ordered) (*table_ordered_indexes)[table_name][label.name][checked_values[cur].second].push_back(insert_pos);
        if (label.is_unordered) (*table_unordered_indexes)[table_name][label.name][checked_values[cur].second].push_back(insert_pos);
            
        ++cur;
    }
    table.close_exc();
    
    // Увеличиваем позицию последней строки в файле
    if (table_positions[table_name].last == std::nullopt) {
        table_positions[table_name].last = table_positions[table_name].start;
    }
    else {
        table_positions[table_name].last.value() += helper_->count_bytes_in_row(table_name, false);
    }
}

void Sql::select(const std::unordered_set<std::string>& columns, const std::string& table_name) {

    // Проверка на наличие такой таблицы
    const std::string table_path_name = data_dir_ + "/" + table_name;
    if (!std::filesystem::exists(table_path_name)) {
        throw SqlException("Table does not exist\n");
    }

    std::vector<std::size_t> paddings; // вектор с отступами для каждого столбца
    int cur_padding = 0;

    std::vector<std::string> label_names = helper_->get_label_names(table_name, columns, &paddings);
    std::vector<Sql::ValueType> value_types;
    std::vector<std::vector<variants>> data_rows = helper_->get_data_rows(columns, table_name, &value_types);
    
    // Перевести значения в строчки для выводы
    std::vector<std::string> data_rows_str;
    int cur_type = 0;

    for (const auto& row : data_rows) {
        cur_padding = 0;

        for (const auto& var : row) {
            std::string str = helper_->convert_variants_for_output(var, value_types[cur_type]);
            data_rows_str.push_back(str);
            paddings[cur_padding] = std::max(paddings[cur_padding], str.size());
            ++cur_type;
            ++cur_padding;
        }
    }

    std::cout << std::left; // устанавливаем выравнивание по левому краю

    // Вывод названий столбцов
    cur_padding = 0;
    for (const auto& name : label_names) {
       std::cout << std::setw(paddings[cur_padding]) << name << "    ";
       ++cur_padding;
    }
    std::cout << '\n';

    // Вывод строк значений
    cur_type = 0;
    for (const auto& row : data_rows) {
        cur_padding = 0;
        for (const auto& var : row) {
            std::cout << std::setw(paddings[cur_padding]) << helper_->convert_variants_for_output(var, value_types[cur_type]) << "    ";
            ++cur_type;
            ++cur_padding;
        }
        std::cout << '\n';
    }
}

void Sql::delete_table(const std::string& table_name) {
    // Проверка на наличие такой таблицы
    const std::string table_path_name = data_dir_ + "/" + table_name;
    if (!std::filesystem::exists(table_path_name)) {
        throw SqlException("Table does not exist\n");
    }

    // Удалить файл
    if (!std::filesystem::remove(table_path_name)) {
        throw SqlException("cant remove table");
    }

    // Удалить связанную с таблицей инфомрацию
    table_labels.erase(table_name);
    table_positions.erase(table_name);
    table_ordered_indexes->erase(table_name); 
    table_unordered_indexes->erase(table_name);

}