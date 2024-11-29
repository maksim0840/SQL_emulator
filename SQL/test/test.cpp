#include "test.h"
#include <gtest/gtest.h>
#include <filesystem>

bool Tests::check_labels_in_table(const std::vector<Sql::ColumnLabel>& labels_for_cmp, const std::string& table_name) {
    std::vector<Sql::ColumnLabel> labels;
    Sql db;
    db.read_table_labels(table_name, &labels);

    size_t sz1 = labels.size();
    size_t sz2 = labels_for_cmp.size();
    if (sz1 != sz2) {
        return false;
    }

    for (size_t i = 0; i < sz1; ++i) {
        if (!(
            labels[i].name_size == labels_for_cmp[i].name_size &&
            labels[i].name == labels_for_cmp[i].name &&
            labels[i].value_type == labels_for_cmp[i].value_type &&
            labels[i].value_max_size == labels_for_cmp[i].value_max_size &&
            labels[i].value_default_size == labels_for_cmp[i].value_default_size &&
            labels[i].value_default == labels_for_cmp[i].value_default &&
            labels[i].is_unique == labels_for_cmp[i].is_unique &&
            labels[i].is_autoincrement == labels_for_cmp[i].is_autoincrement &&
            labels[i].is_key == labels_for_cmp[i].is_key &&
            labels[i].is_ordered == labels_for_cmp[i].is_ordered &&
            labels[i].is_unordered == labels_for_cmp[i].is_unordered
            )) {

            return false;
        }
    }

    return true;
}

bool Tests::check_index_in_table(const Sql::IndexType index_type, const std::string& column_name, const std::string& table_name) {
    std::vector<Sql::ColumnLabel> labels;
    Sql db;
    db.read_table_labels(table_name, &labels);

    for (const auto& label : labels) {
        if (label.name == column_name) {
            if ((index_type == Sql::IndexType::ORDERED && label.is_ordered) || 
                (index_type == Sql::IndexType::UNORDERED && label.is_unordered)) {
                
                return true;
            }
            break;
        }
    }
    return false;
}

bool Tests::check_rows_in_table(const std::vector<std::vector<variants>>& rows_cmp, const std::string& table_name) {
    Sql db;

    std::vector<std::string> label_names = db.helper_->get_label_names(table_name);
    std::unordered_set<std::string> label_names_umap(label_names.begin(), label_names.end());
    std::vector<std::vector<variants>> rows = db.helper_->get_data_rows(label_names_umap, table_name);

    size_t sz1 = rows.size();
    size_t sz2 = rows_cmp.size();
    if (sz1 != sz2) {
        return false;
    }

    for (size_t i = 0; i < sz1; ++i) {

        size_t sz_sz1 = rows[i].size();
        size_t sz_sz2 = rows_cmp[i].size();
        if (sz_sz1 != sz_sz2) {
            return false;
        }

        for (size_t j = 0; j < sz_sz1; ++j) {
            if (rows[i][j] != rows_cmp[i][j]) {
                return false;
            }
        }
    }

    return true;
}




// Создание таблицы
TEST(create_table, test1) {
    Parser parser;
    parser.execute("create table TEST ({key, autoincrement} id : int32, {unique} login: string[32], password_hash: bytes[8], is_admin: bool = false);");

    std::vector<Sql::ColumnLabel> labels_cmp = {
        {.name_size=2, .name="id", .value_type=Sql::ValueType::INT32, .value_max_size=4, .value_default_size=0, .value_default=std::monostate{}, .is_unique=true, .is_autoincrement=true, .is_key=true, .is_ordered=true, .is_unordered=false},
        {.name_size=5, .name="login", .value_type=Sql::ValueType::STRING, .value_max_size=32, .value_default_size=0, .value_default=std::monostate{}, .is_unique=true, .is_autoincrement=false, .is_key=false, .is_ordered=false, .is_unordered=false},
        {.name_size=13, .name="password_hash", .value_type=Sql::ValueType::BYTES, .value_max_size=16, .value_default_size=0, .value_default=std::monostate{}, .is_unique=false, .is_autoincrement=false, .is_key=false, .is_ordered=false, .is_unordered=false},
        {.name_size=8, .name="is_admin", .value_type=Sql::ValueType::BOOL, .value_max_size=1, .value_default_size=1, .value_default=false, .is_unique=false, .is_autoincrement=false, .is_key=false, .is_ordered=false, .is_unordered=false}
    };
    EXPECT_EQ(Tests::check_labels_in_table(labels_cmp, "TEST"), true);
    if (!std::filesystem::remove("../data/TEST")) {
        throw "cant remove test table";
    }
};

// Создание таблицы с отступами и символами разных регистров
TEST(create_table, test2) {
    Parser parser;
    parser.execute("cReAtE \n tAbLe \n tEsT ({UniQue,autoiNcrement,kEy}iD:int32,{KeY}lOgIn:string[32],   pa99woRd_hash   :   bytes \n  [ \n  8   ]   ,   iS_aDmIn  \n : \n  bool=fAlSE);");
    
    std::vector<Sql::ColumnLabel> labels_cmp = {
        {.name_size=2, .name="iD", .value_type=Sql::ValueType::INT32, .value_max_size=4, .value_default_size=0, .value_default=std::monostate{}, .is_unique=true, .is_autoincrement=true, .is_key=true, .is_ordered=true, .is_unordered=false},
        {.name_size=5, .name="lOgIn", .value_type=Sql::ValueType::STRING, .value_max_size=32, .value_default_size=0, .value_default=std::monostate{}, .is_unique=true, .is_autoincrement=false, .is_key=true, .is_ordered=true, .is_unordered=false},
        {.name_size=13, .name="pa99woRd_hash", .value_type=Sql::ValueType::BYTES, .value_max_size=16, .value_default_size=0, .value_default=std::monostate{}, .is_unique=false, .is_autoincrement=false, .is_key=false, .is_ordered=false, .is_unordered=false},
        {.name_size=8, .name="iS_aDmIn", .value_type=Sql::ValueType::BOOL, .value_max_size=1, .value_default_size=1, .value_default=false, .is_unique=false, .is_autoincrement=false, .is_key=false, .is_ordered=false, .is_unordered=false}
    };

    EXPECT_EQ(Tests::check_labels_in_table(labels_cmp, "tEsT"), true);
    if (!std::filesystem::remove("../data/tEsT")) {
        throw "cant remove test table";
    }
};

// Создание небольшой таблицы
TEST(create_table, test3) {
    Parser parser;
    parser.execute("create table test (id : int32 = -200);");

    std::vector<Sql::ColumnLabel> labels_cmp = {
        {.name_size=2, .name="id", .value_type=Sql::ValueType::INT32, .value_max_size=4, .value_default_size=4, .value_default=-200, .is_unique=false, .is_autoincrement=false, .is_key=false, .is_ordered=false, .is_unordered=false}
    };
    EXPECT_EQ(Tests::check_labels_in_table(labels_cmp, "test"), true);
    if (!std::filesystem::remove("../data/test")) {
        throw "cant remove test table";
    }
};

// Создание нескольких разных (по именам) таблиц
TEST(create_table, test4) {
    Parser parser;
    parser.execute("create table TEST1 (id : int32 = -200); create table TEST2 (user : bytes[10]);");

    std::vector<Sql::ColumnLabel> labels_cmp1 = {
        {.name_size=2, .name="id", .value_type=Sql::ValueType::INT32, .value_max_size=4, .value_default_size=4, .value_default=-200, .is_unique=false, .is_autoincrement=false, .is_key=false, .is_ordered=false, .is_unordered=false}
    };
    std::vector<Sql::ColumnLabel> labels_cmp2 = {
        {.name_size=4, .name="user", .value_type=Sql::ValueType::BYTES, .value_max_size=20, .value_default_size=0, .value_default=std::monostate{}, .is_unique=false, .is_autoincrement=false, .is_key=false, .is_ordered=false, .is_unordered=false}
    };

    EXPECT_EQ(Tests::check_labels_in_table(labels_cmp1, "TEST1"), true);
    EXPECT_EQ(Tests::check_labels_in_table(labels_cmp2, "TEST2"), true);
    if (!std::filesystem::remove("../data/TEST1")) {
        throw "cant remove test table";
    }
    if (!std::filesystem::remove("../data/TEST2")) {
        throw "cant remove test table";
    }
};

// Создание нескольких одинаковых (по именам) таблиц
TEST(create_table, test5) {
    Parser parser;
    EXPECT_ANY_THROW(parser.execute("create table TEST (id : int32 = -200); create table TEST (user : bytes[10]);"));
    if (!std::filesystem::remove("../data/TEST")) {
        throw "cant remove test table";
    }
};

// Создание нескольких одинаковых (по именам) столбцов в одной таблице
TEST(create_table, test6) {
    Parser parser;
    EXPECT_ANY_THROW(parser.execute("create table TEST (id : int32, {unique} id: string[32]);"));
};

// Создание без аргументов
TEST(create_table, test7) {
   Parser parser;
	 EXPECT_ANY_THROW(parser.execute("create table test ();"));
};

// Лишний символ
TEST(create_table, test8) {
    Parser parser;
	   EXPECT_ANY_THROW(parser.execute("create table, test (id : int32 = -200);"));
};

// Тип не предусматривает указание размера
TEST(create_table, test9) {
   Parser parser;
   EXPECT_ANY_THROW(parser.execute("create table test (id : int32[4] = -200);"));
};

// Конфликтующие параметры (notnull default и unique)
TEST(create_table, test10) {
    Parser parser;
    EXPECT_ANY_THROW(parser.execute("create table TEST ({unique} value : int32 = 1001);"));
};





// Создание индекса
TEST(create_index, test1) {
    Parser parser;
    parser.execute("create table TEST ({key, autoincrement} id : int32, {unique} login: string[32], password_hash: bytes[8], is_admin: bool = false);");
    
    EXPECT_EQ(Tests::check_index_in_table(Sql::IndexType::ORDERED, "password_hash", "TEST"), false);
    parser.execute("create ordered index on TEST by password_hash;");
    EXPECT_EQ(Tests::check_index_in_table(Sql::IndexType::ORDERED, "password_hash", "TEST"), true);

    if (!std::filesystem::remove("../data/TEST")) {
        throw "cant remove test table";
    }
};

// Ordered index
TEST(create_index, test2) {
    Parser parser;
    parser.execute("create table TEST ({key, autoincrement} id : int32, {unique} login: string[32], password_hash: bytes[8], is_admin: bool = false);");
    
    EXPECT_EQ(Tests::check_index_in_table(Sql::IndexType::ORDERED, "password_hash", "TEST"), false);
    parser.execute("create ordered index on TEST by password_hash;");
    EXPECT_EQ(Tests::check_index_in_table(Sql::IndexType::ORDERED, "password_hash", "TEST"), true);

    if (!std::filesystem::remove("../data/TEST")) {
        throw "cant remove test table";
    }
};

// Unordered index
TEST(create_index, test3) {
    Parser parser;
    parser.execute("create table TEST ({key, autoincrement} id : int32, {unique} login: string[32], password_hash: bytes[8], is_admin: bool = false);");
    
    EXPECT_EQ(Tests::check_index_in_table(Sql::IndexType::ORDERED, "password_hash", "TEST"), false);
    parser.execute("create ordered index on TEST by password_hash;");
    EXPECT_EQ(Tests::check_index_in_table(Sql::IndexType::ORDERED, "password_hash", "TEST"), true);

    if (!std::filesystem::remove("../data/TEST")) {
        throw "cant remove test table";
    }
};

// 2 типа индекса для 1 столбца
TEST(create_index, test4) {
    Parser parser;
    parser.execute("create table TEST ({key, autoincrement} id : int32, {unique} login: string[32], password_hash: bytes[8], is_admin: bool = false);");
    
    EXPECT_EQ(Tests::check_index_in_table(Sql::IndexType::ORDERED, "password_hash", "TEST"), false);
    EXPECT_EQ(Tests::check_index_in_table(Sql::IndexType::UNORDERED, "password_hash", "TEST"), false);
    parser.execute("create ordered index on TEST by password_hash;");
    parser.execute("create unordered index on TEST by password_hash;");
    EXPECT_EQ(Tests::check_index_in_table(Sql::IndexType::ORDERED, "password_hash", "TEST"), true);
    EXPECT_EQ(Tests::check_index_in_table(Sql::IndexType::UNORDERED, "password_hash", "TEST"), true);

    if (!std::filesystem::remove("../data/TEST")) {
        throw "cant remove test table";
    }
};

// Индекс уже есть 
TEST(create_index, test5) {
    Parser parser;
    parser.execute("create table TEST ({key, autoincrement} id : int32, {unique} login: string[32], password_hash: bytes[8], is_admin: bool = false);");
    
    EXPECT_EQ(Tests::check_index_in_table(Sql::IndexType::ORDERED, "id", "TEST"), true);
    parser.execute("create ordered index on TEST by id;");
    EXPECT_EQ(Tests::check_index_in_table(Sql::IndexType::ORDERED, "id", "TEST"), true);

    if (!std::filesystem::remove("../data/TEST")) {
        throw "cant remove test table";
    }
};

// Нетипичное написание
TEST(create_index, test6) {
    Parser parser;
    parser.execute("create table TEST (id : int32 = -200);");

    EXPECT_EQ(Tests::check_index_in_table(Sql::IndexType::UNORDERED, "id", "TEST"), false);
    parser.execute("CrEate \n \n \t   UNorDereD     IndeX  \n  oN TEST \n\n bY    id;");
    EXPECT_EQ(Tests::check_index_in_table(Sql::IndexType::UNORDERED, "id", "TEST"), true);

    if (!std::filesystem::remove("../data/TEST")) {
        throw "cant remove test table";
    }
};

// Лишние символы
TEST(create_index, test7) {
    Parser parser;
    parser.execute("create table TEST (id : int32 = -200);");

    EXPECT_ANY_THROW(parser.execute("create unordered index, on TEST by id;"));
 
    if (!std::filesystem::remove("../data/TEST")) {
        throw "cant remove test table";
    }   
};

// Несуществующий столбец
TEST(create_index, test8) {
    Parser parser;
    parser.execute("create table TEST (id : int32 = -200);");

    EXPECT_ANY_THROW(parser.execute("create unordered index on TEST by Id;"));
 
    if (!std::filesystem::remove("../data/TEST")) {
        throw "cant remove test table";
    }   
};

// Несуществующая таблица
TEST(create_index, test9) {
    Parser parser;
    parser.execute("create table TEST (id : int32 = -200);");

    EXPECT_ANY_THROW(parser.execute("create unordered index on TesT by id;"));
 
    if (!std::filesystem::remove("../data/TEST")) {
        throw "cant remove test table";
    }   
};

// Несуществующий индекс
TEST(create_index, test10) {
    Parser parser;
    parser.execute("create table TEST (id : int32 = -200);");

    EXPECT_ANY_THROW(parser.execute("create null index on TEST by id;"));
 
    if (!std::filesystem::remove("../data/TEST")) {
        throw "cant remove test table";
    }   
};





// Методом перечисленяи всех
TEST(insert, test1) {
    Parser parser;
    parser.execute("create table TEST ({key, autoincrement} id : int32, {unique} login: string[32], password_hash: bytes[8], is_admin: bool = false);");
    parser.execute("insert ( ,\"vasya\", 0xdeadbeefdeadbeef, ) to TEST;");
    
    std::vector<std::vector<variants>> data = {{0, "vasya", "deadbeefdeadbeef", false}};
    
    EXPECT_EQ(Tests::check_rows_in_table(data, "TEST"), true);

    if (!std::filesystem::remove("../data/TEST")) {
        throw "cant remove test table";
    }   
};

// Методом присвоения
TEST(insert, test2) {
    Parser parser;
    parser.execute("create table TEST ({key, autoincrement} id : int32, {unique} login: string[32], password_hash: bytes[8], is_admin: bool = false);");
    parser.execute("insert (login = \"vasya\", password_hash = 0xdeadbeefdeadbeef) to TEST;");

    std::vector<std::vector<variants>> data = {{0, "vasya", "deadbeefdeadbeef", false}};
    
    EXPECT_EQ(Tests::check_rows_in_table(data, "TEST"), true);

    if (!std::filesystem::remove("../data/TEST")) {
        throw "cant remove test table";
    }   
};

// Замена значения у поля с default
TEST(insert, test3) {
    Parser parser;
    parser.execute("create table TEST ({key, autoincrement} id : int32, {unique} login: string[32], password_hash: bytes[8], is_admin: bool = false);");
    parser.execute("insert (,\"admin\", 0x0000000000000000, true) to TEST;");

    std::vector<std::vector<variants>> data = {{0, "admin", "0000000000000000", true}};
    
    EXPECT_EQ(Tests::check_rows_in_table(data, "TEST"), true);

    if (!std::filesystem::remove("../data/TEST")) {
        throw "cant remove test table";
    }   
};

// C разделителями и символами разных регистров
TEST(insert, test4) {
    Parser parser;
    parser.execute("create table TEST ({key, autoincrement} id : int32, {unique} login: string[32], password_hash: bytes[8], is_admin: bool = false);");
    parser.execute("InSeRt   \t  (  \n  is_admin = tRUe, login = \"admin\", password_hash = \n\n0x0000000000000000,) tO \n \n    TEST;");

    std::vector<std::vector<variants>> data = {{0, "admin", "0000000000000000", true}};
    
    EXPECT_EQ(Tests::check_rows_in_table(data, "TEST"), true);

    if (!std::filesystem::remove("../data/TEST")) {
        throw "cant remove test table";
    }   
};

// Указание значения у autoincrement
TEST(insert, test5) {
    Parser parser;
    parser.execute("create table TEST ({key, autoincrement} id : int32, {unique} login: string[32], password_hash: bytes[8], is_admin: bool = false);");
    EXPECT_ANY_THROW(parser.execute("insert (2, \"admin\", 0x0000000000000000, true) to TEST;"));

    if (!std::filesystem::remove("../data/TEST")) {
        throw "cant remove test table";
    }  
};

// Переполнение строки
TEST(insert, test6) {
    Parser parser;
    parser.execute("create table TEST ({key, autoincrement} id : int32, {unique} login: string[32], password_hash: bytes[8], is_admin: bool = false);");
    EXPECT_ANY_THROW(parser.execute("insert (, \"012345678901234567890123456789admin\", 0x0000000000000000, true) to TEST;"));

    if (!std::filesystem::remove("../data/TEST")) {
        throw "cant remove test table";
    }  
};

// Не указание "0x" в байтовой последовательности
TEST(insert, test7) {
    Parser parser;
    parser.execute("create table TEST ({key, autoincrement} id : int32, {unique} login: string[32], password_hash: bytes[8], is_admin: bool = false);");
    EXPECT_ANY_THROW(parser.execute("insert (, \"admin\", 0000000000000000, true) to TEST;"));  

    if (!std::filesystem::remove("../data/TEST")) {
        throw "cant remove test table";
    }  
};

// Указание не того типа в поле
TEST(insert, test8) {
    Parser parser;
    parser.execute("create table TEST ({key, autoincrement} id : int32, {unique} login: string[32], password_hash: bytes[8], is_admin: bool = false);");
    EXPECT_ANY_THROW(parser.execute("insert (, \"admin\", 0x0000000000000000, 101) to TEST;"));

    if (!std::filesystem::remove("../data/TEST")) {
        throw "cant remove test table";
    }  
};

// Работа autoincrement
TEST(insert, test9) {
    Parser parser;
    parser.execute("create table TEST ({key, autoincrement} id : int32, {unique} login: string[32], password_hash: bytes[8], is_admin: bool = false);");
    parser.execute("insert (, \"admin1\", 0x0ffffff0000, true) to TEST;");
    parser.execute("insert (, \"admin2\", 0x00, false) to TEST;");
    parser.execute("insert (, \"admin3\", 0xaa, true) to TEST;");

    std::vector<std::vector<variants>> data = {
        {0, "admin1", "0ffffff0000", true},
        {1, "admin2", "00", false},
        {2, "admin3", "aa", true}};

    EXPECT_EQ(Tests::check_rows_in_table(data, "TEST"), true);

    if (!std::filesystem::remove("../data/TEST")) {
        throw "cant remove test table";
    }   
};

// Пропуски значений
TEST(insert, test10) {
    Parser parser;
    parser.execute("create table TEST ({key, autoincrement} id : int32, {unique} login: string[32], password_hash: bytes[8], is_admin: bool = false);");
    parser.execute("insert (is_admin=false) to TEST;");

    std::vector<std::vector<variants>> data = {{0, std::monostate{}, std::monostate{}, false}};

    EXPECT_EQ(Tests::check_rows_in_table(data, "TEST"), true);

    if (!std::filesystem::remove("../data/TEST")) {
        throw "cant remove test table";
    }   
};





// Вывод таблицы без данных (только заголовок)
TEST(select_without_condition, test1) {
    Parser parser;

    // Создаём буфер и перенаправляем std::cout
    std::ostringstream buffer;
    std::streambuf* old_cout = std::cout.rdbuf(buffer.rdbuf());

    parser.execute("create table TEST ({key, autoincrement} id : int32, {unique} login: string[32], password_hash: bytes[8], is_admin: bool = false);");
    parser.execute("select * from TEST;");

    // Сырая строка
    std::string cmp_string = R"(id    login    password_hash    is_admin    
)";
    
    // Восстанавливаем std::cout
    std::cout.rdbuf(old_cout);

    EXPECT_EQ(cmp_string, buffer.str());

    if (!std::filesystem::remove("../data/TEST")) {
        throw "cant remove test table";
    }   
};

// Вывод большой таблицы через '*'
TEST(select_without_condition, test2) {
    Parser parser;

    // Создаём буфер и перенаправляем std::cout
    std::ostringstream buffer;
    std::streambuf* old_cout = std::cout.rdbuf(buffer.rdbuf());

    parser.execute("create table TEST ({key, autoincrement} id : int32, {unique} login: string[32], password_hash: bytes[8], is_admin: bool = false);");
    parser.execute("insert (, \"admin1\", 0x0ffffff0000, true) to TEST;");
    parser.execute("insert (, \"admin2\", 0x00, false) to TEST;");
    parser.execute("insert (, \"admin3\", 0xaa, true) to TEST;");
    parser.execute("insert (, \"tt_56\", 0xaa, true) to TEST;");
    parser.execute("select * from TEST;");

    // Сырая строка
    std::string cmp_string = \
R"(id    login       password_hash    is_admin    
0     "admin1"    0x0ffffff0000    true        
1     "admin2"    0x00             false       
2     "admin3"    0xaa             true        
3     "tt_56"     0xaa             true        
)";

    // Восстанавливаем std::cout
    std::cout.rdbuf(old_cout);

    EXPECT_EQ(cmp_string, buffer.str());

    if (!std::filesystem::remove("../data/TEST")) {
        throw "cant remove test table";
    }   
};

// Вывод большой таблицы через перечисление столбцов
TEST(select_without_condition, test3) {
    Parser parser;

    // Создаём буфер и перенаправляем std::cout
    std::ostringstream buffer;
    std::streambuf* old_cout = std::cout.rdbuf(buffer.rdbuf());

    parser.execute("create table TEST ({key, autoincrement} id : int32, {unique} login: string[32], password_hash: bytes[8], is_admin: bool = false);");
    parser.execute("insert (, \"admin1\", 0x0ffffff0000, true) to TEST;");
    parser.execute("insert (, \"admin2\", 0x00, false) to TEST;");
    parser.execute("insert (, \"admin3\", 0xaa, true) to TEST;");
    parser.execute("insert (, \"tt_56\", 0xaa, true) to TEST;");
    parser.execute("select is_admin, id from TEST;");

    // Сырая строка
    std::string cmp_string = \
R"(id    is_admin    
0     true     
1     false    
2     true     
3     true     
)";

    // Восстанавливаем std::cout
    std::cout.rdbuf(old_cout);

    EXPECT_EQ(cmp_string, buffer.str());

    if (!std::filesystem::remove("../data/TEST")) {
        throw "cant remove test table";
    }   
};

// Вывод несуществующей таблицы
TEST(select_without_condition, test4) {
    Parser parser;

    parser.execute("create table TEST ({key, autoincrement} id : int32, {unique} login: string[32], password_hash: bytes[8], is_admin: bool = false);");
    EXPECT_ANY_THROW(parser.execute("select * from TEST0;"));

    if (!std::filesystem::remove("../data/TEST")) {
        throw "cant remove test table";
    }   
};

// C разделителями и символами разных регистров
TEST(select_without_condition, test5) {
    Parser parser;

    // Создаём буфер и перенаправляем std::cout
    std::ostringstream buffer;
    std::streambuf* old_cout = std::cout.rdbuf(buffer.rdbuf());

    parser.execute("create table TEST ({key, autoincrement} id : int32, {unique} login: string[32], password_hash: bytes[8], is_admin: bool = false);");
    parser.execute("SelECT \n   \t   *  \tFRoM \t\t\t \n    TEST;");

    // Сырая строка
    std::string cmp_string = R"(id    login    password_hash    is_admin    
)";
    
    // Восстанавливаем std::cout
    std::cout.rdbuf(old_cout);

    EXPECT_EQ(cmp_string, buffer.str());

    if (!std::filesystem::remove("../data/TEST")) {
        throw "cant remove test table";
    }   
};






// Удаление пустой таблицы
TEST(delete_without_condition, test1) {
    Parser parser;

    parser.execute("create table TEST ({key, autoincrement} id : int32, {unique} login: string[32], password_hash: bytes[8], is_admin: bool = false);");
    EXPECT_NO_THROW(parser.execute("delete TEST;"));
};

// Удаление заполненной таблицы
TEST(delete_without_condition, test2) {
    Parser parser;

    parser.execute("create table TEST ({key, autoincrement} id : int32, {unique} login: string[32], password_hash: bytes[8], is_admin: bool = false);");
    parser.execute("insert (, \"admin1\", 0x0ffffff0000, true) to TEST;");
    parser.execute("insert (, \"admin2\", 0x00, false) to TEST;");
    parser.execute("insert (, \"admin3\", 0xaa, true) to TEST;");
    parser.execute("insert (, \"tt_56\", 0xaa, true) to TEST;");
    EXPECT_NO_THROW(parser.execute("delete TEST;"));
};

// C разделителями и символами разных регистров
TEST(delete_without_condition, test3) {
    Parser parser;

    parser.execute("create table TEST ({key, autoincrement} id : int32, {unique} login: string[32], password_hash: bytes[8], is_admin: bool = false);");
    EXPECT_NO_THROW(parser.execute("DELEte  \t \t   \n  TEST   \n;"));
};


// Неправильное написание таблицы при удалении
TEST(delete_without_condition, test4) {
    Parser parser;

    parser.execute("create table TEST ({key, autoincrement} id : int32, {unique} login: string[32], password_hash: bytes[8], is_admin: bool = false);");
    EXPECT_ANY_THROW(parser.execute("delete TE_ST;"));

    if (!std::filesystem::remove("../data/TEST")) {
        throw "cant remove test table";
    }  
};

// Несколько команд в строчке запросов
TEST(delete_without_condition, test5) {
    Parser parser;

    parser.execute("create table TEST1 (id : int32); create table TEST2 (name : int32);");
    EXPECT_NO_THROW(parser.execute("delete TEST1;delete TEST2;"));

};










