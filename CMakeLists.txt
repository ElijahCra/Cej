cmake_minimum_required(VERSION 3.28)
project(CejCompiler)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
add_subdirectory(Test)
add_subdirectory(Compiler)

message(STATUS "Current binary directory: ${CMAKE_BINARY_DIR}")
message(STATUS "Current source directory: ${CMAKE_CURRENT_SOURCE_DIR}")

add_compile_options(-Wall -Wextra -Wpedantic)

