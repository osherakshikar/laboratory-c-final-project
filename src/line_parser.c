#include "../include/line_parser.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ========= whitespace & comment helpers ========= */

static void remove_comment_from_semicolon(char *line) {
    char *semicolon = strchr(line, ';');
    if (semicolon) *semicolon = '\0';
}

static char *skip_leading_whitespace(char *p) {
    while (*p && strchr(" \t\r\n", *p)) p++;
    return p;
}

static void trim_trailing_whitespace_inplace(char *p) {
    char *end = p + strlen(p);
    while (end > p && strchr(" \t\r\n", end[-1])) end--;
    *end = '\0';
}

/* ========= identifier & small utilities ========= */


/* Return next token delimited by whitespace; advances *cursor. */
static char *next_whitespace_delimited_token(char **cursor) {
    char *scan = *cursor;
    char *whitespace_pos;

    scan = skip_leading_whitespace(scan);
    if (!*scan) { *cursor = scan; return NULL; }

    whitespace_pos = strpbrk(scan, " \t\r\n");
    if (whitespace_pos) {
        *whitespace_pos = '\0';
        *cursor = whitespace_pos + 1;
    } else {
        *cursor = scan + strlen(scan);
    }
    return scan;
}

/* Return next comma-separated piece (trimmed); advances *cursor. */
static char *next_comma_separated_piece(char **cursor) {
    char *start, *comma;
    if (!cursor || !*cursor) return NULL;

    start = skip_leading_whitespace(*cursor);
    if (!*start) { *cursor = start; return NULL; }

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

/* ========= registers & operands ========= */

/* r0..r7 => 0..7; -1 not a register; -2 looks like rX but invalid (e.g., r8) */
static int parse_register_token(const char *tok) {
    if (!tok) return -1;
    if (tok[0]=='r' && tok[1]>='0' && tok[1]<='7' && tok[2]=='\0') return tok[1]-'0';
    if (tok[0]=='r' && isdigit((unsigned char)tok[1]) && tok[2]=='\0') return -2;
    return -1;
}

/* "#-?\d+" → immediate. Returns 1 on success, 0 not immediate, negative on format error. */
static int parse_immediate_token(const char *tok, int *out_value) {
    char *endptr;
    long val;
    if (!tok || tok[0] != '#') return 0;
    val = strtol(tok + 1, &endptr, 10);
    if (*endptr != '\0') return -ERROR_INVALID_NUMBER_FORMAT;
    if (out_value) *out_value = (int)val;
    return 1;
}

/* LABEL[rX][rY] (compact, no spaces). Returns 1 ok, 0 not matrix, negative on error. */
static int parse_matrix_access_token(const char *tok, operand_t *out_op) {
    const char *br1, *br1_close, *br2, *br2_close;
    size_t base_len;
    int row_reg, col_reg;

    br1 = strchr(tok, '[');
    if (!br1) return 0;

    base_len = (size_t)(br1 - tok);
    if (base_len == 0 || base_len >= (size_t)MAX_LABEL_LENGTH) return -ERROR_INVALID_LABEL;

    /* copy base label */
    memcpy(out_op->value.label, tok, base_len);
    out_op->value.label[base_len] = '\0';

    br1_close = strchr(br1 + 1, ']');
    if (!br1_close) return -ERROR_INVALID_ADDRESSING_MODE;

    br2 = strchr(br1_close + 1, '[');
    if (!br2) return -ERROR_INVALID_ADDRESSING_MODE;

    br2_close = strchr(br2 + 1, ']');
    if (!br2_close || br2_close[1] != '\0') return -ERROR_INVALID_ADDRESSING_MODE;

    /* enforce [rX] [rY] */
    if ((br1[1] != 'r') || !isdigit((unsigned char)br1[2]) || br1[3] != ']') return -ERROR_INVALID_REGISTER;
    if ((br2[1] != 'r') || !isdigit((unsigned char)br2[2]) || br2[3] != ']') return -ERROR_INVALID_REGISTER;

    row_reg = br1[2] - '0';
    col_reg = br2[2] - '0';
    if (row_reg < 0 || row_reg > 7 || col_reg < 0 || col_reg > 7) return -ERROR_INVALID_REGISTER;

    out_op->mode    = MATRIX_ACCESS;
    out_op->row_reg = row_reg;
    out_op->col_reg = col_reg;
    return 1;
}

/* Parse any single operand token into operand_t. */
static error_code_t parse_operand_token(const char *tok, operand_t *out_op) {
    int imm_status, imm_value, reg;
    int matrix_status;

    memset(out_op, 0, sizeof(*out_op));
    out_op->mode = DIRECT; /* default */

    /* immediate */
    imm_status = parse_immediate_token(tok, &imm_value);
    if (imm_status < 0) return (error_code_t)(-imm_status);
    if (imm_status == 1) {
        out_op->mode = IMMEDIATE;
        out_op->value.immediate_value = imm_value;
        return ERROR_OK;
    }

    /* register */
    reg = parse_register_token(tok);
    if (reg >= 0) { out_op->mode = REGISTER_DIRECT; out_op->value.reg_num = reg; return ERROR_OK; }
    if (reg == -2) return ERROR_INVALID_REGISTER;

    /* matrix access */
    matrix_status = parse_matrix_access_token(tok, out_op);
    if (matrix_status < 0) return (error_code_t)(-matrix_status);
    if (matrix_status == 1) return ERROR_OK;

    /* plain label */
    if (!isalpha(tok[0])) return ERROR_INVALID_OPERAND_SYNTAX;
    {
        size_t i, n = strlen(tok);
        if (n >= (size_t)MAX_LABEL_LENGTH) return ERROR_INVALID_LABEL;
        for (i = 1; i < n; ++i) if (!isalnum(tok[i])) return ERROR_INVALID_LABEL;
        memcpy(out_op->value.label, tok, n + 1);
        out_op->mode = DIRECT;
    }
    return ERROR_OK;
}

/* ========= opcode/directive lookup ========= */

typedef struct { const char *mnemonic; op_code_t opcode; int required_operands; } opcode_desc_t;
static const opcode_desc_t OPCODES[] = {
    { "mov", MOV_OP, 2 }, { "cmp", CMP_OP, 2 }, { "add", ADD_OP, 2 }, { "sub", SUB_OP, 2 },
    { "lea", LEA_OP, 2 }, { "clr", CLR_OP, 1 }, { "not", NOT_OP, 1 }, { "inc", INC_OP, 1 },
    { "dec", DEC_OP, 1 }, { "jmp", JMP_OP, 1 }, { "bne", BNE_OP, 1 }, { "jsr", JSR_OP, 1 },
    { "red", RED_OP, 1 }, { "prn", PRN_OP, 1 }, { "rts", RTS_OP, 0 }, { "stop", STOP_OP, 0 },
    { 0, UNKNOWN_OP, 0 }
};

static op_code_t lookup_opcode_by_mnemonic(const char *tok, int *out_required) {
    int i;
    for (i = 0; OPCODES[i].mnemonic; ++i) {
        if (strcmp(OPCODES[i].mnemonic, tok) == 0) {
            if (out_required) *out_required = OPCODES[i].required_operands;
            return OPCODES[i].opcode;
        }
    }
    return UNKNOWN_OP;
}

static directive_t lookup_directive_by_token(const char *tok) {
    if (strcmp(tok, ".data")   == 0) return DATA_DIRECTIVE;
    if (strcmp(tok, ".string") == 0) return STRING_DIRECTIVE;
    if (strcmp(tok, ".mat")    == 0) return MATRIX_DIRECTIVE;
    if (strcmp(tok, ".entry")  == 0) return ENTRY_DIRECTIVE;
    if (strcmp(tok, ".extern") == 0) return EXTERN_DIRECTIVE;
    return (directive_t)-1;
}

/* ========= directive parsers ========= */

static error_code_t parse_data_directive_payload(char *payload, int_array_t *out_values) {
    char *cursor = payload, *field;
    out_values->count = 0;

    if (!payload) return ERROR_EXPECTED_OPERAND;

    field = next_comma_separated_piece(&cursor);
    if (!field || !*field) return ERROR_EXPECTED_OPERAND;

    while (field) {
        char *endptr;
        long val;

        if (!*field) return ERROR_EXTRANEOUS_COMMA;
        val = strtol(field, &endptr, 10);
        if (*endptr != '\0') return ERROR_INVALID_NUMBER_FORMAT;
        if (out_values->count >= MAX_DATA_ITEMS) return ERROR_INVALID_MATRIX_INITIALIZATION; /* cap overflow */
        out_values->values[out_values->count++] = (int)val;

        field = next_comma_separated_piece(&cursor);
        if (field && !*field) return ERROR_EXTRANEOUS_COMMA;
    }
    return ERROR_OK;
}

static error_code_t parse_string_directive_payload(char *payload, char dest[MAX_STRING_LEN]) {
    char *open_quote, *close_quote;
    size_t len;

    if (!payload) return ERROR_INVALID_STRING_FORMAT;
    payload = skip_leading_whitespace(payload);

    open_quote  = strchr(payload, '\"');
    if (!open_quote) return ERROR_INVALID_STRING_FORMAT;
    close_quote = strrchr(open_quote + 1, '\"');
    if (!close_quote || close_quote <= open_quote + 1) return ERROR_INVALID_STRING_FORMAT;

    len = (size_t)(close_quote - (open_quote + 1));
    if (len >= (size_t)MAX_STRING_LEN) return ERROR_INVALID_STRING_FORMAT;

    memcpy(dest, open_quote + 1, len);
    dest[len] = '\0';

    /* reject trailing garbage */
    close_quote++;
    close_quote = skip_leading_whitespace(close_quote);
    if (*close_quote) return ERROR_TRAILING_CHARACTERS;

    return ERROR_OK;
}

static error_code_t parse_matrix_directive_payload(char *payload, matrix_def_t *out_mat) {
    char *cursor = payload;
    char *rows_text, *cols_text, *cell_text;
    char *endptr;
    int rows, cols, i, expected_cells;

    if (!payload) return ERROR_INVALID_MATRIX_DIMENSIONS;

    rows_text = next_comma_separated_piece(&cursor);
    cols_text = next_comma_separated_piece(&cursor);
    if (!rows_text || !cols_text || !*rows_text || !*cols_text) return ERROR_INVALID_MATRIX_DIMENSIONS;

    rows = (int)strtol(rows_text, &endptr, 10);
    if (*endptr!='\0' || rows<=0 || rows>MAX_MATRIX_ROWS) return ERROR_INVALID_MATRIX_DIMENSIONS;

    cols = (int)strtol(cols_text, &endptr, 10);
    if (*endptr!='\0' || cols<=0 || cols>MAX_MATRIX_COLS) return ERROR_INVALID_MATRIX_DIMENSIONS;

    expected_cells = rows * cols;
    if (expected_cells > MAX_MATRIX_CELLS) return ERROR_INVALID_MATRIX_INITIALIZATION;

    out_mat->rows = rows;
    out_mat->cols = cols;

    for (i = 0; i < expected_cells; ++i) {
        long v;
        cell_text = next_comma_separated_piece(&cursor);
        if (!cell_text || !*cell_text) return ERROR_INVALID_MATRIX_INITIALIZATION;
        v = strtol(cell_text, &endptr, 10);
        if (*endptr != '\0') return ERROR_INVALID_NUMBER_FORMAT;
        out_mat->cells[i] = (int)v;
    }

    /* ensure no extra values */
    if (next_comma_separated_piece(&cursor)) return ERROR_INVALID_MATRIX_INITIALIZATION;

    return ERROR_OK;
}

/* ========= top-level line parser ========= */

error_code_t parse_line(char *line, parsed_line *out) {
    char working_copy[MAX_LINE_LENGTH];
    char *cursor, *token, *operands_text;
    size_t src_len;
    int i, required_operands;

    if (!line || !out) return ERROR_INVALID_OPERAND_SYNTAX;

    memset(out, 0, sizeof(*out));
    out->kind = LINE_EMPTY_OR_COMMENT;

    strncpy(working_copy, line, sizeof(working_copy) - 1);
    working_copy[sizeof(working_copy) - 1] = '\0';

    remove_comment_from_semicolon(working_copy);
    trim_trailing_whitespace_inplace(working_copy);
    cursor = skip_leading_whitespace(working_copy);
    if (!*cursor) return ERROR_OK;

    /* first token: label or directive/opcode */
    token = next_whitespace_delimited_token(&cursor);
    if (!token) return ERROR_OK;

    /* optional label */
    if (token[strlen(token) - 1] == ':') {
        size_t label_len = strlen(token) - 1;
        if (label_len == 0 || label_len >= (size_t)MAX_LABEL_LENGTH) return ERROR_INVALID_LABEL;
        if (!isalpha(token[0])) return ERROR_INVALID_LABEL;
        for (i = 1; i < (int)label_len; ++i) if (!isalnum(token[i])) return ERROR_INVALID_LABEL;
        memcpy(out->label, token, label_len);
        out->label[label_len] = '\0';

        token = next_whitespace_delimited_token(&cursor);
        if (!token) return ERROR_INVALID_OPERAND_SYNTAX; /* label with no body */
    }

    /* directive? */
    if (token[0] == '.') {
        directive_t kind = lookup_directive_by_token(token);
        char *payload;

        if ((int)kind < 0) return ERROR_INVALID_OPERAND_SYNTAX;

        out->kind = LINE_DIRECTIVE;
        out->body.directive.type = kind;

        payload = skip_leading_whitespace(cursor);

        switch (kind) {
            case DATA_DIRECTIVE:
                return parse_data_directive_payload(payload, &out->body.directive.operands.data);
            case STRING_DIRECTIVE:
                return parse_string_directive_payload(payload, out->body.directive.operands.string_val);
            case MATRIX_DIRECTIVE:
                return parse_matrix_directive_payload(payload, &out->body.directive.operands.mat);
            case ENTRY_DIRECTIVE:
            case EXTERN_DIRECTIVE: {
                char *symbol = next_whitespace_delimited_token(&payload);
                size_t cap = sizeof(out->body.directive.operands.symbol_name);
                if (!symbol) return ERROR_EXPECTED_OPERAND;
                if (!isalpha(symbol[0])) return ERROR_INVALID_LABEL;
                for (i = 1; symbol[i]; ++i) if (!isalnum(symbol[i])) return ERROR_INVALID_LABEL;
                if (strlen(symbol) >= cap) return ERROR_INVALID_LABEL;
                memcpy(out->body.directive.operands.symbol_name, symbol, strlen(symbol) + 1);
                if (next_whitespace_delimited_token(&payload)) return ERROR_TRAILING_CHARACTERS;
                return ERROR_OK;
            }
            default: break;
        }
        return ERROR_INVALID_OPERAND_SYNTAX; /* should not happen */
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

    /* quick comma sanity checks */
    if (operands_text[0] == ',' || operands_text[strlen(operands_text) - 1] == ',') return ERROR_EXTRANEOUS_COMMA;
    if (strstr(operands_text, ",,")) return ERROR_EXTRANEOUS_COMMA;

    /* parse up to 2 comma-separated operands */
    {
        char *list_cursor = operands_text;
        char *field;
        int parsed = 0;

        field = next_comma_separated_piece(&list_cursor);
        if (!field || !*field) return ERROR_EXPECTED_OPERAND;
        {
            error_code_t ec = parse_operand_token(field, &out->body.operation.source_op);
            if (ec != ERROR_OK) return ec;
            parsed = 1;
        }

        field = next_comma_separated_piece(&list_cursor);
        if (field && *field) {
            error_code_t ec = parse_operand_token(field, &out->body.operation.dest_op);
            if (ec != ERROR_OK) return ec;
            parsed = 2;

            /* ensure no third operand */
            if (next_comma_separated_piece(&list_cursor)) return ERROR_TOO_MANY_OPERANDS;
        } else if (field && !*field) {
            return ERROR_EXTRANEOUS_COMMA;
        }

        if (parsed != required_operands) return ERROR_INVALID_OPERAND_COUNT_FOR_COMMAND;
        out->body.operation.n_operands = parsed;
    }

    return ERROR_OK;
}