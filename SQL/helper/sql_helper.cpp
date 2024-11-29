#include "sql_helper.h"
#include <filesystem>

// Конструктор
Sql::Helper::Helper(Sql* sql) : sql_(sql) {}

// Количество байт в строке
size_t Sql::Helper::count_bytes_in_row(const std::string& table_name, const bool check_real_data) {
    if (sql_->table_positions[table_name].last == std::nullopt && check_real_data) { // если данных нет
        return 0;
    }

    size_t bytes_in_row = 0; // количество байт в строке в секции с данными
    for (auto const& label : sql_->table_labels[table_name]) {
        bytes_in_row += sizeof(label.value_max_size) + label.value_max_size;
    }

    return bytes_in_row;
}

// Количество строк в таблице
size_t Sql::Helper::count_rows_in_table(const std::string& table_name) {
    if (sql_->table_positions[table_name].last == std::nullopt) {
        return 0;
    }

    size_t bytes_in_row = count_bytes_in_row(table_name);
    return (sql_->table_positions[table_name].last.value() - sql_->table_positions[table_name].start + 1) / bytes_in_row + 1;
}

// Записать в файл таблицы переменную с типом variants
void Sql::Helper::write_variants_value(const variants value, const size_t value_max_size, FileStreamWithExceptions* table) {
    size_t value_size = get_variants_size(value);
    if (value_size > value_max_size) {
        throw "value size overflow";
    }

    if (std::holds_alternative<std::string>(value)) {
        table->write_exc(std::get<std::string>(value).c_str(), value_size);
    }
    else if (std::holds_alternative<int>(value)) {
        table->write_exc(reinterpret_cast<const char*>(&(std::get<int>(value))), value_size);
    } 
    else if (std::holds_alternative<bool>(value)) {
        table->write_exc(reinterpret_cast<const char*>(&(std::get<bool>(value))), value_size);
    }

    // Дополнительно добавить пустое пространство
    std::string buffer(value_max_size - value_size, '\0');
    table->write_exc(buffer.c_str(), value_max_size - value_size);
}

// Прочитать переменную файла таблицы с типом variants
variants Sql::Helper::read_variants_value(const Sql::ValueType value_type, const size_t value_size, const size_t value_max_size, FileStreamWithExceptions* table) {
    if (value_size > value_max_size) {
        throw "value size overflow";
    }

    variants var;
    if (value_size == 0) {
        var = std::monostate{};
    }
    else if (value_type == Sql::ValueType::INT32) {
        int value;
        table->read_exc(reinterpret_cast<char*>(&value), value_size);
        var = value;
    }
    else if (value_type == Sql::ValueType::BOOL) {
        bool value;
        table->read_exc(reinterpret_cast<char*>(&value), value_size);
        var = value;
    }
    else {
        std::vector<char> buffer(value_size);
        table->read_exc(buffer.data(), value_size); // добавить строку, через копирование веткора
        var = std::string(buffer.begin(), buffer.end());
    }

    // Дополнительно отступить пустое пространство
    table->seekg(value_max_size - value_size, std::ios::cur);
    return var;
}

// Узнать размер переменной внутри variants
size_t Sql::Helper::get_variants_size(const variants value) {
    size_t value_size = 0;

    if (std::holds_alternative<std::string>(value)) {
        value_size = std::get<std::string>(value).size();
    }
    else if (std::holds_alternative<int>(value)) {
        value_size = sizeof(int);
    } 
    else if (std::holds_alternative<bool>(value)) {
        value_size = sizeof(bool);
    }
    return value_size;
}

// Определить строку, которая соответствует значению переменной типа variants
std::string Sql::Helper::convert_variants_for_output(const variants value, const Sql::ValueType value_type) {
    if (std::holds_alternative<std::string>(value)) {
        if (value_type == Sql::ValueType::BYTES) return "0x" + std::get<std::string>(value);
        else if (value_type == Sql::ValueType::STRING) return '"' + std::get<std::string>(value) + '"';
    }
    else if (std::holds_alternative<int>(value)) {
        return std::to_string(std::get<int>(value));
    } 
    else if (std::holds_alternative<bool>(value)) {
        return std::get<bool>(value) ? "true" : "false";
    }
    return "NULL"; 
}

// Получить вектор названий столбцов таблицы
std::vector<std::string> Sql::Helper::get_label_names(const std::string& table_name, const std::unordered_set<std::string>& columns, std::vector<size_t>* paddings) {
    std::vector<std::string> label_names;
    
    auto not_exist = columns.end();
    size_t columns_size = columns.size();
    bool all_columns = ((columns_size == 1 && columns.find("*") != not_exist) || columns_size == 0);

    for (const auto& label : sql_->table_labels[table_name]) {
        if (columns.find(label.name) != not_exist || all_columns) {
            label_names.push_back(label.name);
        }
        if (paddings != nullptr) {
            paddings->push_back(label.name_size); // сохраняем размер
        }
    }

    return label_names;
}

// Получить вектор значений строк (без их размера)
std::vector<std::vector<variants>> Sql::Helper::get_data_rows(const std::unordered_set<std::string>& columns, const std::string& table_name, std::vector<Sql::ValueType>* value_types) {
    std::vector<std::vector<variants>> data_rows;

    // Проверка на наличие такой таблицы
    const std::string table_path_name = "../data/" + table_name;
    if (!std::filesystem::exists(table_path_name)) {
        throw std::ios_base::failure("Table does not exist\n");
    }
    
    // Открытие таблицы
    FileStreamWithExceptions table;
    table.open_exc(table_path_name, std::ios_base::binary | std::ios_base::in);

    table.seekg(sql_->table_positions[table_name].start, std::ios::beg); // перемещаемся на начало данных

    auto not_exist = columns.end();
    size_t columns_size = columns.size();
    bool all_columns = ((columns_size == 1 && columns.find("*") != not_exist) || columns_size == 0); // надо ли вывести все значения строк
    size_t rows_in_table = count_rows_in_table(table_name);

    // Пробегаем все строчки и сохраняем нужные столбцы
    for (size_t row = 0; row < rows_in_table; ++row) {
        std::vector<variants> row_data;
        
        for (const auto& label : sql_->table_labels[table_name]) {
            size_t value_size;
            table.read_exc(reinterpret_cast<char*>(&value_size), sizeof(value_size));
            variants var =  read_variants_value(label.value_type, value_size, label.value_max_size, &table); // читаем значение переменной
            
            if (columns.find(label.name) != not_exist || all_columns) {
                row_data.push_back(var);
                if (value_types != nullptr) value_types->push_back(label.value_type); // (опционально для select) дополнительно сохраняем тип
            }
        }
        data_rows.push_back(row_data);
    }
    
    table.close_exc();
    return data_rows;
}

// Проверяем значения на корректность и выполнение условий флагов
std::vector<std::pair<size_t, variants>> Sql::Helper::get_checked_values_to_insert(const std::string& table_name, const std::unordered_map<std::string, variants>& row_values, FileStreamWithExceptions* table) {

    std::vector<std::pair<size_t, variants>> checked_values; // сохраняем размер значения и само значение для будущей печати (сначала нужно проверить ВСЕ значения на валидность)
   
    for (const auto& label : sql_->table_labels[table_name]) {
        variants val = row_values.at(label.name);

        // Получаем размер значения
        size_t val_size = get_variants_size(val);
        if (val_size > label.value_max_size) {
            throw "value size overflow"; 
        }

        // Проверяем атрибут is_autoincrement
        if (label.is_autoincrement) {
            checked_values.push_back({sizeof(int), static_cast<int>(count_rows_in_table(table_name))});
            continue;
        }
        // NULL значение
        if (label.value_default_size == 0 && std::holds_alternative<std::monostate>(val)) {
            checked_values.push_back({val_size, ""});
            continue;
        }
        // Default значение
        if (label.value_default_size != 0 && std::holds_alternative<std::monostate>(val)) {
            val_size = label.value_default_size;
            val = label.value_default;
        }
        // is_unique == false, значит просто записываем
        if (!label.is_unique) {
            checked_values.push_back({val_size, val});
            continue;
        }
        // is_unique == true, значит проверяем на повторения
        if (label.is_unordered) { // есть unordered индекс
            if ((*(sql_->table_unordered_indexes))[table_name][label.name][val].size() != 0) {
                throw "cant write not unique value";
            }
            checked_values.push_back({val_size, val});
            continue;
        }
        else if (label.is_ordered) { // есть ordered индекс
            if ((*(sql_->table_ordered_indexes))[table_name][label.name][val].size() != 0) {
                throw "cant write not unique value";
            }
            checked_values.push_back({val_size, val});
            continue;
        }
        // нет индексов (проверять перебором все строки)
        if (!check_unique(val, label.name, table_name, table)) {
            throw "cant write not unique value";
        }
        checked_values.push_back({val_size, val});
    }

    return checked_values;
}

// Проверка на уникальность значения (если не подключены индексы)
bool Sql::Helper::check_unique(const variants cmp_val, const std::string& column_name, const std::string& table_name, FileStreamWithExceptions* table) {
    table->seekg(sql_->table_positions[table_name].start, std::ios::beg); // перемещаем указатель на начало секции с данными
    
    Sql::ValueType value_type;
    size_t value_max_size;

    for (const auto& label : sql_->table_labels[table_name]) {
        if (label.name == column_name) {
            value_type = label.value_type;
            value_max_size = label.value_max_size;
            break;
        }
        // Доходим до начала нужного столбца значений
        table->seekg(sizeof(label.value_max_size) + label.value_max_size, std::ios::cur);
    }
    
    size_t bytes_in_row = count_bytes_in_row(table_name);
    size_t rows_in_table = count_rows_in_table(table_name);
    
    for (size_t row = 0; row < rows_in_table; ++row) { // по всем рядам читаем значение выбранного столбца
        size_t value_size;
        table->read_exc(reinterpret_cast<char*>(&value_size), sizeof(value_size));
        if (read_variants_value(value_type, value_size, value_max_size, table) == cmp_val) {
            return false;
        }
        table->seekg(-1*(value_max_size + sizeof(value_max_size)), std::ios::cur); // возвращаемся в начало текущего значения
        table->seekg(bytes_in_row, std::ios::cur); // переходим на следующее значение
    }

    return true;
}