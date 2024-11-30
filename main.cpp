
#include "parser.h"

int main(void) {
    // Parser parser("/home/user/Documents/SQL_emulator/SQL/data"); // указать полный путь
   
   
    Parser parser("../SQL/data"); // или указать относительный путь ОТ РАСПОЛОЖЕНИЯ ИСПОЛНЯЕМОГО ФАЙЛА
    parser.execute("create table table0 ({key, autoincrement} id : int32, {unique} login: string[32], password_hash: bytes[8], is_admin: bool = false);");

    parser.execute("insert (, \"admin1\", 0x0ffffff0000, true) to table0;");
    parser.execute("insert (, \"admin2\", 0x00, false) to table0;");
    parser.execute("insert (, \"admin3\", 0xaa, true) to table0;");
  
    parser.execute("select * from table0;");
    parser.execute("delete table0;");
    
    
    return 0;
}
