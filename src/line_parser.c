#include "../include/line_parser.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

/*
 * =====================================================================================
 * Filename:  line_parser.c
 * Description: Parses a single line of assembly code, extracting labels, directives,
 * and operations with their operands. It handles whitespace, comments, and various
 * syntax rules for labels, directives, and instructions.
 * This module provides function to parse line, identify errors, and return structured
 * data representing the parsed line.
 * =====================================================================================
 */


/* --- Private Helper Functions --- */

/* -- whitespace & comment helpers -- */

/* Remove comment from semicolon to end of line (if exists).
 * Modifies the line in place.
 */
static void remove_comment_from_semicolon(const char *line) {
    char *semicolon = strchr(line, ';');
    if (semicolon) *semicolon = '\0';
}

/* Skip leading whitespace characters in a string.
 * Returns a pointer to the first non-whitespace character.
 */
static char *skip_leading_whitespace(char *p) {
    while (*p && isspace((unsigned char) p[0])) p++;
    return p;
}

/* Trim trailing whitespace in place.
 * Modifies the string to remove trailing spaces, tabs, and newlines.
 */
static void trim_trailing_whitespace_inplace(char *p) {
    char *end = p + strlen(p);
    while (end > p && isspace((unsigned char) end[-1])) end--;
    *end = '\0';
}

/* -- identifier & small utilities -- */

/* Retrieves the next token from a string, advancing a cursor.
 * The token is delimited by any character in the `delimiters` string.
 * Leading whitespace is skipped, and trailing whitespace is trimmed from the token.
 * return A pointer to the null-terminated token, or NULL if no token is found.
 */
static char *next_token(char **cursor, const char *delimiters) {
    char *token_start;
    char *token_end;

    token_start = skip_leading_whitespace(*cursor);
    if (!*token_start) {
        *cursor = token_start;
        return NULL;
    }

    token_end = strpbrk(token_start, delimiters);
    if (token_end) {
        *token_end = '\0';
        *cursor = token_end + 1;
    } else {
        *cursor = token_start + strlen(token_start);
    }

    trim_trailing_whitespace_inplace(token_start);
    return token_start;
}

/* Validates if a string is a legal label.
 * A legal label starts with a letter and is followed by letters or digits .
 */
static bool_t is_valid_label(const char *label) {
    int i, len;

    len = (int) strlen(label);
    if (len == 0 || len >= MAX_LABEL_LENGTH || !isalpha((unsigned char) label[0])) {
        return FALSE;
    }
    for (i = 1; i < len; ++i) {
        if (!isalnum((unsigned char) label[i])) {
            return FALSE;
        }
    }
    return TRUE;
}

/* Parses a string into an integer, ensuring no trailing characters exist. */
static error_code_t parse_integer(const char *s, int *out_val) {
    char *endptr;
    *out_val = (int) strtol(s, &endptr, 10);
    if (*endptr != '\0' || endptr == s) {
        return ERROR_INVALID_NUMBER_FORMAT;
    }
    return ERROR_OK;
}

/* -- registers & operands -- */

/* Parse register token (r0..r7 or r8..r9). Returns 0..7 for r0..r7, -2 for r8/r9, -1 if not register. */
static int parse_register_token(const char *tok) {
    if (tok[0] == 'r' && tok[1] >= '0' && tok[1] <= '7' && tok[2] == '\0') return tok[1] - '0';
    if (tok[0] == 'r' && isdigit((unsigned char) tok[1]) && tok[2] == '\0') return -2;
    return -1;
}

/* Parse immediate token . Returns 1 on success, 0 if not immediate, negative on error.
 * Sets out_value to the parsed integer value.
 */
static int parse_immediate_token(const char *tok, int *out_value) {
    error_code_t err;
    if (tok[0] != '#') return 0;
    err = parse_integer(tok + 1, out_value);
    return (err == ERROR_OK) ? 1 : -((int) err);
}

/* Parse matrix access token [base_label][rX][rY].
 * Returns 1 on success, native on error, 0 if not a matrix access.
 * Sets out_op to the parsed operand with mode MATRIX_ACCESS.
 */
static int parse_matrix_access_token(const char *tok, operand_t *out_op) {
    const char *first_open, *first_close, *second_open, *second_close;
    char buf[MAX_LABEL_LENGTH];
    long len;
    int row_reg, col_reg;

    first_open = strchr(tok, '[');
    if (!first_open || first_open == tok) return 0; /* no '[' or starts with it (no label) */

    first_close = strchr(first_open, ']');
    if (!first_close) return 0; /* treat as direct label if no ']' found */

    second_open = strchr(first_close, '[');
    if (!second_open || second_open != first_close + 1) return 0; /* second '[' must be right after ']' */

    second_close = strchr(second_open, ']');
    if (!second_close || *(second_close + 1) != '\0') return 0; /* must end with ']' and have no trailing chars */

    /* validate and copy base label */
    len = (long) (first_open - tok);
    strncpy(buf, tok, len);
    buf[len] = '\0';
    if (!is_valid_label(buf)) return -ERROR_ILLEGAL_LABEL;
    strcpy(out_op->value.label, buf);

    /* parse first register */
    len = (long) (first_close - (first_open + 1));
    if (len <= 0 || len >= (int) sizeof(buf)) return -ERROR_INVALID_REGISTER;
    strncpy(buf, first_open + 1, len);
    buf[len] = '\0';
    if ((row_reg = parse_register_token(buf)) < 0) return -ERROR_INVALID_REGISTER;

    /* parse second register */
    len = (long) (second_close - (second_open + 1));
    if (len <= 0 || len >= (int) sizeof(buf)) return -ERROR_INVALID_REGISTER;
    strncpy(buf, second_open + 1, len);
    buf[len] = '\0';
    if ((col_reg = parse_register_token(buf)) < 0) return -ERROR_INVALID_REGISTER;

    out_op->mode = MATRIX_ACCESS;
    out_op->row_reg = row_reg;
    out_op->col_reg = col_reg;
    return 1;
}

/* parses a single operand token into an operand_t struct */
static error_code_t parse_operand(const char *tok, operand_t *out_op) {
    int status, value, reg;

    if (!tok || !*tok) return ERROR_EXPECTED_OPERAND;

    /* try parsing as immediate: #number */
    status = parse_immediate_token(tok, &value);
    if (status < 0) return (error_code_t) (-status);
    if (status == 1) {
        out_op->mode = IMMEDIATE;
        out_op->value.immediate_value = value;
        return ERROR_OK;
    }

    /* try parsing as register: r0-r7 */
    reg = parse_register_token(tok);
    if (reg >= 0) {
        out_op->mode = REGISTER_DIRECT;
        out_op->value.reg_num = reg;
        return ERROR_OK;
    }
    if (reg == -2) return ERROR_INVALID_REGISTER;

    /* try parsing as matrix access: LABEL[rX][rY] */
    status = parse_matrix_access_token(tok, out_op);
    if (status < 0) return (error_code_t) (-status);
    if (status == 1) return ERROR_OK;

    /* Fallback to direct label */
    if (!is_valid_label(tok)) return ERROR_ILLEGAL_LABEL;
    out_op->mode = DIRECT;
    strcpy(out_op->value.label, tok);
    return ERROR_OK;
}

/* -- opcode/directive lookup -- */

/* Struct to describe an opcode with its mnemonic and required operand count. */
typedef struct {
    const char *mnemonic;
    op_code_t opcode;
    int required_operands;
} opcode_desc_t;

/* List of all opcodes with their mnemonics and required operand counts.
 * The last entry is a sentinel with mnemonic NULL to mark the end of the list.
 */
static const opcode_desc_t OPCODES[] = {
    {"mov", MOV_OP, 2}, {"cmp", CMP_OP, 2}, {"add", ADD_OP, 2}, {"sub", SUB_OP, 2},
    {"lea", LEA_OP, 2}, {"clr", CLR_OP, 1}, {"not", NOT_OP, 1}, {"inc", INC_OP, 1},
    {"dec", DEC_OP, 1}, {"jmp", JMP_OP, 1}, {"bne", BNE_OP, 1}, {"jsr", JSR_OP, 1},
    {"red", RED_OP, 1}, {"prn", PRN_OP, 1}, {"rts", RTS_OP, 0}, {"stop", STOP_OP, 0},
    {0, UNKNOWN_OP, 0}
};

/* Lookup opcode by mnemonic.
 * Returns the corresponding op_code_t or UNKNOWN_OP if not found.
 * Sets out_required to the number of operands required for this opcode.
 */
static op_code_t lookup_opcode_by_mnemonic(const char *tok, int *out_required) {
    int i;
    for (i = 0; OPCODES[i].mnemonic; ++i) {
        if (strcmp(OPCODES[i].mnemonic, tok) == 0) {
            *out_required = OPCODES[i].required_operands;
            return OPCODES[i].opcode;
        }
    }
    *out_required = 0;
    return UNKNOWN_OP;
}

/* Lookup directive by token.
 * Returns the corresponding directive_t or -1 if not found.
 * The returned value can be used to determine the type of directive.
 */
static directive_t lookup_directive_by_token(const char *tok) {
    if (strcmp(tok, ".data") == 0) return DATA_DIRECTIVE;
    if (strcmp(tok, ".string") == 0) return STRING_DIRECTIVE;
    if (strcmp(tok, ".mat") == 0) return MATRIX_DIRECTIVE;
    if (strcmp(tok, ".entry") == 0) return ENTRY_DIRECTIVE;
    if (strcmp(tok, ".extern") == 0) return EXTERN_DIRECTIVE;
    return (directive_t) -1;
}

/* -- directive parsers -- */

/* Parse the payload of a .data directive.
 * Expects a comma-separated list of integers.
 * Fills out_values with the parsed integers and sets count.
 * Returns ERROR_OK on success, or an error code on failure.
 */
static error_code_t parse_data_directive_payload(char *payload, int_array_t *out_values) {
    char *token;
    int val;
    error_code_t err;

    out_values->count = 0;

    if (!payload || !*payload) return ERROR_EXPECTED_OPERAND;

    while ((token = next_token(&payload, ",")) != NULL) {
        if (!*token) return ERROR_TRAILING_CHARACTERS; /* handles cases like "1, ,2" or "1," */
        if (out_values->count >= MAX_DATA_ITEMS) return ERROR_DATA_OVERFLOW;

        err = parse_integer(token, &val);
        if (err != ERROR_OK) return err;
        out_values->values[out_values->count++] = val;
    }
    return ERROR_OK;
}

/* Parse the payload of a .string directive.
 * Expects a string enclosed in double quotes.
 * Fills dest with the parsed string and returns ERROR_OK on success.
 * Returns an error code on failure.
 */
static error_code_t parse_string_directive_payload(char *payload, char dest[MAX_STRING_LEN]) {
    char *start, *end;
    size_t len;

    start = skip_leading_whitespace(payload);
    if (*start != '"') return ERROR_INVALID_STRING_FORMAT;

    end = strrchr(start + 1, '"');
    if (!end) return ERROR_INVALID_STRING_FORMAT;

    len = end - (start + 1);
    if (len >= MAX_STRING_LEN) return ERROR_STRING_TOO_LONG;

    strncpy(dest, start + 1, len);
    dest[len] = '\0';

    /* check for trailing non-whitespace characters */
    if (*skip_leading_whitespace(end + 1) != '\0') return ERROR_TRAILING_CHARACTERS;

    return ERROR_OK;
}

/* Extracts an integer from a bracketed expression [value].
 * Advances the cursor to the position after the closing bracket.
 * Returns ERROR_OK on success, or an error code on failure.
 */
static error_code_t extract_bracketed_integer(char **cursor, int *out_val) {
    char *start;
    char *open_bracket, *close_bracket, *content;
    error_code_t err;

    start = *cursor;
    open_bracket = strchr(start, '[');
    if (!open_bracket) return ERROR_INVALID_MATRIX_DIMENSIONS;

    close_bracket = strchr(open_bracket + 1, ']');
    if (!close_bracket) return ERROR_INVALID_MATRIX_DIMENSIONS;

    *close_bracket = '\0'; /* temporarily terminate to parse content */
    content = open_bracket + 1;
    err = parse_integer(content, out_val);
    *close_bracket = ']'; /* restore original char */

    if (err != ERROR_OK) return err;

    *cursor = close_bracket + 1;
    return ERROR_OK;
}

/* Parse the payload of a .mat directive.
 * Expects a matrix definition in the format [rows][cols][v1, v2, ...].
 * Fills out_mat with the parsed matrix definition and returns ERROR_OK on success.
 * Returns an error code on failure.
 */
static error_code_t parse_matrix_directive_payload(char *payload, matrix_def_t *out_mat) {
    int i, val, total_cells;
    error_code_t err;
    char *cursor, *token;

    cursor = payload;

    /* parse [rows] and [cols] */
    if ((err = extract_bracketed_integer(&cursor, &out_mat->rows)) != ERROR_OK) return err;
    if ((err = extract_bracketed_integer(&cursor, &out_mat->cols)) != ERROR_OK) return err;

    if (out_mat->rows <= 0 || out_mat->rows > MAX_MATRIX_ROWS ||
        out_mat->cols <= 0 || out_mat->cols > MAX_MATRIX_COLS) {
        return ERROR_INVALID_MATRIX_DIMENSIONS;
    }

    total_cells = out_mat->rows * out_mat->cols;
    cursor = skip_leading_whitespace(cursor);

    /* if no initializers, zero-fill the matrix */
    if (*cursor == '\0') {
        for (i = 0; i < total_cells; ++i) out_mat->cells[i] = 0;
        return ERROR_OK;
    }

    /* parse cell initializers */
    for (i = 0; i < total_cells; ++i) {
        token = next_token(&cursor, ",");
        if (!token || !*token) return ERROR_INVALID_MATRIX_INITIALIZATION;

        if ((err = parse_integer(token, &val)) != ERROR_OK) return err;
        out_mat->cells[i] = val;
    }

    /* check for extra initializers or trailing comma */
    if (next_token(&cursor, ",")) return ERROR_INVALID_MATRIX_INITIALIZATION;

    return ERROR_OK;
}

/* Validates the addressing modes of operands for a given instruction.
 * ERROR_OK if the modes are valid, otherwise ERROR_INVALID_ADDRESSING_MODE.
 */
static error_code_t validate_addressing_modes(const parsed_line *p) {
    op_code_t opcode = p->body.operation.opcode;

    /* for single-operand instructions, the operand is in source_op. */
    const operand_t *single_op = &p->body.operation.source_op;

    switch (opcode) {
        /* two Operand Instructions */
        case MOV_OP:
        case CMP_OP:
        case ADD_OP:
        case SUB_OP:
            /* source Can be any of the 4 modes. No check needed. */
            /* destination Cannot be immediate */
            if (p->body.operation.dest_op.mode == IMMEDIATE)
                return ERROR_INVALID_ADDRESSING_MODE;
            break;

        case LEA_OP:
            /* source must be DIRECT or MATRIX_ACCESS */
            if (p->body.operation.source_op.mode != DIRECT && p->body.operation.source_op.mode != MATRIX_ACCESS)
                return ERROR_INVALID_ADDRESSING_MODE;
            /* destination cannot be immediate */
            if (p->body.operation.dest_op.mode == IMMEDIATE) /* destination Cannot be immediate */
                return ERROR_INVALID_ADDRESSING_MODE;
            break;

        /* single operand instructions */
        case CLR_OP:
        case NOT_OP:
        case INC_OP:
        case DEC_OP:
        case JMP_OP:
        case BNE_OP:
        case JSR_OP:
        case RED_OP:
            /* operand is treated as a destination and cannot be immediate */
            if (single_op->mode == IMMEDIATE) return ERROR_INVALID_ADDRESSING_MODE;
            break;

        /* operand can be anything, no validation needed. */
        case PRN_OP:
        case RTS_OP:
        case STOP_OP:
            break;

        default:
            return ERROR_INVALID_ADDRESSING_MODE;
    }
    return ERROR_OK;
}

error_code_t parse_line(char *line, parsed_line *out) {
    char working_copy[MAX_LINE_LENGTH] = {0}; /* working copy of the line */
    char *cursor, *token;
    int  required_operands = 0;
    int token_len;
    error_code_t error;

    if (!line || !out) return ERROR_INVALID_ARGUMENT;
    if (strlen(line) >= MAX_LINE_LENGTH) return ERROR_LINE_TOO_LONG;

    memset(out, 0, sizeof(*out));
    strncpy(working_copy, line, sizeof(working_copy) - 1);
    working_copy[sizeof(working_copy) - 1] = '\0';


    remove_comment_from_semicolon(working_copy);
    trim_trailing_whitespace_inplace(working_copy);
    cursor = skip_leading_whitespace(working_copy);
    out->kind = LINE_EMPTY_OR_COMMENT;

    if (!*cursor) return ERROR_OK; /* empty line or comment */

    /* first token: maybe label */
    token = next_token(&cursor, " \t\n\r");
    token_len = (int) strlen(token);

    /* optional label */
    if (token[token_len - 1] == ':' && token_len > 0) {
        token[token_len - 1] = '\0'; /* remove the colon */
        if (!is_valid_label(token)) return ERROR_ILLEGAL_LABEL;
        strcpy(out->label, token);
        token = next_token(&cursor, " \t\n\r"); /* get next token (command) */
    }

    if (!token) {
        /* label only line */
        return out->label[0] ? ERROR_INVALID_OPERAND_SYNTAX : ERROR_OK;
    }

    /* directive check */
    if (token[0] == '.') {
        out->kind = LINE_DIRECTIVE;
        out->body.directive.type = lookup_directive_by_token(token);
        if ((int) out->body.directive.type < 0) return ERROR_INVALID_DIRECTIVE;

        switch (out->body.directive.type) {
            case DATA_DIRECTIVE:
                return parse_data_directive_payload(cursor, &out->body.directive.operands.data);
            case STRING_DIRECTIVE:
                return parse_string_directive_payload(cursor, out->body.directive.operands.string_val);
            case MATRIX_DIRECTIVE:
                return parse_matrix_directive_payload(cursor, &out->body.directive.operands.mat);
            case ENTRY_DIRECTIVE:
            case EXTERN_DIRECTIVE: {
                token = next_token(&cursor, " \t\n\r");
                if (!token || !is_valid_label(token)) return ERROR_INVALID_LABEL;
                strcpy(out->body.directive.operands.symbol_name, token);
                if (next_token(&cursor, " \t\n\r")) return ERROR_TRAILING_CHARACTERS;
                return ERROR_OK;
            }
        }
    }

    /* must be instruction */
    out->kind = LINE_OPERATION;
    out->body.operation.opcode = lookup_opcode_by_mnemonic(token, &required_operands);
    if (out->body.operation.opcode == UNKNOWN_OP) return ERROR_UNKNOWN_COMMAND_NAME;
    out->body.operation.n_operands = 0;

    if (required_operands > 0) {
        token = next_token(&cursor, ",");
        if ((error = parse_operand(token, &out->body.operation.source_op)) != ERROR_OK) return error;
        out->body.operation.n_operands = 1;
    }

    if (required_operands > 1) {
        token = next_token(&cursor, ",");
        if ((error = parse_operand(token, &out->body.operation.dest_op)) != ERROR_OK) return error;
        out->body.operation.n_operands = 2;
    }

    /* check for extraneous characters/operands */
    if (*skip_leading_whitespace(cursor) != '\0') {
        return (required_operands == 2 && strchr(cursor, ',')) ? ERROR_TOO_MANY_OPERANDS : ERROR_TRAILING_CHARACTERS;
    }

    if (out->body.operation.n_operands != required_operands) {
        return ERROR_INVALID_OPERAND_COUNT_FOR_COMMAND;
    }

    /* validate operand addressing modes for the parsed instruction */
    if ((error = validate_addressing_modes(out)) != ERROR_OK) {
        return error;
    }

    return ERROR_OK;
}
