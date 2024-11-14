project(SQL)

add_executable(sql_usage main.cpp)

add_subdirectory(td)
target_link_libraries(sql_usage PRIVATE Td::TdStatic)
set_property(TARGET sql_usage PROPERTY CXX_STANDARD 17) !!!!!! нужна минимум 17