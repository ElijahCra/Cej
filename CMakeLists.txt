cmake_minimum_required(VERSION 3.28)
project(CejCompiler)

set(CMAKE_CXX_STANDARD 20)

add_executable(CejCompiler main.cpp
        Nodes.cpp
        Generator.cpp)
