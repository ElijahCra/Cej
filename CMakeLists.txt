cmake_minimum_required(VERSION 3.28)
project(CejCompiler)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(Test)
include(FetchContent)

FetchContent_Declare(
        ELFIO
        GIT_REPOSITORY https://github.com/serge1/ELFIO.git
        GIT_TAG main
)
FetchContent_MakeAvailable(ELFIO)

FetchContent_GetProperties(ELFIO)
if(NOT elfio_POPULATED)
    FetchContent_Populate(ELFIO)
endif()

message(STATUS "Current binary directory: ${CMAKE_BINARY_DIR}")
message(STATUS "Current source directory: ${CMAKE_CURRENT_SOURCE_DIR}")

add_compile_options(-Wall -Wextra -Wpedantic)

add_executable(CejCompiler main.cpp)
target_include_directories(CejCompiler PRIVATE ${elfio_SOURCE_DIR})

