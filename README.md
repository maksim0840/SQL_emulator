# SQL_emulator
Структура файла с таблицей:

0 строка:
------------------
(8 byte) labels_size - количество колонок

1...n строка:
----------------------
(8 byte) name_size, 
(name_len byte) name,
(4 byte) value_type, 
(8 byte) value_default_size;
(value_default_len byte) char* value_default;
(1 byte) unique attribute,
(1 byte) autoincrement attribute,
(1 byte) key attribute,

- В create_table передаётся vector колонок в правильном расположении для записи в качестве служебной информации.
- Служебная информация о структуре колонок читается функцией get_labels, возвращающая unorder_map со служебной информацией всех доступных колонок для более удобного обращения к ним.



НЕОБХОДИМЫЕ ПРОВЕРКИ:
1) create_table или parser: проверка на наличие повторяющиея имена колонок до момента создания таблицы