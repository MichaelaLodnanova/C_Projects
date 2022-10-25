cmake_minimum_required(VERSION 3.0)

project(hw05)

# -------------------------------------------------------------------
# BEWARE EDITING LINES BELOW
# (unless you know what you're doing of course)

# Search for source files.
# If you add a new source to your project, just run ‹cmake -B build -S .›
# or rerun CMake configuration from your IDE.
file(GLOB SOURCES "*.c")
set(SOURCES_LIB ${SOURCES})
list(FILTER SOURCES_LIB EXCLUDE REGEX "/?main\\.c$")

# Test sources
file(GLOB TESTS_GLOB "tests/*.c")
foreach (file_path ${TESTS_GLOB})
    message("Found test: " ${file_path})
endforeach ()
set(TEST_SOURCES
    tests/cut_main_runner.c
    ${TESTS_GLOB}
    ${SOURCES_LIB}
)

# Targets
set(EXECUTABLE checkperms)
set(EXECUTABLE_TESTS checkperms-tests)

# Executable
add_executable(${EXECUTABLE} ${SOURCES} main.c)
target_compile_definitions(${EXECUTABLE} PUBLIC _POSIX_C_SOURCE=200809L _DEFAULT_SOURCE=1)

# Tests
add_definitions(-DCUT)
add_executable(${EXECUTABLE_TESTS} ${TEST_SOURCES})
target_compile_definitions(${EXECUTABLE_TESTS} PUBLIC _POSIX_C_SOURCE=200809L  _DEFAULT_SOURCE=1)

# NOTE: Uncomment the following line if you want to use `getopt_long`.
target_compile_definitions(${EXECUTABLE} PUBLIC _GNU_SOURCE=1)

# Configure compiler warnings
if (CMAKE_C_COMPILER_ID MATCHES Clang OR ${CMAKE_C_COMPILER_ID} STREQUAL GNU)
  # using regular Clang, AppleClang or GCC
  # NOTE: We strongly suggest adding -Werror
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -std=c99 -Wall -Wextra -pedantic")
elseif (${CMAKE_C_COMPILER_ID} STREQUAL MSVC)
  # using Visual Studio C++
  message(FATAL_ERROR "This homework should not be compiled on Windows using MSVC since it requires POSIX. Use a Linux machine or WSL.")
endif()

if(MINGW)
    target_compile_definitions(${EXECUTABLE} PRIVATE __USE_MINGW_ANSI_STDIO=1)
    target_compile_definitions(${EXECUTABLE_TESTS} PRIVATE _CRT_SECURE_NO_DEPRECATE)
endif()
