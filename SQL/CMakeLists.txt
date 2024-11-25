cmake_minimum_required(VERSION 3.10)
project(SQL)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# включаем поддиректории
add_subdirectory(core)
add_subdirectory(executing)
add_subdirectory(parser)
add_subdirectory(setup)
add_subdirectory(test)!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

# глобальные includ-ы
include_directories(${CMAKE_SOURCE_DIR}/parser)
