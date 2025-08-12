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
static void remove_comment_from_semicolon(char *line) {
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

/* Return next whitespace delimited token; advances *cursor to next char.
 * If no token found, returns NULL and cursor points to end of string.
 * The token is null-terminated in place, so the caller can modify it.
 */
static char *next_whitespace_delimited_token(char **cursor) {
    char *scan = *cursor; /* scan points to the current position in the string */
    char *whitespace_pos; /* position of the next whitespace character */

    scan = skip_leading_whitespace(scan);
    if (!*scan) {
        *cursor = scan;
        return NULL;
    }

    whitespace_pos = strpbrk(scan, " \t\n\r");
    if (whitespace_pos) {
        *whitespace_pos = '\0';
        *cursor = whitespace_pos + 1;
    } else {
        *cursor = scan + strlen(scan);
    }
    return scan;
}

/* Return next comm separated piece (trimmed) advances *cursor to next char.
 * If no piece found, returns NULL and cursor points to end of string
 * The piece is null terminated in place, so the caller can modify it.
 */
static char *next_comma_separated_piece(char **cursor) {
    char *start, *comma;
    if (!*cursor) return NULL;

    start = skip_leading_whitespace(*cursor);
    if (!*start) {
        *cursor = start;
        return NULL;
    }

    comma = strchr(start, ',');
    if (comma) {
        *comma = '\0';
        trim_trailing_whitespace_inplace(start);
        *cursor = comma + 1;
    } else {
        trim_trailing_whitespace_inplace(start);
        *cursor = start + strlen(start);
    }
    return start;
}

/* Extract content between brackets [ ].
 * Returns ERROR_OK on success, or an error code on failure.
 * Sets content_start to the start of the content and bracket_end to the end of the closing bracket.
 */
static error_code_t extract_bracket_content(const char *start, char **content_start, char **bracket_end) {
    char *open_bracket, *close_bracket;

    open_bracket = strchr(start, '[');
    if (!open_bracket) return ERROR_INVALID_ADDRESSING_MODE;

    close_bracket = strchr(open_bracket + 1, ']');
    if (!close_bracket) return ERROR_INVALID_ADDRESSING_MODE;

    /* Check if content is non-empty */
    if (close_bracket == open_bracket + 1)
        return ERROR_INVALID_MATRIX_DIMENSIONS;

    *content_start = open_bracket + 1;
    *bracket_end = close_bracket;
    return ERROR_OK;
}

/* -- registers & operands -- */

/* Parse register token (r0..r7 or r8..r9). Returns 0..7 for r0..r7, -2 for r8/r9, -1 if not register. */
static int parse_register_token(const char *tok) {
    if (!tok) return -1;
    if (tok[0] == 'r' && tok[1] >= '0' && tok[1] <= '7' && tok[2] == '\0') return tok[1] - '0';
    if (tok[0] == 'r' && isdigit((unsigned char) tok[1]) && tok[2] == '\0') return -2;
    return -1;
}

/* Parse immediate token . Returns 1 on success, 0 if not immediate, negative on error.
 * Sets out_value to the parsed integer value.
 */
static int parse_immediate_token(const char *tok, int *out_value) {
    char *endptr;
    int val;
    if (!tok || tok[0] != '#') return 0;
    val = (int) strtol(tok + 1, &endptr, 10);
    if (*endptr != '\0') return -ERROR_INVALID_NUMBER_FORMAT;
    *out_value = val;
    return 1;
}

/* Parse matrix access token [base_label][rX][rY].
 * Returns 1 on success, native on error, 0 if not a matrix access.
 * Sets out_op to the parsed operand with mode MATRIX_ACCESS.
 */
static int parse_matrix_access_token(const char *tok, operand_t *out_op) {
    char *content_start, *bracket_end;
    error_code_t error;
    int base_len;
    int row_reg, col_reg;

    error = extract_bracket_content(tok, &content_start, &bracket_end);
    if (error == ERROR_INVALID_ADDRESSING_MODE) return 0;
    if (error != ERROR_OK) return -(int) error; /* error code from extract_bracket */
    if (bracket_end - content_start != 2) return -ERROR_INVALID_REGISTER;


    /* copy base label */
    base_len = (int) (content_start - tok - 1);
    if (base_len == 0 || base_len >= MAX_LABEL_LENGTH) return -ERROR_INVALID_LABEL;
    memcpy(out_op->value.label, tok, base_len);
    out_op->value.label[base_len] = '\0';

    if (content_start[0] != 'r' || !isdigit((unsigned char) content_start[1]) || content_start[2] != ']')
        return -ERROR_INVALID_OPERAND_SYNTAX; /* must be [rX][rY] */
    row_reg = content_start[1] - '0';

    error = extract_bracket_content(bracket_end, &content_start, &bracket_end);
    if (error != ERROR_OK) return error; /* error code from extract_bracket */
    if (bracket_end - content_start != 2) return -ERROR_INVALID_REGISTER;

    if (content_start[0] != 'r' || !isdigit((unsigned char) content_start[1]) || content_start[2] != ']')
        return -ERROR_INVALID_REGISTER; /* must be [rX][rY] */
    col_reg = content_start[1] - '0';

    if (row_reg < 0 || row_reg > 7 || col_reg < 0 || col_reg > 7) return -ERROR_INVALID_REGISTER;

    out_op->mode = MATRIX_ACCESS;
    out_op->row_reg = row_reg;
    out_op->col_reg = col_reg;
    return 1;
}

/* Parse operand token.
 * Returns ERROR_OK on success, or an error code on failure.
 * Sets out_op to the parsed operand.
 */
static error_code_t parse_operand_token(const char *tok, operand_t *out_op) {
    int imm_status, imm_value, reg;
    int matrix_status, i, n;

    out_op->mode = DIRECT; /* default */

    /* immediate */
    imm_status = parse_immediate_token(tok, &imm_value);
    if (imm_status < 0) return ERROR_INVALID_NUMBER_FORMAT;
    if (imm_status == 1) {
        out_op->mode = IMMEDIATE;
        out_op->value.immediate_value = imm_value;
        return ERROR_OK;
    }

    /* register */
    reg = parse_register_token(tok);
    if (reg >= 0) {
        out_op->mode = REGISTER_DIRECT;
        out_op->value.reg_num = reg;
        return ERROR_OK;
    }
    if (reg == -2) return ERROR_INVALID_REGISTER;

    /* matrix access */
    matrix_status = parse_matrix_access_token(tok, out_op);
    if (matrix_status < 0) return (error_code_t) (matrix_status);
    if (matrix_status == 1) return ERROR_OK;

    /* plain label */
    if (!isalpha((unsigned char) tok[0])) return ERROR_INVALID_OPERAND_SYNTAX;
    n = (int) strlen(tok);
    if (n >= MAX_LABEL_LENGTH) return ERROR_INVALID_LABEL;
    for (i = 1; i < n; ++i)
        if (!isalnum((unsigned char) tok[i])) return ERROR_ILLEGAL_LABEL;
    memcpy(out_op->value.label, tok, n + 1);
    out_op->value.label[n] = '\0';
    out_op->mode = DIRECT;
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
    char *cursor = payload, *field, *endptr;
    int val;
    out_values->count = 0;

    if (!payload) return ERROR_EXPECTED_OPERAND;

    field = next_comma_separated_piece(&cursor); /* first field */
    if (!field || !*field) return ERROR_EXPECTED_OPERAND;

    while (field) {
        if (!*field) return ERROR_INVALID_DATA_NAME;
        val = (int) strtol(field, &endptr, 10);
        if (*endptr != '\0') return ERROR_INVALID_NUMBER_FORMAT;
        if (out_values->count >= MAX_DATA_ITEMS) return ERROR_DATA_OVERFLOW; /* cap overflow */
        out_values->values[out_values->count++] = val;
        field = next_comma_separated_piece(&cursor); /* next field */
    }
    return ERROR_OK;
}

/* Parse the payload of a .string directive.
 * Expects a string enclosed in double quotes.
 * Fills dest with the parsed string and returns ERROR_OK on success.
 * Returns an error code on failure.
 */
static error_code_t parse_string_directive_payload(char *payload, char dest[MAX_STRING_LEN]) {
    char *open_quote, *close_quote;
    int len;

    if (!payload) return ERROR_INVALID_STRING_FORMAT;
    payload = skip_leading_whitespace(payload);

    open_quote = strchr(payload, '\"');
    if (!open_quote) return ERROR_INVALID_STRING_FORMAT;
    close_quote = strrchr(open_quote + 1, '\"');
    if (!close_quote || close_quote <= open_quote + 1) return ERROR_INVALID_STRING_FORMAT;

    len = (int) (close_quote - (open_quote + 1)); /* length of the string inside quotes */
    if (len >= MAX_STRING_LEN) return ERROR_INVALID_STRING_FORMAT;

    memcpy(dest, open_quote + 1, len);
    dest[len] = '\0';

    /* reject trailing garbage */
    close_quote++;
    close_quote = skip_leading_whitespace(close_quote);
    if (*close_quote) return ERROR_TRAILING_CHARACTERS;
    return ERROR_OK;
}

/* Parse the payload of a .mat directive.
 * Expects a matrix definition in the format [rows][cols][v1, v2, ...].
 * Fills out_mat with the parsed matrix definition and returns ERROR_OK on success.
 * Returns an error code on failure.
 */
static error_code_t parse_matrix_directive_payload(char *payload, matrix_def_t *out_mat) {
    char *content_start, *bracket_end;
    char *endptr;
    error_code_t error;
    int rows, cols, need, i, val;
    char saved;
    char *cursor, *piece;

    if (!payload) return ERROR_INVALID_MATRIX_DIMENSIONS;
    payload = skip_leading_whitespace(payload);

    /* Parse rows dimension */
    error = extract_bracket_content(payload, &content_start, &bracket_end);
    if (error != ERROR_OK) return error;

    /* rows inside first [ ] */
    {
        saved = *bracket_end;
        *bracket_end = '\0';
        rows = (int) strtol(content_start, &endptr, 10);
        if (*endptr != '\0') {
            *bracket_end = saved;
            return ERROR_INVALID_NUMBER_FORMAT;
        }
        *bracket_end = saved;
    }

    /* Validate rows */
    error = extract_bracket_content(bracket_end, &content_start, &bracket_end);
    if (error != ERROR_OK) return error;

    /* cols inside second [ ] */
    {
        saved = *bracket_end;
        *bracket_end = '\0';
        cols = (int) strtol(content_start, &endptr, 10);
        if (*endptr != '\0') {
            *bracket_end = saved;
            return ERROR_INVALID_NUMBER_FORMAT;
        }
        *bracket_end = saved;
    }

    /* validate dims/range */
    if (rows <= 0 || rows > MAX_MATRIX_ROWS) return ERROR_INVALID_MATRIX_DIMENSIONS;
    if (cols <= 0 || cols > MAX_MATRIX_COLS) return ERROR_INVALID_MATRIX_DIMENSIONS;

    need = rows * cols;
    out_mat->rows = rows;
    out_mat->cols = cols;

    /* after dims  either nothing or a comma list of ints */
    cursor = skip_leading_whitespace(bracket_end + 1);
    if (*cursor == '\0') {
        /* No values , zero-init cells */
        out_mat->cells[0] = 0;
        return ERROR_OK;
    }

    /* parse values: v1, v2, ... (no extra / no missing / no trailing comma) */
    {
        for (i = 0; i < need; ++i) {
            piece = next_comma_separated_piece(&cursor);
            if (!piece || !*piece) return ERROR_INVALID_MATRIX_INITIALIZATION; /* missing/empty */
            val = strtol(piece, &endptr, 10);
            if (*endptr != '\0') return ERROR_INVALID_NUMBER_FORMAT;
            out_mat->cells[i] = val;
        }

        /* ensure no extra numbers and no trailing comma */
        piece = next_comma_separated_piece(&cursor);
        if (piece) {
            if (*piece) return ERROR_INVALID_MATRIX_INITIALIZATION; /* extra value */
            return ERROR_TRAILING_CHARACTERS; /* dangling comma */
        }
    }
    return ERROR_OK;
}


error_code_t parse_line(char *line, parsed_line *out) {
    char working_copy[MAX_LINE_LENGTH];
    char *cursor, *token, *operands_text, *payload;
    char *symbol;
    int label_len;
    int i, required_operands;

    char *list_cursor, *field;
    int parsed = 0;
    error_code_t error;

    if (!line || !out) return ERROR_INVALID_OPERAND_SYNTAX;

    memset(out, 0, sizeof(*out));
    out->kind = LINE_EMPTY_OR_COMMENT;

    strncpy(working_copy, line, sizeof(working_copy) - 1);
    working_copy[sizeof(working_copy) - 1] = '\0';

    remove_comment_from_semicolon(working_copy);
    trim_trailing_whitespace_inplace(working_copy);
    cursor = skip_leading_whitespace(working_copy);
    if (!*cursor) return ERROR_OK; /* empty line or comment */

    /* first token: label or directive/opcode */
    token = next_whitespace_delimited_token(&cursor);
    if (!token) return ERROR_OK;

    /* optional label */
    if (token[strlen(token) - 1] == ':') {
        label_len = (int) strlen(token) - 1; /* remove trailing ':' */
        if (label_len == 0 || label_len >= MAX_LABEL_LENGTH) return ERROR_INVALID_LABEL;
        if (!isalpha((unsigned char) token[0])) return ERROR_ILLEGAL_LABEL;
        for (i = 1; i < label_len; ++i) if (!isalnum((unsigned char) token[i])) return ERROR_ILLEGAL_LABEL;
        memcpy(out->label, token, label_len);
        out->label[label_len] = '\0';

        token = next_whitespace_delimited_token(&cursor);
        if (!token) return ERROR_INVALID_OPERAND_SYNTAX; /* label with no body */
    }

    /* directive check */
    if (token[0] == '.') {
        out->kind = LINE_DIRECTIVE;
        out->body.directive.type = lookup_directive_by_token(token);
        if ((int) out->body.directive.type < 0) return ERROR_INVALID_DIRECTIVE;


        payload = skip_leading_whitespace(cursor);
        switch (out->body.directive.type) {
            case DATA_DIRECTIVE:
                return parse_data_directive_payload(payload, &out->body.directive.operands.data);
            case STRING_DIRECTIVE:
                return parse_string_directive_payload(payload, out->body.directive.operands.string_val);
            case MATRIX_DIRECTIVE:
                return parse_matrix_directive_payload(payload, &out->body.directive.operands.mat);
            case ENTRY_DIRECTIVE:
            case EXTERN_DIRECTIVE: {
                symbol = next_whitespace_delimited_token(&payload);
                label_len = sizeof(out->body.directive.operands.symbol_name);
                if (!symbol) return ERROR_INVALID_LABEL;
                if (!isalpha((unsigned char) symbol[0])) return ERROR_ILLEGAL_LABEL;
                for (i = 1; symbol[i]; ++i)
                    if (!isalnum((unsigned char) symbol[i])) return ERROR_ILLEGAL_LABEL;
                if ((int) strlen(symbol) >= label_len) return ERROR_ILLEGAL_LABEL;
                memcpy(out->body.directive.operands.symbol_name, symbol, strlen(symbol) + 1);
                if (next_whitespace_delimited_token(&payload)) return ERROR_TRAILING_CHARACTERS;
                return ERROR_OK;
            }
            default: break;
        }
    }

    /* instruction */
    out->kind = LINE_OPERATION;
    out->body.operation.opcode = lookup_opcode_by_mnemonic(token, &required_operands);
    if (out->body.operation.opcode == UNKNOWN_OP) return ERROR_UNKNOWN_COMMAND_NAME;

    operands_text = skip_leading_whitespace(cursor);

    if (required_operands == 0) {
        if (*operands_text) return ERROR_INVALID_OPERAND_COUNT_FOR_COMMAND;
        return ERROR_OK;
    }

    if (!*operands_text) return ERROR_EXPECTED_OPERAND;

    /* quick extra commas check */
    if (operands_text[0] == ',' || operands_text[strlen(operands_text) - 1] == ',') return ERROR_TRAILING_CHARACTERS;
    if (strstr(operands_text, ",,")) return ERROR_TRAILING_CHARACTERS;

    /* parse up to 2 comma-separated operands */
    {
        list_cursor = operands_text;
        field = next_comma_separated_piece(&list_cursor);
        if (!field || !*field) return ERROR_EXPECTED_OPERAND;
        error = parse_operand_token(field, &out->body.operation.source_op);
        if (error != ERROR_OK) return error;
        parsed = 1;


        field = next_comma_separated_piece(&list_cursor);
        if (field && *field) {
            error = parse_operand_token(field, &out->body.operation.dest_op);
            if (error != ERROR_OK) return error;
            parsed = 2;

            /* ensure no third operand */
            if (next_comma_separated_piece(&list_cursor)) return ERROR_TOO_MANY_OPERANDS;
        } else if (field && !*field) {
            return ERROR_TRAILING_CHARACTERS;
        }

        if (parsed != required_operands) return ERROR_INVALID_OPERAND_COUNT_FOR_COMMAND;
        out->body.operation.n_operands = parsed;
    }

    return ERROR_OK;
}