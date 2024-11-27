# SQL_emulator

```
cd SQL
mkdir build
cd build
cmake ..
cmake --build .
./test/test_binary
```

Сделано:
- create table
- create index (индексы работают пока только для insert при проверке unique значений)

В процессе:
- insert (в процессе создания тестов)
- select, delete (работают без condition (в процессе создания парсера условий) + в процессе создания тестов)
- join, update (не приступал)

---------------------
# Структура файла с таблицей #

0 строка:

- (8 byte) labels_size - количество колонок

1...labels_size строка:

- (8 byte) name_size, 

- (name_len byte) name,

- (4 byte) value_type, 

- (8 byte) value_max_size; // максимальный размер типа

- (8 byte) value_default_size; // размер deafault значения (0 => null)

- (value_default_len byte) char* value_default;

- (1 byte) unique attribute,

- (1 byte) autoincrement attribute,

- (1 byte) key attribute,

- (1 byte) order index,

- (1 byte) unorder index,


labels_size+1..... строка (значения таблицы):

- (8 byte) value_size + (4 byte) value, если тип INT32

- (8 byte) value_size + (1 byte) value, если тип BOOL

- (8 byte) value_size + (value_size byte) value + (max_value_size-value_size byte) padding, если тип STRING

- (8 byte) value_size + (value_size byte) value + (max_value_size-value_size byte) padding, если тип BYTES

------------------------------------

