
# добавляем в качестве static библиотеки: есть .cpp файлы, требуется компиляция файлов
add_library(setup STATIC tables_info.cpp)

# связываем, сообщаем о зависимости setup от core
target_link_libraries(setup PUBLIC core)
