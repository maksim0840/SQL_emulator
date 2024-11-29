# добавляем в качестве static библиотеки: есть .cpp файлы, требуется компиляция файлов
add_library(helper STATIC sql_helper.cpp sql_helper.h)

# связываем, сообщаем о зависимости helper от core
target_link_libraries(helper PUBLIC core)

# связываем helper со своей собсвтенной папкой для добавления include без указания полного пути до файлов parser тем, кто будет подключаться
target_include_directories(helper PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
