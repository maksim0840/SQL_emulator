# SQL_emulator #
---------------------------
## 1) Сборка самой библиотеки (+тесты) ##
SQL - директория библиотеки, где расположены все её файлы
```
cd SQL
mkdir build
cd build
cmake ..
cmake --build .
./test/test_binary
```
## 2) Сборка программы, которая хочет использовать библиотеку ##
SQL_emulator - директория с файлом main.cpp, использующим эту библиотеку
```
cd SQL_emulator
mkdir build
cd build
cmake ..
cmake --build .
./main_app
```
* временно не работает из-за использования относительных путей до папки с таблицами в библиотеке

------------------------------------------
## Прогресс ##

Сделано:
- create table
- create index (индексы работают при проверке наличия значений в столбце при вставке insert с атрибутом unique)
- insert
+- Сделано (работают без condition):
- insert (в процессе создания тестов)
- select
- delete (работают без condition)
Не приступал:
- join
- update

---------------------
## Структура файлов библиотеки ##
- core - главная папка с определениями основных функций библиотеки
- helper - папка, которая реализовывает вложенный класс helper (вложенный в класс sql), который нужен для упрощения работы основных функций
- setup - папка, которая реализует процесс инициализации базы данных при создании экземпляра класса sql (содержит конструктор sql и функции считывания файлов таблиц при инициализации)
- condition - доконца не реализованное представление о том, как должен выглядить класс condition для работы с большими выражениями в запросах
- execution - функции для записи в файлы таблиц, т.е. основные функции выполняемые sql-ем
- parser - парсинг строк, создание и обработка запросов, чтобы потом отправить их в execution
- test - тесты
- data - папка с таблицами (бинарными файлами)


-----------------------------
## Структура файла с таблицей ##

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



