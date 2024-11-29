#pragma once
#include <stdexcept>


class SqlException : public std::exception {
private:
    std::string message;

public:
    explicit SqlException(const std::string& msg) : message(msg) {}

    const char* what() const noexcept override {
        return message.c_str();
    }
};