# добавляем в качестве static библиотеки: есть .cpp файлы, требуется компиляция файлов
add_library(condition STATIC condition.cpp condition.h)

# связываем, сообщаем о зависимости condition от core
target_link_libraries(condition PUBLIC core)

# связываем condition со своей собсвтенной папкой для добавления include без указания полного пути до файлов parser тем, кто будет подключаться
target_include_directories(condition PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
