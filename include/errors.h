#ifndef ERRORS_H
#define ERRORS_H
#include "globals.h"
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

typedef struct {
    int error_id; /* Unique error identifier */
    char* error_message; /* Error message to be displayed */

}error;

typedef enum {
    /* General Errors */
    ERROR_LINE_TOO_LONG,

    /* Macro Errors (Pre-Assembler) */
    ERROR_INVALID_MACRO_NAME,
    ERROR_MACRO_DEFINITION_UNCLOSED,

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

    /* Semantic Errors (First & Second Pass) */
    ERROR_DUPLICATE_LABEL_DEFINITION,
    ERROR_UNDEFINED_SYMBOL_USED,
    ERROR_EXTERNAL_SYMBOL_CANNOT_BE_ENTRY,
    ERROR_ENTRY_SYMBOL_NOT_DEFINED,
    ERROR_INVALID_OPERAND_COUNT_FOR_COMMAND,
    ERROR_INVALID_SOURCE_ADDRESSING_MODE,
    ERROR_INVALID_DEST_ADDRESSING_MODE
} error_code_t;

void print_error(int error_code);

void print_error_file(const char *file_name, int error_code, int line_number);

#endif
