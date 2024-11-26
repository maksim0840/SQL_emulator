# добавляем в качестве static библиотеки: есть .cpp файлы, требуется компиляция файлов
add_library(executing STATIC basic_operations.cpp)

# связываем, сообщаем о зависимости executing от setup
target_link_libraries(executing  PUBLIC setup condition)
