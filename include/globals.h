#ifndef GLOBALS_H
#define GLOBALS_H
/*
 * =====================================================================================
 * Filename: globals.h
 * Description: Global definitions and utility functions for the assembler project.
 * This header file defines constants, types, and utility functions used throughout the project.
 * It includes definitions for boolean types, maximum sizes for various data structures,
 * and utility functions for string manipulation and file path creation.
 * =====================================================================================
 */
/**
 * @enum bool_t
 * @brief Boolean type definition for true/false values.
 */
typedef enum {
    FALSE = 0,
    TRUE = 1
} bool_t;

#define ADDRESS_BASE 100 /* code address base */
#define MAX_DATA_ITEMS 32 /* .data numbers per line */
#define MAX_MATRIX_ROWS 15 /* max matrix rows max 15 bits 6..9 , 2..5 */
#define MAX_MATRIX_COLS 15 /* max matrix cols max 15 bits 6..9 , 2..5 */
#define MAX_MATRIX_CELLS (MAX_MATRIX_ROWS * MAX_MATRIX_COLS)
#define MAX_LINE_LENGTH 82 /* 80 chars + newline / terminator */
#define MAX_LABEL_LENGTH  31  /* 30 chars + terminator */
#define IMAGE_LENGTH 256 /* max image size in words */
#define MAX_STRING_LEN (MAX_LINE_LENGTH -2) /* fits any single input line */


/**
 * Copy a string to a new allocated memory.
 * The caller is responsible for freeing the returned string.
 *
 * @param str The string to duplicate.
 * @return A pointer to the newly allocated string, or NULL on failure.
 */
char *dupstr(const char *str);

/**
 * Checks if a given name is a reserved keyword.
 * Returns TRUE if the name is reserved, FALSE otherwise.
 *
 * @param name The name to check.
 * @return bool_t indicating whether the name is a reserved keyword.
 */
bool_t is_reserved_keyword(const char* name);

/**
 * Creates a file path by appending an ending to a file name.
 * If the file name contains a '.', it will be removed before appending the ending.
 *
 * @param file_name The base file name.
 * @param ending The ending to append (".ob", ".ent", ".ext").
 * @return A newly allocated string containing the full file path, or NULL on failure.
 */
char *create_file_path(const char *file_name, const char *ending);
#endif
