#ifndef ERRORS_H
#define ERRORS_H

/*
* =====================================================================================
* Filename:  errors.h
* Description: Defines a centralized error handling system for the assembler.
* It includes an enumeration of all possible error codes and functions
* to log and report these errors in a structured manner.
* =====================================================================================
*/

/* Single unified error code enum  */
typedef enum {
    /* Success */
    ERROR_OK = 0,

    /* General Errors */
    ERROR_LINE_TOO_LONG,
    ERROR_CANNOT_OPEN_FILE,
    ERROR_WRITE_FAILED,
    ERROR_MEMORY_ALLOCATION_FAILED,

    /* Macro Errors (Pre-Assembler) */
    ERROR_INVALID_MACRO_NAME,
    ERROR_FAILED_PREPROCESSING,
    ERROR_RESERVED_MACRO_NAME,
    ERROR_TOKEN_AFTER_MACRO,

    /* Syntax & Parsing Errors */
    ERROR_INVALID_LABEL,
    ERROR_ILLEGAL_LABEL,
    ERROR_UNKNOWN_COMMAND_NAME,
    ERROR_DATA_OVERFLOW,
    ERROR_INVALID_DIRECTIVE,
    ERROR_INVALID_OPERAND_SYNTAX,
    ERROR_INVALID_NUMBER_FORMAT,
    ERROR_INVALID_STRING_FORMAT,
    ERROR_INVALID_MATRIX_DIMENSIONS,
    ERROR_INVALID_MATRIX_INITIALIZATION,
    ERROR_INVALID_MATRIX_FORMAT,
    ERROR_INVALID_REGISTER,
    ERROR_MISSING_COMMA_BETWEEN_OPERANDS,
    ERROR_INVALID_ADDRESSING_MODE,
    ERROR_STRING_TOO_LONG,
    ERROR_INVALID_ARGUMENT,
    ERROR_EXPECTED_OPERAND,
    ERROR_TRAILING_CHARACTERS,
    ERROR_DUPLICATE_ENTRY_DECLARATION,

    /* Semantic Errors (First & Second Pass) */
    ERROR_FIRST_PASSED,
    ERROR_DUPLICATE_LABEL_DEFINITION,
    ERROR_UNDEFINED_SYMBOL_USED,
    ERROR_EXTERNAL_SYMBOL_CANNOT_BE_ENTRY,
    ERROR_ENTRY_SYMBOL_NOT_DEFINED
} error_code_t;

/**
 * Print an error message based on the error code.
 * This function maps error codes to human-readable messages.
 *
 * @param error_code The error code to print.
 */
void print_error(int error_code);

/**
 * Print an error message with file and line information.
 * This function provides detailed context for the error, including
 * the file name and line number where the error occurred.
 *
 * @param file_name The name of the file where the error occurred.
 * @param error_code The error code to print.
 * @param line_number The line number in the file where the error occurred.
 */
void print_error_file(const char *file_name, int error_code, int line_number);

#endif
