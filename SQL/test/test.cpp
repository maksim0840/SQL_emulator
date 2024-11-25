#include "parser.cpp"

#include <gtest/gtest.h>

class Tests {
    static bool check_labels_in_table(const std::string& table_name, const std::vector<Sql::ColumnLabel>& labels_cmp) {
        Sql sql;
        std::vector<Sql::ColumnLabel> labels;
        sql.Sql::read_table_labels(table_name, &labels);

        size_t sz1 = labels.size();
        size_t sz2 = labels_cmp.size();
        if (sz1 != sz2) {
            return false;
        }

        for (size_t i = 0; i < sz1; ++i) {
            if (!(
                labels[i].name_size == labels_cmp[i].name_size &&
                labels[i].name == labels_cmp[i].name &&
                labels[i].value_type == labels_cmp[i].value_type &&
                labels[i].value_max_size == labels_cmp[i].value_max_size &&
                labels[i].value_default_size == labels_cmp[i].value_default_size &&
                labels[i].value_default == labels_cmp[i].value_default &&
                labels[i].is_unique == labels_cmp[i].is_unique &&
                labels[i].is_autoincrement == labels_cmp[i].is_autoincrement &&
                labels[i].is_key == labels_cmp[i].is_key
                )) {

                return false;
            }
        }

        return true;
    }

    Tests() {

    }
};

/*
Можно:
тест2) можно не указыать аттрибуты == делать скобки аттрибутов пустыми (или не делать скобки)
тест2) Между конструкциями, которые уже имеют собственный разделитель (напрмиер ':' или ',') можно использовать любые разделители из списка {' ', '\n', '\t'} или не использовать вовсе
тест2) В любом непользовательском объекте можно использовать любой регистр букв, если этот объект не имеет расширенного алфавита (имеет только буквы) ((типам данных из-за int32 нельзя менять регистр )) в любом не пользовательском объекте который не имеет расширенного алфавита для названий (т.е. везде кроме типов данных)

Нельзя:
тест3) нельзя не указывать столбцов
тест4) нельзя писать посторонние символы в разделителях
тест5) нельзя писать посторонние символы в названиях
тест6) нельзя превосходить указанное в [] размера для string и bytes
тест7) нельзя недописывать значение до указанное в [] размера для bytes
*/



TEST(create_table, test1) {
    Parser parser;
    parser.execute("create table TEST ({key, autoincrement} id : int32, {unique} login: string[32], password_hash: bytes[8], is_admin: bool = false)");

	std::vector<Sql::ColumnLabel> labels_cmp = {
        {.name_size=2, .name="id", .value_type=Sql::ValueType::INT32, .value_max_size=4, .value_default_size=0, .value_default=std::monostate{}, .is_unique=false, .is_autoincrement=true, .is_key=true},
        {.name_size=5, .name="login", .value_type=Sql::ValueType::STRING, .value_max_size=32, .value_default_size=0, .value_default=std::monostate{}, .is_unique=true, .is_autoincrement=false, .is_key=false},
        {.name_size=13, .name="password_hash", .value_type=Sql::ValueType::BYTES, .value_max_size=8, .value_default_size=0, .value_default=std::monostate{}, .is_unique=false, .is_autoincrement=false, .is_key=false},
        {.name_size=8, .name="is_admin", .value_type=Sql::ValueType::BOOL, .value_max_size=1, .value_default_size=1, .value_default=false, .is_unique=false, .is_autoincrement=false, .is_key=false}
    };

	//EXPECT_EQ(check_labels_in_table(labels_cmp, "TEST"), true);
};

// TEST(create_table, test2) {
//     std::vector<Sql::ColumnLabel> correct = {
//         {.name_size=2, .name="iD", .value_type=Sql::ValueType::INT32, .value_max_size=4, .value_default_size=0, .value_default=std::monostate{}, .is_unique=false, .is_autoincrement=false, .is_key=false},
//         {.name_size=5, .name="lOgIn", .value_type=Sql::ValueType::STRING, .value_max_size=32, .value_default_size=0, .value_default=std::monostate{}, .is_unique=true, .is_autoincrement=true, .is_key=true},
//         {.name_size=13, .name="pa99woRd_hash", .value_type=Sql::ValueType::BYTES, .value_max_size=8, .value_default_size=0, .value_default=std::monostate{}, .is_unique=false, .is_autoincrement=false, .is_key=false},
//         {.name_size=8, .name="iS_aDmIn", .value_type=Sql::ValueType::BOOL, .value_max_size=1, .value_default_size=1, .value_default=false, .is_unique=false, .is_autoincrement=false, .is_key=false}
//     }
    
// 	parser.execute("cReAtE \n tAbLe \n tEsT ({}iD:int32,{UniQue,autoiNcrement,kEy}lOgIn:string[32],   pa99woRd_hash   :   bytes \n  [ \n  8   ]   ,   iS_aDmIn  \n : \n  bool=fAlSE);");
    
// 	EXPECT_EQ(cmp_labels_vectors(), true);

// };

// TEST(create_table, test3) {
// 	parser.execute("create table test (id : int32);");

//     EXPECT_EQ(cmp_labels_vectors(), true)
// };

// TEST(create_table, test4) {
// 	EXPECT_ANY_THROW(parser.execute("create table test ();"));
// };

// TEST(create_table, test5) {
// 	EXPECT_ANY_THROW(parser.execute("create table test (id : int32[4]);"));
// };

// TEST(create_table, test6) {
// 	EXPECT_ANY_THROW(parser.execute("create table) test (id : int32);"));
// };


/*


TEST(create_table, test3) {
	
	EXPECT_EQ(1, 1);

};
// TEST(create_table, test2) {
//     run::Starter<int> starter;

//     int argc = 2;
//     char* argv[] = {"", FILE_PUSH_PATH};
// 	starter.execute(argc, argv);

// 	std::vector<int> result = {15};

// 	EXPECT_EQ(result, starter.get_result());

// };
// TEST(create_table, test3) {
//     run::Starter<int> starter;

//     int argc = 2;
//     char* argv[] = {"", FILE_PUSH_PATH};
// 	starter.execute(argc, argv);

// 	std::vector<int> result = {15};

// 	EXPECT_EQ(result, starter.get_result());

// };
// TEST(create_table, test4) {
//     run::Starter<int> starter;

//     int argc = 2;
//     char* argv[] = {"", FILE_PUSH_PATH};
// 	starter.execute(argc, argv);

// 	std::vector<int> result = {15};

// 	EXPECT_EQ(result, starter.get_result());

// };
// TEST(create_table, test5) {
//     run::Starter<int> starter;

//     int argc = 2;
//     char* argv[] = {"", FILE_PUSH_PATH};
// 	starter.execute(argc, argv);

// 	std::vector<int> result = {15};

// 	EXPECT_EQ(result, starter.get_result());

// };
*/
