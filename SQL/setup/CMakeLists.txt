
# добавляем в качестве static библиотеки: есть .cpp файлы, требуется компиляция файлов
add_library(setup STATIC tables_info.cpp)

# связываем, сообщаем о зависимости setup от helper
target_link_libraries(setup PUBLIC helper)
