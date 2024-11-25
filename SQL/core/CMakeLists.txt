# добавляем в качестве INTERFACE библиотеки: нет cpp файлов, используется для передачи без компиляции
add_library(core INTERFACE) 

# связываем core со своей собсвтенной папкой для добавления include без указания полного пути до файлов core тем, кто будет подключаться
target_include_directories(core INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
