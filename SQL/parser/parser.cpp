#include "parser.h"

bool Parser::is_sep() {
    if (input[pos] == ',' || input[pos] == ' ' || input[pos] == ';' || input[pos] == '\n' || input[pos] == '\t' \
    || input[pos] == '{' || input[pos] == '}' || input[pos] == '(' || input[pos] == ')') {
        return true;
    }
    return false;
}

void Parser::skip_spaces() {
    while(input[pos] == ' ' || input[pos] == '\n' || input[pos] == '\t') {
        ++pos;
    }
}

bool Parser::skip_comma() {
    skip_spaces();
    if (input[pos] == ',') {
        ++pos;
        return true;
    }
    return false;
}

std::string Parser::to_lower(const std::string& str) {
    std::string low_str;
    size_t str_size = str.size();

    for (size_t i = 0; i < str_size; ++i) {
        low_str.push_back(std::tolower(str[i]));
    }
    return low_str;
}

std::string Parser::expect_name(bool throw_exceptions) {
    skip_spaces();
    std::string name;
    while (std::isalpha(input[pos])) {
        name.push_back(input[pos]);
        ++pos;
    }
    if (!is_sep() && throw_exceptions) {
        throw SqlException("unavailable symbol");
    }
    if (name.size() == 0) {
        throw SqlException("empty naming");
    }
    return name;
}

std::string Parser::expect_extended_name(bool throw_exceptions) {
    skip_spaces();
    std::string name;
    size_t name_size = 0;

    while (std::isalpha(input[pos]) || input[pos] == '_' || std::isdigit(input[pos])) {
        if (!std::isalpha(input[pos]) && name_size == 0) { // доп символы в начале
            throw SqlException("unvaliable first char naming");
        }
        name.push_back(input[pos]);
        ++pos;
        ++name_size;
    }
    if (!is_sep() && throw_exceptions) {
        throw SqlException("unavailable symbol");
    }
    if (name.size() == 0) {
        throw SqlException("empty naming");
    }
    return name;
}

int Parser::expect_value(bool throw_exceptions) {
    skip_spaces();
    std::string str_value;
    size_t str_value_size = 0;

    while (std::isdigit(input[pos]) || (input[pos] == '-' && str_value_size == 0)) {
        str_value.push_back(input[pos]);
        ++pos;
        ++str_value_size;
    }
    if (!is_sep() && throw_exceptions) {
        throw SqlException("unavailable symbol");
    }
    if (str_value.size() == 0) {
        throw SqlException("empty value");
    }
    return std::stoi(str_value);
}

std::string Parser::expect_command() {
    return to_lower(expect_name());
}

bool Parser::expect_keyword(const std::string& keyword, bool throw_exceptions) {
    size_t prev_pos = pos;
    std::string str =  expect_command();
    if (keyword != str) {
        if (throw_exceptions) throw SqlException("unknown keyword");
        pos = prev_pos; // если ключевое слово неверное и исключение не ожидается, то возвращаемся назад (в начало предполагаемого keyword)
        return false;
    }
    return true;
}

void Parser::expect_ending() {
    skip_spaces();
    if (input[pos] != ';') {
        throw SqlException("cant find closing symbol");
    }
    ++pos;
}


// Прочитать значения разных типов в строку
int Parser::read_int32_str() {
    return expect_value();
}

bool Parser::read_bool_str() {
    std::string bool_str = expect_command();
    if (bool_str == "false") {
        return false;
    }
    else if (bool_str == "true") {
        return true;
    }
    throw SqlException("uncorrect bool value");
}

std::string Parser::read_string_str(const size_t max_size) {
    skip_spaces();
    if (input[pos] != '"') {
        throw SqlException("cant find start of string value");
    }
    ++pos;

    std::string str;
    size_t str_size = 0;
    while (input[pos] != '"' && pos < sz) {
        if (str_size > max_size) {
            throw SqlException("string is too big");
        }
        str.push_back(input[pos]);
        ++pos;
        ++str_size;
    }
    if (pos >= sz) {
        throw SqlException("cant find end of string value");
    }
    ++pos;
    return str;
}

std::string Parser::read_bytes_str(const size_t max_size, bool can_be_smaller) {
    skip_spaces();
    if (input[pos] != '0' || input[pos + 1] != 'x') {
        throw SqlException("uncorrect bytes format");
    }
    pos += 2;

    std::string bytes;
    for (size_t i = 0; i < max_size; ++i) {
        if (std::isdigit(input[pos])) {
            bytes.push_back(input[pos]);
        }
        else if ('a' <= std::tolower(input[pos]) &&  std::tolower(input[pos]) <= 'f') {
            bytes.push_back(std::tolower(input[pos]));
        }
        else {
            if (!can_be_smaller) throw SqlException("uncorrect byte");
            else break;
        }
        ++pos;
    }

    return bytes;
}


// Для create table
void Parser::expect_label_attributes(Sql::ColumnLabel* label) {
    label->is_unique = false;
    label->is_autoincrement = false;
    label->is_key = false;

    skip_spaces();
    if (input[pos] != '{') { // нет аттрибутов
        return;
    }
    ++pos;

    skip_spaces();
    if (input[pos] == '}') { // пустые скобки (нет аттрибутов)
        ++pos;
        return;
    }

    for (int i = 0; i < 3; ++i) {
        std::string str = expect_command();

        if (str == "unique") {
            if (label->is_unique == true) throw SqlException("dublicated attribute");
            label->is_unique = true;
        }
        else if (str == "autoincrement") {
            if (label->is_autoincrement == true) throw SqlException("dublicated attribute");
            label->is_autoincrement = true;
        }
        else if (str == "key") {
            if (label->is_key == true) throw SqlException("dublicated attribute");
            label->is_key = true;
        }
        else {
            throw SqlException("unexpected attribute");
        }

        skip_comma();
        if (input[pos] == '}') {
            break;
        }
    }
    if (input[pos] != '}') {
        throw SqlException("too many attributes");
    }
    ++pos;
}

void Parser::expect_label_name(Sql::ColumnLabel* label) {
    std::string name = expect_extended_name(false);
    label->name = name;
    label->name_size = name.size();
}

int Parser::expect_label_size() {
    skip_spaces();
    if (input[pos] != '[') {
        throw SqlException("size must be in brackets");
    }
    ++pos;
 
    int size = expect_value(false);
    if (size < 0) {
        throw SqlException("size must be positive");
    }

    skip_spaces();
    if (input[pos] != ']') {
        throw SqlException("size must be in brackets");
    }
    ++pos;
    return size;
}

void Parser::expect_label_type(Sql::ColumnLabel* label) {
    skip_spaces();
    if (input[pos] != ':') { // нет типа
        throw SqlException("empty type");
    }
    ++pos;

    std::string str = expect_extended_name(false);
    if (str == "int32") {
        label->value_type = Sql::ValueType::INT32;
        label->value_max_size = 4;
    }
    else if (str == "bool") {
        label->value_type = Sql::ValueType::BOOL;
        label->value_max_size = 1;
    }
    else if (str == "string") {
        label->value_type = Sql::ValueType::STRING;
        label->value_max_size = expect_label_size();
    }
    else if (str == "bytes") {
        label->value_type = Sql::ValueType::BYTES;
        label->value_max_size = expect_label_size()*2;
    }
    else {
        throw SqlException("unexpected type");
    }
}

void Parser::expect_label_default(Sql::ColumnLabel* label) {
    skip_spaces();
    if (input[pos] != '=') {
        label->value_default_size = 0;
        label->value_default = std::monostate{};
        return;
    }
    ++pos;

    if (label->value_type == Sql::ValueType::INT32) {
        label->value_default = read_int32_str();
    }
    else if (label->value_type == Sql::ValueType::BOOL) {
        label->value_default = read_bool_str();
    }
    else if (label->value_type == Sql::ValueType::STRING) {
        label->value_default = read_string_str(label->value_max_size);
    }
    else if (label->value_type == Sql::ValueType::BYTES) {
        label->value_default = read_bytes_str(label->value_max_size);
    }

    if (std::holds_alternative<std::monostate>(label->value_default)) {
        label->value_default_size = 0;
    }
    else {
        label->value_default_size = label->value_max_size;
    }
}

std::vector<Sql::ColumnLabel> Parser::expect_label() {
    std::vector<Sql::ColumnLabel> labels_vec;

    skip_spaces();
    if (input[pos] != '(') {
        throw SqlException("cant open label section");
    }
    ++pos;

    while (true) {
        Sql::ColumnLabel label;

        expect_label_attributes(&label);
        expect_label_name(&label);
        expect_label_type(&label);
        expect_label_default(&label);

        labels_vec.push_back(label);
        if(!skip_comma()) {
            break;
        }
    }

    if (input[pos] != ')') {
        throw SqlException("cant close label section");
    }
    ++pos;

    return labels_vec;
}

void Parser::determine_presets_by_attributes(std::vector<Sql::ColumnLabel>* labels) {
    size_t labels_size = labels->size();

    for (size_t i = 0; i < labels_size; ++i) {
        // Выключаем по умолчанию индексы
        (*labels)[i].is_ordered = false;
        (*labels)[i].is_unordered = false;
        // Выставляем индекс ordered и атрибут unique если есть атрибут key
        if ((*labels)[i].is_key) {
            (*labels)[i].is_ordered = true;
            (*labels)[i].is_unique = true;
        }
        // Атрибуты не могут содержать дефолтные значения
        if ((*labels)[i].value_default_size != 0 && ((*labels)[i].is_unique || (*labels)[i].is_autoincrement)) {
            throw SqlException("cant set default value with attributes");
        }
        // Если есть атрибут autoincrement то значение может быть только INT32
        if ((*labels)[i].is_autoincrement && (*labels)[i].value_type != Sql::ValueType::INT32) {
            throw SqlException("uncorrect value_type for autoincrement attribute");
        }
    }
}
    
void Parser::check_for_dublicated_columns(const std::vector<Sql::ColumnLabel>& labels) {
    std::unordered_set<std::string> unique;

    for (const auto& label : labels) {
        if (unique.find(label.name) != unique.end()) {
            throw SqlException("dublicated columns names");
        }
        unique.insert(label.name);
    }
}


Sql::IndexType Parser::expect_index_type() {
    std::string str = expect_command();
    if (str == "ordered") {
        return Sql::IndexType::ORDERED;
    }
    else if (str == "unordered") {
        return Sql::IndexType::UNORDERED;
    }
    throw("unexpected index type");
}

Parser::InsertingType Parser::determine_inserting_type() {
    size_t saved_pos = pos;

    while(input[pos] != ';') {
        if (input[pos] == '=') { 
            pos = saved_pos;
            return InsertingType::BY_ASSIGNMENT;
        }
        ++pos;
    }
    pos = saved_pos;
    return InsertingType::BY_ORDER;

    
}

void Parser::expect_order_value(std::vector<Parser::InsertingValueInfo>* row_values) {
    skip_spaces();
    
    if (input[pos] == ',' || input[pos] == ')') {
        InsertingValueInfo value_info = {.value_=std::monostate{}, .type_=Sql::ValueType::INT32};
        row_values->push_back(value_info);
    }
    else if (input[pos] == '0' && input[pos + 1] == 'x') {
        InsertingValueInfo value_info = {.value_=read_bytes_str(sz, true), .type_=Sql::ValueType::BYTES}; 
        row_values->push_back(value_info);
    }
    else if (std::isdigit(input[pos])) {
        InsertingValueInfo value_info = {.value_=read_int32_str(), .type_=Sql::ValueType::INT32}; 
        row_values->push_back(value_info);
    }
    else if (std::isalpha(input[pos])) {
        InsertingValueInfo value_info = {.value_=read_bool_str(), .type_=Sql::ValueType::BOOL}; 
        row_values->push_back(value_info);
    }
    else if (input[pos] == '"') {
        InsertingValueInfo value_info = {.value_=read_string_str(sz), .type_=Sql::ValueType::STRING};; 
        row_values->push_back(value_info);
    }
    else {
        throw SqlException("unexpected symbol");
    }
}

void Parser::expect_assignment_value(std::vector<InsertingValueInfo>* row_values) {
    skip_spaces();
    std::string name = expect_extended_name(false);

    skip_spaces();
    if (input[pos] != '=') {
        throw SqlException("unexpected symbol");
    }
    ++pos;

    expect_order_value(row_values);
    row_values->back().name_ = name;
}

Parser::InsertingType Parser::expect_row_values(std::vector<InsertingValueInfo>* row_values) {

    skip_spaces();
    if (input[pos] != '(') {
        throw SqlException("cant open row_values section");
    }
    ++pos;

    Parser::InsertingType inserting_type = determine_inserting_type();

    // Добавляем значения в соотвествии с их типом
    while (input[pos] != ';' && input[pos] != ')') {
        if (inserting_type == Parser::InsertingType::BY_ORDER) {
            expect_order_value(row_values);
        }
        else if (inserting_type == Parser::InsertingType::BY_ASSIGNMENT) {
            expect_assignment_value(row_values);
        }
        if (!skip_comma()) break;
    }
   
    if (input[pos] != ')') {
        throw SqlException("cant close row_values section");
    }
    ++pos;

    return inserting_type;
}

std::unordered_map<std::string, variants> Parser::prepare_row_order_values(const std::vector<InsertingValueInfo>& old_row_values, const std::vector<Sql::ColumnLabel>& labels) {
    std::unordered_map<std::string, variants> new_row_values;
    // Проверка полноты введённых значений
    size_t labels_size = labels.size();
    if (labels_size != old_row_values.size()) {
        throw SqlException("unequal number of row components");
    }

    for (size_t i = 0; i < labels_size; ++i) {
        // Проверка введённых значений на совпадение типов со столбцами
        if (!std::holds_alternative<std::monostate>(old_row_values[i].value_) && labels[i].value_type != old_row_values[i].type_) {
            throw SqlException("uncorrect value type of inserting column");
        }
        if (labels[i].is_autoincrement) {
            if (old_row_values[i].type_ != Sql::ValueType::INT32) throw SqlException("uncorrect value type for autoincrement column");
            if (!std::holds_alternative<std::monostate>(old_row_values[i].value_)) throw SqlException("cant set value in autoincrement column by user");
        }
        new_row_values[labels[i].name] = old_row_values[i].value_;
    }
    return new_row_values;
}

std::unordered_map<std::string, variants> Parser::prepare_row_assignment_values(const std::vector<InsertingValueInfo>& old_row_values, const std::vector<Sql::ColumnLabel>& labels) {
    std::unordered_map<std::string, variants> new_row_values;

    // Для удобного обращения
    std::unordered_map<std::string, Sql::ColumnLabel> labels_by_name;
    for (const auto& label : labels) {
        labels_by_name[label.name] = label;
        new_row_values[label.name] = std::monostate{}; // для незаполненных значений
    }
    auto not_exist = labels_by_name.end();

    for (const auto& value_info : old_row_values) {
        // Проверка введённых значений на совпадение имён со столбцами
        if (labels_by_name.find(value_info.name_) == not_exist) {
            throw SqlException("uncorrect column name");
        }
        // Проверка введённых значений на совпадение типов со столбцами
        if (!std::holds_alternative<std::monostate>(value_info.value_) && labels_by_name[value_info.name_].value_type != value_info.type_) {
            throw SqlException("uncorrect value type of inserting column");
        }
        if (labels_by_name[value_info.name_].is_autoincrement) {
            if (value_info.type_ != Sql::ValueType::INT32) throw SqlException("uncorrect value type for autoincrement column");
            if (!std::holds_alternative<std::monostate>(value_info.value_)) throw SqlException("cant set value in autoincrement column by user");
        }
        new_row_values[value_info.name_] = value_info.value_;
    }
    return new_row_values;
}


std::unordered_set<std::string>Parser::expect_columns_names() {
    std::unordered_set<std::string> columns_names;
    skip_spaces();

    if (input[pos] == '*') {
        ++pos;
        columns_names.insert("*");
        return columns_names;
    }

    while (input[pos] != ';') {
        size_t save_pos = pos;
        std::string column_name = expect_extended_name();
        
        if (column_name == "from") {
            pos = save_pos;
            break;
        }

        auto res = columns_names.insert(column_name);
        if (!res.second) { // уже существует
            throw SqlException("dublicated column names");
        }

        skip_comma();
    }

    return columns_names;
}


void Parser::execute(const std::string& str) {
    if (str.back() != ';') {
        throw SqlException("cant find closing symbol");
    }
    input = str;
    pos = 0;
    sz = str.size();

    while (pos < sz) { // если несколько запросов в одном, то проходим все
        std::string command = expect_command();

        // create table
        if (command == "create" && expect_keyword("table", false)) {
            std::string query_table_name;
            std::vector<Sql::ColumnLabel> query_labels;

            query_table_name = expect_extended_name();
            query_labels = expect_label();
            expect_ending();

            determine_presets_by_attributes(&query_labels);
            check_for_dublicated_columns(query_labels);

            sql_->create_table(query_table_name, query_labels);
        }
        // create index
        else if (command == "create") {
            std::string query_table_name;
            std::string query_column_name;
            Sql::IndexType query_index_type;

            query_index_type = expect_index_type();
            expect_keyword("index");
            expect_keyword("on");
            query_table_name = expect_extended_name();
            expect_keyword("by");
            query_column_name = expect_extended_name();
            expect_ending();

            sql_->create_index(query_table_name, query_column_name, query_index_type);
        }
        else if (command == "insert") {
            std::string query_table_name;
            std::vector<InsertingValueInfo> row_values;

            InsertingType inserting_type = expect_row_values(&row_values);
            expect_keyword("to");
            query_table_name = expect_extended_name();
            expect_ending();

            // Проверка значений и подготовка к отправке
            std::unordered_map<std::string, variants> query_row_values;
            if (inserting_type == InsertingType::BY_ORDER) {
                query_row_values = prepare_row_order_values(row_values, sql_->table_labels[query_table_name]);
            }
            else if (inserting_type == InsertingType::BY_ASSIGNMENT)  {
                query_row_values = prepare_row_assignment_values(row_values, sql_->table_labels[query_table_name]);
            }
            
            sql_->insert(query_table_name, query_row_values);
        }
        else if (command == "select") {
            // Без condition !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
            
            std::string query_table_name;
            std::unordered_set<std::string> query_column_names;

            query_column_names = expect_columns_names();
            expect_keyword("from");
            query_table_name = expect_extended_name(false);
            expect_ending();
            
            if (query_column_names.size() != 1 && query_column_names.find("*") != query_column_names.end()) {
                throw SqlException("unexpected symbol");
            }

            sql_->select(query_column_names, query_table_name);
        }
        else if (command == "delete") {
            // Без condition !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

            std::string query_table_name;
            query_table_name = expect_extended_name(false);
            expect_ending();

            sql_->delete_table(query_table_name);

        }
        // else if (command == "update") {
        //     //
        // }
        else {
            throw SqlException("unknownd command");
        }
    }
}

Parser::Parser(const std::string& data_dir) {
    sql_ = new Sql(data_dir);
}

Parser::~Parser() {
    delete sql_;
}