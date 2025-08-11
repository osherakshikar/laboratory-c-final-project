#ifndef ERRORS_H
#define ERRORS_H
/*
* =====================================================================================
*
* Filename:  errors.h
*
* Description: Defines a centralized error handling system for the assembler.
* It includes an enumeration of all possible error codes and functions
* to log and report these errors in a structured manner.
*
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

    /* Macro Errors (Pre-Assembler) */
    ERROR_INVALID_MACRO_NAME,

    /* Syntax & Parsing Errors */
    ERROR_INVALID_LABEL,
    ERROR_UNKNOWN_COMMAND_NAME,
    ERROR_MISSING_COMMA_BETWEEN_OPERANDS,
    ERROR_EXTRANEOUS_COMMA,
    ERROR_INVALID_OPERAND_SYNTAX,
    ERROR_INVALID_NUMBER_FORMAT,
    ERROR_INVALID_STRING_FORMAT,
    ERROR_INVALID_MATRIX_DIMENSIONS,
    ERROR_INVALID_MATRIX_INITIALIZATION,
    ERROR_INVALID_REGISTER,
    ERROR_INVALID_ADDRESSING_MODE,
    ERROR_EXPECTED_OPERAND,
    ERROR_TOO_MANY_OPERANDS,
    ERROR_TRAILING_CHARACTERS,
    ERROR_INVALID_OPERAND_COUNT_FOR_COMMAND,
    ERROR_INVALID_SOURCE_ADDRESSING_MODE,
    ERROR_INVALID_DEST_ADDRESSING_MODE,

    /* Semantic Errors (First & Second Pass) */
    ERROR_DUPLICATE_LABEL_DEFINITION,
    ERROR_UNDEFINED_SYMBOL_USED,
    ERROR_EXTERNAL_SYMBOL_CANNOT_BE_ENTRY,
    ERROR_ENTRY_SYMBOL_NOT_DEFINED
} error_code_t;

/* Print only the message (to stdout). */
void print_error(int error_code);

/* Print "<file>:<line>: error: <message>" (to stdout). */
void print_error_file(const char *file_name, int error_code, int line_number);

#endif /* ERRORS_H */

