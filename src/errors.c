#include "../include/errors.h"
#include <stdio.h>


/* Local helper: map code -> constant string (no global arrays). */
static const char *error_message(const int code) {
    switch (code) {
        /* Success */
        case ERROR_OK: return "no error";

        /* General */
        case ERROR_LINE_TOO_LONG: return "line is longer than the allowed maximum";
        case ERROR_CANNOT_OPEN_FILE: return "cannot open the specified file";
        case ERROR_WRITE_FAILED: return "failed to write output";

        /* Macro (pre-assembler) */
        case ERROR_INVALID_MACRO_NAME: return "invalid macro name";

        /* Syntax & Parsing */
        case ERROR_INVALID_LABEL: return "invalid label syntax";
        case ERROR_ILLEGAL_LABEL: return "illegal label";
        case ERROR_UNKNOWN_COMMAND_NAME: return "unknown command (mnemonic not recognized)";
        case ERROR_INVALID_DATA_NAME: return "invalid data name";
        case ERROR_DATA_OVERFLOW: return "too many data items provided";
        case ERROR_INVALID_DIRECTIVE: return "invalid directive";
        case ERROR_ILLEGAL_MATRIX_SYNTAX: return "illegal matrix syntax";
        case ERROR_INVALID_OPERAND_SYNTAX: return "invalid operand syntax";
        case ERROR_INVALID_NUMBER_FORMAT: return "invalid number format";
        case ERROR_INVALID_STRING_FORMAT: return "invalid string format";
        case ERROR_INVALID_MATRIX_DIMENSIONS: return "invalid matrix dimensions";
        case ERROR_INVALID_MATRIX_INITIALIZATION: return "invalid matrix initialization";
        case ERROR_INVALID_REGISTER: return "invalid register";
        case ERROR_INVALID_ADDRESSING_MODE: return "invalid addressing mode";
        case ERROR_EXPECTED_OPERAND: return "operand expected but not found";
        case ERROR_TOO_MANY_OPERANDS: return "too many operands for this command";
        case ERROR_TRAILING_CHARACTERS: return "trailing characters after statement";
        case ERROR_INVALID_OPERAND_COUNT_FOR_COMMAND: return "invalid operand count for command";
        case ERROR_INVALID_SOURCE_ADDRESSING_MODE: return "invalid source addressing mode";
        case ERROR_INVALID_DEST_ADDRESSING_MODE: return "invalid destination addressing mode";

        /* Semantic */
        case ERROR_DUPLICATE_LABEL_DEFINITION: return "duplicate label definition";
        case ERROR_UNDEFINED_SYMBOL_USED: return "undefined symbol used";
        case ERROR_EXTERNAL_SYMBOL_CANNOT_BE_ENTRY: return "external symbol cannot be entry";
        case ERROR_ENTRY_SYMBOL_NOT_DEFINED: return "entry symbol not defined";

        default: return "unknown error code";
    }
}

void print_error(int error_code) {
    printf("error: %s\n", error_message(error_code));
}

void print_error_file(const char *file_name, int error_code, int line_number) {
    printf("There is error in %s at line:%d ERROR: %s\n", file_name, line_number, error_message(error_code));
}
