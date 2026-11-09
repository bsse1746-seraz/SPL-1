cmake_minimum_required(VERSION 3.16)
project(TimeTableX LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(timetablex_core STATIC
    src/app/SchedulingService.cpp
    src/core/Scheduler.cpp
    src/core/SchedulingRules.cpp
    src/core/SessionGenerator.cpp
    src/core/Validator.cpp
    src/data/DemoData.cpp
    src/io/TimetableCsv.cpp
)
target_include_directories(timetablex_core PUBLIC include)

add_executable(timetablex
    src/main.cpp
    src/cli/Console.cpp
    tests/SelfTests.cpp
)
target_include_directories(timetablex PRIVATE tests)
target_link_libraries(timetablex PRIVATE timetablex_core)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(timetablex_core PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(timetablex PRIVATE -Wall -Wextra -Wpedantic)
endif()

include(CTest)
if(BUILD_TESTING)
    add_test(NAME timetablex_self_tests COMMAND timetablex --test)
endif()
