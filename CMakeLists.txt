cmake_minimum_required(VERSION 3.10)
project(MainSql)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(SQL)

add_executable(main_app main.cpp)

target_link_libraries(main_app PUBLIC parser)

target_include_directories(main_app PUBLIC SQL)
