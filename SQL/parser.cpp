
#include <vector>
#include <string>
#include <iostream>
#include <cctype>
#include "parser.h"


void Parser::skip_spaces() {
    while(input[pos] == ' ' || input[pos] == '\n') {
        ++pos;
    }
}

bool Parser::skip_comma() {
    bool comma = false;
    skip_spaces();
    if (input[pos] == ',') {
        comma = true;
        ++pos;
    }
    return comma;
}

std::string Parser::to_lower(const std::string& str) {
    std::string low_str;
    size_t str_size = str.size();

    for (size_t i = 0; i < str_size; ++i) {
        low_str.push_back(std::tolower(str[i]));
    }
    return low_str;
}

std::string Parser::expect_name(bool throw_exceptions = true) {
    skip_spaces();
    std::string name;
    while (std::isalpha(input[pos])) {
        name.push_back(input[pos]);
        ++pos;
    }
    if (input[pos] != ',' && input[pos] != ' ' && input[pos] != '\n' && input[pos] != ')' && input[pos] != '}' && input[pos] != ';' && throw_exceptions) {
        throw "unavailable symbol";
    }
    if (name.size() == 0) {
        throw "empty naming";
    }
    return name;
}

std::string Parser::expect_extended_name(bool throw_exceptions = true) {
    skip_spaces();
    std::string name;
    size_t name_size = 0;

    while (std::isalpha(input[pos]) || input[pos] == '_' || std::isdigit(input[pos])) {
        if (!std::isalpha(input[pos]) && name_size == 0) { // доп символы в начале
            throw "unvaliable first char naming";
        }
        name.push_back(input[pos]);
        ++pos;
        ++name_size;
    }
    if (input[pos] != ',' && input[pos] != ' ' && input[pos] != '\n' && input[pos] != ')' && input[pos] != '}' && input[pos] != ';' && throw_exceptions) {
        throw "unavailable symbol";
    }
    if (name.size() == 0) {
        throw "empty naming";
    }
    return name;
}
// std::vector<std::string> Parser::expect_names_before_keyword(const std::string& keyword) {
//     std::vector<std::string> names_vec;
//     while (true) {
//         std::string name = expect_name();
//         if (to_lower(name) == keyword) {
//             break;
//         }
//         names_vec.push_back(name);
//     }
//     return names_vec;
// }

int Parser::expect_value() {
    skip_spaces();
    std::string str_value;
    size_t str_value_size = 0;

    while (std::isdigit(input[pos]) || (input[pos] == '-' && str_value_size == 0)) {
        str_value.push_back(input[pos]);
        ++pos;
        ++str_value_size;
    }
    return std::stoi(str_value);
}

std::string Parser::expect_command() {
    return to_lower(expect_name());
}

void Parser::expect_keyword(const std::string& keyword) {
    std::string str =  expect_command();
    if (keyword != str) {
        throw "unknown keyword";
    }
}

void Parser::expect_ending() {
    skip_spaces();
    if (input[pos] != ';') {
        throw "cant find closing symbol";
    }
    ++pos;
}


// Прочитать значения разных типов в строку
std::string Parser::read_int32_str() {
    return std::to_string(expect_value());
}

std::string Parser::read_bool_str() {
    std::string bool_str = expect_command();
    if (bool_str != "false" && bool_str != "true") {
        throw "uncorrect bool value";
    }
    return bool_str;
}

std::string Parser::read_string_str(const size_t max_size) {
    skip_spaces();
    if (input[pos] != '"') {
        throw "cant find start of string value";
    }
    ++pos;

    std::string str;
    size_t str_size = 0;
    while (input[pos] != '"' && pos < sz) {
        if (str_size > max_size) {
            throw "string is too big";
        }
        str.push_back(input[pos]);
        ++pos;
        ++str_size;
    }
    if (pos >= sz) {
        throw "cant find end of string value";
    }
    ++pos;
    return str;
}

std::string Parser::read_bytes_str(const size_t max_size) {
    skip_spaces();
    if (input[pos] != '0' || input[pos + 1] != 'x') {
        throw "uncorrect bytes format";
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
            throw "uncorrect byte";
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
    if (input[pos] != '{') { // no attributes
        return;
    }
    ++pos;

    for (int i = 0; i < 3; ++i) {
        std::string str = expect_command();

        if (str == "unique") {
            if (label->is_unique == true) throw "dublicated attribute";
            label->is_unique = true;
        }
        else if (str == "autoincrement") {
            if (label->is_autoincrement == true) throw "dublicated attribute";
            label->is_autoincrement = true;
        }
        else if (str == "key") {
            if (label->is_key == true) throw "dublicated attribute";
            label->is_key = true;
        }
        else {
            throw "unexpected attribute";
        }

        skip_comma();
        if (input[pos] == '}') {
            ++pos;
            break;
        }
    }
    if (input[pos] != '}') {
        std::cout<<input[pos];
        exit(-1);
        throw "too many attributes";
    }
}

void Parser::expect_label_name(Sql::ColumnLabel* label) {
    std::string name = expect_extended_name();
    label->name = name;
    label->name_size = name.size();
}

int Parser::expect_label_size(Sql::ColumnLabel* label) {
    skip_spaces();
    if (input[pos] != '[') {
        throw "size must be in brackets";
    }
    ++pos;
 
    int size = expect_value();
    if (size < 0) {
        throw "size must be positive";
    }

    skip_spaces();
    if (input[pos] != ']') {
        throw "size must be in brackets";
    }
    ++pos;
    return size;
}

void Parser::expect_label_type(Sql::ColumnLabel* label) {
    skip_spaces();
    if (input[pos] != ':') { // нет типа
        throw "empty type";
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
        label->value_max_size = expect_label_size(label);
    }
    else if (str == "bytes") {
        label->value_type = Sql::ValueType::BYTES;
        label->value_max_size = expect_label_size(label);
    }
    else {
        throw "unexpected type";
    }
}

void Parser::expect_label_default(Sql::ColumnLabel* label) {
    skip_spaces();
    if (input[pos] != '=') {
        label->value_default_size = 0;
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

    if (label->value_default.size() == 0) {
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
        throw "cant open label section";
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
        throw "cant close label section";
    }
    ++pos;

    return labels_vec;
}


void Parser::execute(const std::string& str) {
    if (str.back() != ';') {
        throw "cant find closing symbol";
    }
    input = str;
    pos = 0;
    sz = str.size();

    while (pos < sz) { // если несколько запросов в одном, то проходим все
        std::string command = expect_command();

        if (command == "create") {
            std::string query_table_name;
            std::vector<Sql::ColumnLabel> query_labels;

            expect_keyword("table");
            query_table_name = expect_extended_name();
            query_labels = expect_label();
            expect_ending();

            ///!!!!!!!!!!!!!!!!!!!!!!!!!!
            std::cout << query_table_name << '\n';
            int i = 0;
            for (Sql::ColumnLabel lab : query_labels) {
                std::cout << "col" << i << ":\n";
                std::cout << "name_size" << " = " << lab.name_size << '\n';
                std::cout << "name" << " = " << lab.name << '\n';
                std::cout << "value_type" << " = " << (int)lab.value_type << '\n';
                std::cout << "value_max_size" << " = " << lab.value_max_size << '\n';
                std::cout << "value_default_size" << " = " << lab.value_default_size << '\n';
                std::cout << "value_default" << " = " << lab.value_default << '\n';
                std::cout << "is_unique" << " = " << lab.is_unique << '\n';
                std::cout << "is_autoincrement" << " = " << lab.is_autoincrement << '\n';
                std::cout << "is_key" << " = " << lab.is_key << '\n';
                ++i;
            }
        }
        // else if (command == "insert") {
        //     //
        // }
        // else if (command == "select") {
        //     //
        // }
        // else if (command == "update") {
        //     //
        // }
        // else if (command == "delete") {
        //     //
        // }
        // else {
        //     throw "unknownd command";
        // }
    }
}


int main() {
	Parser parser;
	parser.execute("create table users ({key, autoincrement} id :int32, {unique} login: string[32], password_hash: bytes[8], is_admin:bool = false);");
    return 0;
}