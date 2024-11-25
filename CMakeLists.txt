project(SQL_project)
add_executable(main main.cpp)
add_subdirectory(SQL)
target_link_libraries(main PRIVATE SQL)