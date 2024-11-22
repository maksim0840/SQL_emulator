#pragma once
#include <fstream>
#include <stdexcept>

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