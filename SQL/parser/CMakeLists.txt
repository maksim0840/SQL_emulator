# добавляем в качестве static библиотеки: есть .cpp файлы, требуется компиляция файлов
add_library(parser STATIC parser.cpp parser.h)

# связываем, сообщаем о зависимости parser от executing
target_link_libraries(parser PUBLIC executing)

# связываем parser со своей собсвтенной папкой для добавления include без указания полного пути до файлов parser тем, кто будет подключаться
target_include_directories(parser PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})