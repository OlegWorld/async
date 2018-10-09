cmake_minimum_required(VERSION 3.9)
project(async)

find_package(GTest REQUIRED)
find_package (Threads REQUIRED)

set(CMAKE_CXX_STANDARD 14)

add_library(async SHARED async.cpp async.h
                         command_reader.cpp command_reader.h
                         command_handler.cpp command_handler.h
                         command_processors.cpp command_processors.h
                         command_bulk.cpp command_bulk.h
                         thread_utility.h abstract_observer.h)

add_executable(async_test test.cpp)

add_executable(async_cli main.cpp)

target_include_directories(async_test PRIVATE ${GTEST_INCLUDE_DIRS})

set_target_properties(async PROPERTIES   CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        COMPILE_OPTIONS "-Wpedantic;-Wall;-Wextra;-Werror")

set_target_properties(async_test PROPERTIES  CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON)

target_link_libraries(async ${CMAKE_THREAD_LIBS_INIT} stdc++fs)

target_link_libraries(async_cli async)

target_link_libraries(async_test async ${GTEST_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} stdc++fs)


add_test(test async_test)

enable_testing()

#install(TARGETS async RUNTIME DESTINATION bin)
#
#set(CPACK_GENERATOR DEB)
#
#set(CPACK_PACKAGE_VERSION_MAJOR "0")
#set(CPACK_PACKAGE_VERSION_MINOR "1")
#set(CPACK_PACKAGE_VERSION_PATCH "$ENV{TRAVIS_BUILD_NUMBER}")
#
#set(CPACK_PACKAGE_CONTACT osmironov@gmail.com)
#
#include(CPack)