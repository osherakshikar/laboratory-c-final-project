cmake_minimum_required(VERSION 3.31)
project(untitled1 C)

# Enforce ANSI C (C89) standard
set(CMAKE_C_STANDARD 90)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)                 # suppresses GNU extensions

# Add compile options globally
add_compile_options(-std=c89 -ansi -pedantic -Wall
        -Wextra
        -Werror                                 # turn warnings → errors (optional)
        -g                                      # full debug info
)

# Include directories
#include_directories(${CMAKE_SOURCE_DIR})

# ---------------------------------------------------------------------------
# 1) Main executable (assembler project)
# ---------------------------------------------------------------------------
add_executable(assembler
        src/assembler.c
        src/assembler_types.c
        src/line_parser.c
        src/util_hash.c
        src/util_vec.c
        include/macro.h
        src/preprocessor.c
        include/globals.h)
# ---------------------------------------------------------------------------
# 2) Individual test executables
# ---------------------------------------------------------------------------
# Hash table test
add_executable(test_hash
        tests/hash_test.c
        src/util_hash.c)

# Line parser test
add_executable(test_parser
        tests/parser_test.c
        src/line_parser.c)

# Vector utility test
add_executable(test_vec
        tests/vector_test.c
        src/util_vec.c)

# Preprocessor test
add_executable(test_preprocessor
        tests/preprocessor_test.c
        src/preprocessor.c
        src/util_hash.c
        src/util_vec.c)

# ---------------------------------------------------------------------------
# 3) Optional: Create a library for shared code
# ---------------------------------------------------------------------------
#add_library(assembler_lib
#        src/assembler_types.c
#        src/line_parser.c
#        src/util_hash.c)

# ---------------------------------------------------------------------------
# 4)  Targets
# ---------------------------------------------------------------------------
#add_library(util_hash util_hash.c)
#add_executable(test_hash test_hash.c)
#target_link_libraries(test_hash util_hash)