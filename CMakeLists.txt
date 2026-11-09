cmake_minimum_required(VERSION 3.16)
project(TimeTableX LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(timetablex prototype/main.cpp)
target_include_directories(timetablex PRIVATE include)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(timetablex PRIVATE -Wall -Wextra -Wpedantic)
endif()

include(CTest)
if(BUILD_TESTING)
    add_test(NAME timetablex_self_tests COMMAND timetablex --test)
endif()
