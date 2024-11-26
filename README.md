# SQL_emulator
Структура файла с таблицей:

0 строка:
------------------
(8 byte) labels_size - количество колонок

1...labels_size строка:
----------------------
(8 byte) name_size, 
(name_len byte) name,
(4 byte) value_type, 
(8 byte) value_max_size; // максимальный размер типа
(8 byte) value_default_size; // размер deafault значения (0 => null)
(value_default_len byte) char* value_default;
(1 byte) unique attribute,
(1 byte) autoincrement attribute,
(1 byte) key attribute,

labels_size+1..... строка (значения таблицы):
-------------------------
(8 byte) value_size + (4 byte) value, если тип INT32
(8 byte) value_size + (1 byte) value, если тип BOOL
(8 byte) value_size + (value_size byte) value + (max_value_size-value_size byte) padding, если тип STRING
(8 byte) value_size + (value_size byte) value + (max_value_size-value_size byte) padding, если тип BYTES
value_size == 0 <=> None


значения для insert передаются через std::vector<std::variant<std::monostate, int, bool, std::string>> в порядке их появления, где std::monostate - нулевое значение.
-------------------------
// храним заголовки и служебную информацию о всех столбцах из каждой таблицы в структуре std::unordered_map<std::string, std::vector<ColumnLabel>> table_labels; (обращаемся к мапе по имени таблицы, и получаем вектор столбцов этой таблицы в правильно порядке)
-------------------------
если dafault значение отсутвует то value_default_size = 0;
если пользователь пропустил значение то ставиться default, если default-а нет то ставиться null (value_size = 0), если есть одновременно и deafault и unique то при попытке второй раз втсавить default значение бросается исклбючение (если значение не null)
-------------------------
default и autoincrement - взаимоисключащие





unique: все значения уникальны, но позволяет иметь повторяющиеся NULL значания (если пользователь задал ненулевое дефолтное значение при создании таблицы, то выдастся ошибка)
key: тоже самое что и unique но более оптимизированно т.к. автоматически выставляет индексы (по умолчанию: ordered), также автоматически выставляет аттрибут unique и выполняет все его условия 
autoincrement: добавляет к предыдущему значению 1 (если тип int32 и он не NULL), (если пользователь задал ненулевое дефолтное значение при создании таблицы, то выдастся ошибка)

В "вОт ТакОм" виде можно писать аттрибуты и команды без ошибок но типы данных нельзя


- В create_table передаётся vector колонок в правильном расположении для записи в качестве служебной информации.
- Служебная информация о структуре колонок читается функцией get_labels, возвращающая unorder_map со служебной информацией всех доступных колонок для более удобного обращения к ним.



НЕОБХОДИМЫЕ ПРОВЕРКИ:
1) create_table или parser: проверка на наличие повторяющиея имена колонок до момента создания таблицы