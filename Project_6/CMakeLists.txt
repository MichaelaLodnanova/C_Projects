cmake_minimum_required(VERSION 3.0)

# Project configuration
project(hw06)
file(GLOB SOURCES "src/*.c")
set(EXECUTABLE pandemic)

# Tests
file(GLOB TESTS_GLOB "tests/*.c")
set(SOURCES_LIB ${SOURCES})
list(FILTER SOURCES_LIB EXCLUDE REGEX "/?main\\.c$")

foreach (file_path ${TESTS_GLOB})
    message("Found test: " ${file_path})
endforeach ()
set(TEST_SOURCES
    tests/cut_main_runner.c
    tests/exitus.c
    tests/wmain.c
    ${TESTS_GLOB}
    ${SOURCES_LIB}
)
add_definitions(-DCUT -DWRAP_INDIRECT)
add_executable(pandemic-tests ${TEST_SOURCES})

# Executable
add_executable(pandemic ${SOURCES})

# Configure compiler warnings
if (CMAKE_C_COMPILER_ID MATCHES Clang OR ${CMAKE_C_COMPILER_ID} STREQUAL GNU)
    # using regular Clang, AppleClang or GCC
    # Strongly suggested: neable -Werror
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -Wall -Wextra -pedantic -g")
elseif (${CMAKE_C_COMPILER_ID} STREQUAL MSVC)
    # using Visual Studio C++
    target_compile_definitions(${EXECUTABLE} PRIVATE _CRT_SECURE_NO_DEPRECATE)
    target_compile_definitions(${EXECUTABLE_TESTS} PRIVATE _CRT_SECURE_NO_DEPRECATE)
    set(CMAKE_CXX_FLAGS "/permissive- /W4 /EHsc")
endif()

if(MINGW)
    target_compile_definitions(${EXECUTABLE} PRIVATE __USE_MINGW_ANSI_STDIO=1)
    target_compile_definitions(${EXECUTABLE_TESTS} PRIVATE _CRT_SECURE_NO_DEPRECATE)
endif()
