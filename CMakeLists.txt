cmake_minimum_required(VERSION 3.9)
project(async)

set(CMAKE_CXX_STANDARD 14)

add_library(async SHARED async.cpp async.h
                         command_reader.cpp command_reader.h
                         command_handler.cpp command_handler.h)

add_executable(async_cli main.cpp)

target_link_libraries(async_cli async)