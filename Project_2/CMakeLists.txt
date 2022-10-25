cmake_minimum_required(VERSION 3.6)

project(hw02)

set(CMAKE_C_STANDARD 99)

# Executable sources
set(SOURCES main.c minesweeper.c)

# Test sources
file(GLOB TESTS_GLOB "tests/test*.c")
foreach (file_path ${TESTS_GLOB})
    message("Found test: " ${file_path})
endforeach ()
set(TEST_SOURCES minesweeper.c
        tests/cut_main_runner.c
        tests/utils_for_tests.c
        ${TESTS_GLOB})

# Targets
set(EXECUTABLE minesweeper)
set(TESTS_EXECUTABLE minesweeper-tests)

# Executable
add_executable(${EXECUTABLE} ${SOURCES})

# Tests
add_definitions(-DCUT)
add_executable(${TESTS_EXECUTABLE} ${TEST_SOURCES})


# Configure compiler warnings
if (CMAKE_C_COMPILER_ID MATCHES Clang OR ${CMAKE_C_COMPILER_ID} STREQUAL GNU)
    # using regular Clang, AppleClang or GCC
    # Strongly suggested: neable -Werror
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall -Wextra -pedantic -Werror")
elseif (${CMAKE_C_COMPILER_ID} STREQUAL MSVC)
    # using Visual Studio C++
    target_compile_definitions(${EXECUTABLE} PRIVATE _CRT_SECURE_NO_DEPRECATE)
    target_compile_definitions(${TESTS_EXECUTABLE} PRIVATE _CRT_SECURE_NO_DEPRECATE)
    set(CMAKE_CXX_FLAGS "/permissive- /W4 /EHsc")
endif ()

if (MINGW)
    target_compile_definitions(${EXECUTABLE} PRIVATE __USE_MINGW_ANSI_STDIO=1)
    target_compile_definitions(${TESTS_EXECUTABLE} PRIVATE _CRT_SECURE_NO_DEPRECATE)
endif ()
