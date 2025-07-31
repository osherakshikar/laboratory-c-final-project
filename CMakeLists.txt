cmake_minimum_required(VERSION 3.31)
project(untitled1 C)

# Enforce ANSI C (C89) standard
set(CMAKE_C_STANDARD 90)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Add compile options globally
add_compile_options(-std=c89 -pedantic -Wall -fsanitize=undefined -g)

# Include directories (if needed)
include_directories(${CMAKE_SOURCE_DIR})

# Define the executable
add_executable(untitled1 main.c)