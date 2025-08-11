#include "../include/errors.h"

#include <stdio.h>
error errors[] = {
    {ERROR_LINE_TOO_LONG, "Line too long."},
    {ERROR_INVALID_MACRO_NAME, "Invalid macro name."},
    {ERROR_MACRO_DEFINITION_UNCLOSED, "Macro definition not closed."},
    {ERROR_INVALID_LABEL, "Invalid label."},
    {ERROR_UNKNOWN_COMMAND_NAME, "Unknown command name."},
    {ERROR_MISSING_COMMA_BETWEEN_OPERANDS, "Missing comma between operands."},
    {ERROR_EXTRANEOUS_COMMA, "Extraneous comma found."},
    {ERROR_INVALID_OPERAND_SYNTAX, "Invalid operand syntax."},
    {ERROR_INVALID_NUMBER_FORMAT, "Invalid number format."},
    {ERROR_INVALID_STRING_FORMAT, "Invalid string format."},
    {ERROR_INVALID_MATRIX_DIMENSIONS, "Invalid matrix dimensions."},
    {ERROR_INVALID_MATRIX_INITIALIZATION, "Invalid matrix initialization."},
    {ERROR_DUPLICATE_LABEL_DEFINITION, "Duplicate label definition."},
    {ERROR_UNDEFINED_SYMBOL_USED, "Undefined symbol used."},
    {ERROR_EXTERNAL_SYMBOL_CANNOT_BE_ENTRY, "External symbol cannot be an entry."},
    {ERROR_ENTRY_SYMBOL_NOT_DEFINED, "Entry symbol not defined."},
    {ERROR_INVALID_OPERAND_COUNT_FOR_COMMAND, "Invalid operand count for command."},
    {ERROR_INVALID_SOURCE_ADDRESSING_MODE, "Invalid source addressing mode."},
    {ERROR_INVALID_DEST_ADDRESSING_MODE, "Invalid destination addressing mode."}
};

void print_error(int error_code) {
    printf("ERROR: ID:%d~~ | %s\n", error_code, errors[error_code].error_message);
}

void print_error_file(const char *file_name, int error_code, int line_number) {
    if (file_name) {
        printf("ERROR in file '%s' at line %d: %s\n", file_name, line_number, errors[error_code].error_message);
    } else {
        printf("ERROR at line %d: %s\n", line_number, errors[error_code].error_message);
    }
}
