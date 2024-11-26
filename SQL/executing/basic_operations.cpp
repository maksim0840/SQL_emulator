
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
// void replace(const std::string& table_name, const std::string column_name) { 
//  ///////////////////// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//  ///// ???? <=> update table ?
// }

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
        write_variants_value(labels[i].value_default, labels[i].value_default_size, &table);
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

    throw "cant find column name in table";
}

void Sql::insert(const std::string& table_name, const std::unordered_map<std::string, variants> row_values) {
    // Проверка на наличие такой таблицы
    const std::string table_path_name = "../data/" + table_name;
    if (!std::filesystem::exists(table_path_name)) {
        throw std::ios_base::failure("Table does not exist\n");
    }

    // Нахождение стартовой позиции
    size_t insert_pos = table_positions[table_name].start; // начало секции с данными
    if (table_positions[table_name].last != std::nullopt) { // есть записи
        insert_pos = table_positions[table_name].last.value() + count_bytes_in_row(table_name); // конец файла
    }

    // Открытие таблицы
    FileStreamWithExceptions table;
    table.open_exc(table_path_name, std::ios_base::binary | std::ios_base::in | std::ios_base::out); 
    table.seekp(insert_pos, std::ios::cur);// перемещаем указатель на стартовую позицию

    for (const auto& label : table_labels[table_name]) {
        variants val = row_values.at(label.name);
        size_t val_size = sizeof(val);
        if (val_size > label.value_max_size) {
            throw "value size overflow";
        }
        // Вставляем размер последующей инфомрации
        table.write_exc(reinterpret_cast<const char*>(&val_size), sizeof(val_size));
        // NULL значение (заполняем чем угодно)
        if (label.value_default_size == 0 && std::holds_alternative<std::monostate>(val)) {
            std::string buffer(label.value_max_size, '\0');;
            table.write_exc(buffer.c_str(), label.value_max_size);
            if (label.is_ordered) (*table_ordered_indexes)[table_name][label.name][val].push_back(insert_pos);
            if (label.is_unordered) (*table_unordered_indexes)[table_name][label.name][val].push_back(insert_pos);
            continue;
        }
        // Default значение
        else if (label.value_default_size != 0 && std::holds_alternative<std::monostate>(val)) {
            val = label.value_default;
        }
        // Проверяем атрибут is_autoincrement
        if (label.is_autoincrement && std::holds_alternative<int>(val)) {
            std::get<int>(val)++;
        }
        // is_unique == false, значит просто записываем
        if (!label.is_unique) {
            write_variants_value(val, label.value_max_size, &table);
            if (label.is_ordered) (*table_ordered_indexes)[table_name][label.name][val].push_back(insert_pos);
            if (label.is_unordered) (*table_unordered_indexes)[table_name][label.name][val].push_back(insert_pos);
            continue;
        }
        // is_unique == true, значит проверяем на повторения
        if (label.is_unordered) { // есть unordered индекс
            if ((*table_unordered_indexes)[table_name][label.name][val].size() != 0) {
                throw "cant write not unique value";
            }
            write_variants_value(val, label.value_max_size, &table);
            if (label.is_ordered) (*table_ordered_indexes)[table_name][label.name][val].push_back(insert_pos);
            if (label.is_unordered) (*table_unordered_indexes)[table_name][label.name][val].push_back(insert_pos);
            continue;
        }
        else if (label.is_ordered) { // есть ordered индекс
            if ((*table_ordered_indexes)[table_name][label.name][val].size() != 0) {
                throw "cant write not unique value";
            }
            write_variants_value(val, label.value_max_size, &table);
            if (label.is_ordered) (*table_ordered_indexes)[table_name][label.name][val].push_back(insert_pos);
            if (label.is_unordered) (*table_unordered_indexes)[table_name][label.name][val].push_back(insert_pos);
            continue;
        }
        // нет индексов (проверять перебором всех)
        if (!check_unique(val, label.name, table_name, &table)) {
            throw "cant write not unique value";
        }
        write_variants_value(val, label.value_max_size, &table);
    }
    table_positions[table_name].last.value() += count_bytes_in_row(table_name);
}

void Sql::write_variants_value(const variants value, const size_t max_value_size, FileStreamWithExceptions* table) {
    size_t value_size;
    if (std::holds_alternative<std::string>(value)) {
        value_size = std::get<std::string>(value).size();
        table->write_exc(std::get<std::string>(value).c_str(), value_size);
    }
    else if (std::holds_alternative<int>(value)) {
        value_size = sizeof(int);
        table->write_exc(reinterpret_cast<const char*>(&(std::get<int>(value))), value_size);
    } 
    else if (std::holds_alternative<bool>(value)) {
        value_size = sizeof(bool);
        table->write_exc(reinterpret_cast<const char*>(&(std::get<bool>(value))), value_size);
    }

    // дополнительно отступить, незаполненное пространство
    table->seekp(max_value_size - value_size, std::ios::cur);
}

// Проверка на уникальность значения (если не подключены индексы)
bool Sql::check_unique(const variants cmp_val, const std::string& column_name, const std::string& table_name, FileStreamWithExceptions* table) {
    table->seekg(table_positions[table_name].start, std::ios::beg); // перемещаем указатель на начало данных
    
    Sql::ValueType value_type;
    size_t value_max_size;

    for (const auto& label : table_labels[table_name]) {
        if (label.name == column_name) {
            value_type = label.value_type;
            value_max_size = label.value_max_size;
            break;
        }
        // Доходим до начала нужного столбца значений
        table->seekg(sizeof(label.value_max_size) + label.value_max_size, std::ios::cur);

    }
    
    size_t bytes_in_row = count_bytes_in_row(table_name);
    size_t rows_size = (table_positions[table_name].last.value() - table_positions[table_name].start + 1) / bytes_in_row + 1; // количетсво строк
    
    for (size_t row = 0; row < rows_size; ++row) { // по всем рядам читаем значение выбранного столбца
        size_t value_size;
        table->read_exc(reinterpret_cast<char*>(&value_size), sizeof(value_size));
        if (read_variants_value(value_type, value_size, value_max_size, table) == cmp_val) {
            return false;
        }
        table->seekg(-1*(value_max_size + sizeof(value_size)), std::ios::cur); // возвращаемся в начало текущего значения
        table->seekg(bytes_in_row, std::ios::cur); // переходим на следующее значение
    }

    return true;
}

