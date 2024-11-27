#include "test.h"
#include <gtest/gtest.h>


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


TEST(create_table, test1) {
    Parser parser;
    parser.execute("create table TEST ({key, autoincrement} id : int32, {unique} login: string[32], password_hash: bytes[8], is_admin: bool = false);");

    std::vector<Sql::ColumnLabel> labels_cmp = {
        {.name_size=2, .name="id", .value_type=Sql::ValueType::INT32, .value_max_size=4, .value_default_size=0, .value_default=std::monostate{}, .is_unique=true, .is_autoincrement=true, .is_key=true, .is_ordered=true, .is_unordered=false},
        {.name_size=5, .name="login", .value_type=Sql::ValueType::STRING, .value_max_size=32, .value_default_size=0, .value_default=std::monostate{}, .is_unique=true, .is_autoincrement=false, .is_key=false, .is_ordered=false, .is_unordered=false},
        {.name_size=13, .name="password_hash", .value_type=Sql::ValueType::BYTES, .value_max_size=8, .value_default_size=0, .value_default=std::monostate{}, .is_unique=false, .is_autoincrement=false, .is_key=false, .is_ordered=false, .is_unordered=false},
        {.name_size=8, .name="is_admin", .value_type=Sql::ValueType::BOOL, .value_max_size=1, .value_default_size=1, .value_default=false, .is_unique=false, .is_autoincrement=false, .is_key=false, .is_ordered=false, .is_unordered=false}
    };
    EXPECT_EQ(Tests::check_labels_in_table(labels_cmp, "TEST"), true);
    if (!std::filesystem::remove("../data/TEST")) {
        throw "cant remove test table";
    }
};

TEST(create_table, test2) {
    Parser parser;
    parser.execute("cReAtE \n tAbLe \n tEsT ({}iD:int32,{UniQue,autoiNcrement,kEy}lOgIn:string[32],   pa99woRd_hash   :   bytes \n  [ \n  8   ]   ,   iS_aDmIn  \n : \n  bool=fAlSE);");
    
    std::vector<Sql::ColumnLabel> labels_cmp = {
        {.name_size=2, .name="iD", .value_type=Sql::ValueType::INT32, .value_max_size=4, .value_default_size=0, .value_default=std::monostate{}, .is_unique=false, .is_autoincrement=false, .is_key=false, .is_ordered=false, .is_unordered=false},
        {.name_size=5, .name="lOgIn", .value_type=Sql::ValueType::STRING, .value_max_size=32, .value_default_size=0, .value_default=std::monostate{}, .is_unique=true, .is_autoincrement=true, .is_key=true, .is_ordered=true, .is_unordered=false},
        {.name_size=13, .name="pa99woRd_hash", .value_type=Sql::ValueType::BYTES, .value_max_size=8, .value_default_size=0, .value_default=std::monostate{}, .is_unique=false, .is_autoincrement=false, .is_key=false, .is_ordered=false, .is_unordered=false},
        {.name_size=8, .name="iS_aDmIn", .value_type=Sql::ValueType::BOOL, .value_max_size=1, .value_default_size=1, .value_default=false, .is_unique=false, .is_autoincrement=false, .is_key=false, .is_ordered=false, .is_unordered=false}
    };
    EXPECT_EQ(Tests::check_labels_in_table(labels_cmp, "tEsT"), true);
    if (!std::filesystem::remove("../data/tEsT")) {
        throw "cant remove test table";
    }
};

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
        {.name_size=4, .name="user", .value_type=Sql::ValueType::BYTES, .value_max_size=10, .value_default_size=0, .value_default=std::monostate{}, .is_unique=false, .is_autoincrement=false, .is_key=false, .is_ordered=false, .is_unordered=false}
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

// ordered index
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

// unordered index
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

// индекс уже есть 
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

// нетипичное написание
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

// лишние символы
TEST(create_index, test7) {
    Parser parser;
    parser.execute("create table TEST (id : int32 = -200);");

    EXPECT_ANY_THROW(parser.execute("create unordered index, on TEST by id;"));
 
    if (!std::filesystem::remove("../data/TEST")) {
        throw "cant remove test table";
    }   
};

// несуществующий столбец
TEST(create_index, test8) {
    Parser parser;
    parser.execute("create table TEST (id : int32 = -200);");

    EXPECT_ANY_THROW(parser.execute("create unordered index on TEST by Id;"));
 
    if (!std::filesystem::remove("../data/TEST")) {
        throw "cant remove test table";
    }   
};

// несуществующая таблица
TEST(create_index, test9) {
    Parser parser;
    parser.execute("create table TEST (id : int32 = -200);");

    EXPECT_ANY_THROW(parser.execute("create unordered index on TesT by id;"));
 
    if (!std::filesystem::remove("../data/TEST")) {
        throw "cant remove test table";
    }   
};

// несуществующий индекс
TEST(create_index, test10) {
    Parser parser;
    parser.execute("create table TEST (id : int32 = -200);");

    EXPECT_ANY_THROW(parser.execute("create null index on TEST by id;"));
 
    if (!std::filesystem::remove("../data/TEST")) {
        throw "cant remove test table";
    }   
};

