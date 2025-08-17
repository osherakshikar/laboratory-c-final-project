#include "../include/errors.h"
#include <stdio.h>


/* Local helper: map code -> constant string (no global arrays). */
static const char *error_message(const int code) {
    switch (code) {
        /* success */
        case ERROR_OK: return "no error";

        /* general */
        case ERROR_LINE_TOO_LONG: return "line is longer than the allowed maximum";
        case ERROR_CANNOT_OPEN_FILE: return "cannot open the specified file";
        case ERROR_WRITE_FAILED: return "failed to write output";
        case ERROR_MEMORY_ALLOCATION_FAILED: return "failed to allocate memory";

        /* macro (pre-assembler) */
        case ERROR_INVALID_MACRO_NAME: return "invalid macro name";
        case ERROR_FAILED_PREPROCESSING: return "preprocessing failed, check macro definitions or file";
        case ERROR_RESERVED_MACRO_NAME: return "macro name is reserved name";
        case ERROR_TOKEN_AFTER_MACRO: return "unexpected token after macro definition";

        /* syntax & parsing */
        case ERROR_INVALID_LABEL: return "invalid label syntax";
        case ERROR_ILLEGAL_LABEL: return "illegal label";
        case ERROR_UNKNOWN_COMMAND_NAME: return "unknown command (mnemonic not recognized)";
        case ERROR_DATA_OVERFLOW: return "too many data items provided";
        case ERROR_INVALID_DIRECTIVE: return "invalid directive";
        case ERROR_INVALID_OPERAND_SYNTAX: return "invalid operand syntax";
        case ERROR_INVALID_NUMBER_FORMAT: return "invalid number format";
        case ERROR_INVALID_STRING_FORMAT: return "invalid string format";
        case ERROR_INVALID_MATRIX_DIMENSIONS: return "invalid matrix dimensions";
        case ERROR_INVALID_MATRIX_INITIALIZATION: return "invalid matrix initialization";
        case ERROR_INVALID_REGISTER: return "invalid register";
        case ERROR_INVALID_MATRIX_FORMAT: return "invalid matrix format";
        case ERROR_STRING_TOO_LONG: return "string exceeds maximum length";
        case ERROR_INVALID_ARGUMENT: return "invalid argument";
        case ERROR_INVALID_ADDRESSING_MODE: return "invalid addressing mode";
        case ERROR_EXPECTED_OPERAND: return "operand expected but not found";
        case ERROR_TRAILING_CHARACTERS: return "trailing characters after statement";
        case ERROR_DUPLICATE_LABEL_DEFINITION: return "duplicate label definition";
        case ERROR_FIRST_PASSED: return "first pass failed";
        case ERROR_UNDEFINED_SYMBOL_USED: return "undefined symbol used";
        case ERROR_EXTERNAL_SYMBOL_CANNOT_BE_ENTRY: return "external symbol cannot be entry";
        case ERROR_ENTRY_SYMBOL_NOT_DEFINED: return "entry symbol not defined";
        case ERROR_DUPLICATE_ENTRY_DECLARATION: return "duplicate entry declaration";

        default: return "unknown error code";
    }
}

void print_error(int error_code) {
    printf("error: %s\n", error_message(error_code));
}

void print_error_file(const char *file_name, int error_code, int line_number) {
    printf("There is error in %s at line:%d ERROR: %s\n", file_name, line_number, error_message(error_code));
}
