#include "../include/errors.h"
#include <stdio.h>


/* Local helper: map code -> constant string (no global arrays). */
static const char* error_message(const int code)
{
    switch (code) {
    /* Success */
    case ERROR_OK:                                return "no error";

    /* General */
    case ERROR_LINE_TOO_LONG:                     return "line too long";
    case ERROR_EMPTY_OR_COMMENT:                  return "blank/comment line";
    case ERROR_CANNOT_OPEN_FILE:                  return "cannot open file";
    case ERROR_WRITE_FAILED:                      return "write failed";

    /* Macro (pre-assembler) */
    case ERROR_INVALID_MACRO_NAME:                return "invalid macro name";

    /* Syntax & Parsing */
    case ERROR_INVALID_LABEL:                     return "invalid label";
    case ERROR_UNKNOWN_COMMAND_NAME:              return "unknown command name";
    case ERROR_MISSING_COMMA_BETWEEN_OPERANDS:    return "missing comma between operands";
    case ERROR_EXTRANEOUS_COMMA:                  return "extraneous comma";
    case ERROR_INVALID_OPERAND_SYNTAX:            return "invalid operand syntax";
    case ERROR_INVALID_NUMBER_FORMAT:             return "invalid number format";
    case ERROR_INVALID_STRING_FORMAT:             return "invalid string format";
    case ERROR_INVALID_MATRIX_DIMENSIONS:         return "invalid matrix dimensions";
    case ERROR_INVALID_MATRIX_INITIALIZATION:     return "invalid matrix initialization";
    case ERROR_INVALID_REGISTER:                  return "invalid register";
    case ERROR_INVALID_ADDRESSING_MODE:           return "invalid addressing mode";
    case ERROR_EXPECTED_OPERAND:                  return "expected operand";
    case ERROR_TOO_MANY_OPERANDS:                 return "too many operands";
    case ERROR_TRAILING_CHARACTERS:               return "trailing characters after statement";
    case ERROR_INVALID_OPERAND_COUNT_FOR_COMMAND: return "invalid operand count for command";
    case ERROR_INVALID_SOURCE_ADDRESSING_MODE:    return "invalid source addressing mode";
    case ERROR_INVALID_DEST_ADDRESSING_MODE:      return "invalid destination addressing mode";

    /* Semantic */
    case ERROR_DUPLICATE_LABEL_DEFINITION:        return "duplicate label definition";
    case ERROR_UNDEFINED_SYMBOL_USED:             return "undefined symbol used";
    case ERROR_EXTERNAL_SYMBOL_CANNOT_BE_ENTRY:   return "external symbol cannot be entry";
    case ERROR_ENTRY_SYMBOL_NOT_DEFINED:          return "entry symbol not defined";

    default:                                      return "unknown error code";
    }
}

void print_error(int error_code)
{
    printf("error: %s\n", error_message(error_code));
}

void print_error_file(const char *file_name, int error_code, int line_number)
{
    printf("There is error in %s at line:%d ERROR: %s\n", file_name, line_number, error_message(error_code));
}

