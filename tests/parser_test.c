#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "../include/globals.h"
#include "../include/errors.h"
#include "../include/line_parser.h"

static int test_count = 0;
static int passed_tests = 0;

#define RUN_TEST(test_func) do { \
    test_count++; \
    printf("Running %s... ", #test_func); \
    if (test_func()) { \
        printf("PASSED\n"); \
        passed_tests++; \
    } else { \
        printf("FAILED\n"); \
    } \
} while(0)

static int test_empty_lines(void) {
    parsed_line pl;
    error_code_t err;
    char line1[] = "";
    char line2[] = "   \t  ";
    char line3[] = "; this is a comment";

    /* Empty line */
    err = parse_line(line1, &pl);
    if (err != ERROR_OK || pl.kind != LINE_EMPTY_OR_COMMENT) return 0;

    /* Whitespace only */
    err = parse_line(line2, &pl);
    if (err != ERROR_OK || pl.kind != LINE_EMPTY_OR_COMMENT) return 0;

    /* Comment only */
    err = parse_line(line3, &pl);
    if (err != ERROR_OK || pl.kind != LINE_EMPTY_OR_COMMENT) return 0;

    return 1;
}

static int test_labels(void) {
    parsed_line pl;
    error_code_t err;
    char line1[] = "mylabel: mov r1, r2";
    char line2[] = "1label: mov r1, r2";
    char line3[] = "verylonglabelnamethatisinvalida: mov r1, r2";
    char line4[] = ": mov r1, r2";

    /* Valid label with instruction */
    err = parse_line(line1, &pl);
    if (err != ERROR_OK || strcmp(pl.label, "mylabel") != 0) return 0;
    if (pl.kind != LINE_OPERATION) return 0;

    /* Invalid label (starts with digit) */
    err = parse_line(line2, &pl);
    if (err != ERROR_ILLEGAL_LABEL) return 0;

    /* Invalid label (too long) */
    err = parse_line(line3, &pl);
    if (err != ERROR_INVALID_LABEL) return 0;

    /* Empty label */
    err = parse_line(line4, &pl);
    if (err != ERROR_INVALID_LABEL) return 0;

    return 1;
}

static int test_instructions(void) {
    parsed_line pl;
    error_code_t err;
    char line1[] = "mov r1, r2";
    char line2[] = "clr r3";
    char line3[] = "stop";
    char line4[] = "unknown r1, r2";

    /* Two operand instruction */
    err = parse_line(line1, &pl);
    if (err != ERROR_OK || pl.kind != LINE_OPERATION) return 0;
    if (pl.body.operation.opcode != MOV_OP) return 0;
    if (pl.body.operation.n_operands != 2) return 0;

    /* One operand instruction */
    err = parse_line(line2, &pl);
    if (err != ERROR_OK || pl.body.operation.opcode != CLR_OP) return 0;
    if (pl.body.operation.n_operands != 1) return 0;

    /* Zero operand instruction */
    err = parse_line(line3, &pl);
    if (err != ERROR_OK || pl.body.operation.opcode != STOP_OP) return 0;
    if (pl.body.operation.n_operands != 0) return 0;

    /* Unknown instruction */
    err = parse_line(line4, &pl);
    if (err != ERROR_UNKNOWN_COMMAND_NAME) return 0;

    return 1;
}

static int test_operands(void) {
    parsed_line pl;
    error_code_t err;
    char line1[] = "mov r1, r7";
    char line2[] = "mov #42, r1";
    char line3[] = "mov label1, r1";
    char line4[] = "mov r8, r1";

    /* Register operands */
    err = parse_line(line1, &pl);
    if (err != ERROR_OK) return 0;
    if (pl.body.operation.source_op.mode != REGISTER_DIRECT) return 0;
    if (pl.body.operation.source_op.value.reg_num != 1) return 0;
    if (pl.body.operation.dest_op.mode != REGISTER_DIRECT) return 0;
    if (pl.body.operation.dest_op.value.reg_num != 7) return 0;

    /* Immediate operand */
    err = parse_line(line2, &pl);
    if (err != ERROR_OK) return 0;
    if (pl.body.operation.source_op.mode != IMMEDIATE) return 0;
    if (pl.body.operation.source_op.value.immediate_value != 42) return 0;

    /* Label operand */
    err = parse_line(line3, &pl);
    if (err != ERROR_OK) return 0;
    if (pl.body.operation.source_op.mode != DIRECT) return 0;
    if (strcmp(pl.body.operation.source_op.value.label, "label1") != 0) return 0;

    /* Invalid register */
    err = parse_line(line4, &pl);
    if (err != ERROR_INVALID_REGISTER) return 0;

    return 1;
}

static int test_directives(void) {
    parsed_line pl;
    error_code_t err;
    char line1[] = ".data 1, 2, 3";
    char line2[] = ".string \"hello\"";
    char line3[] = ".entry symbol1";
    char line4[] = ".extern symbol2";
    char line5[] = ".invalid";

    /* .data directive */
    err = parse_line(line1, &pl);
    if (err != ERROR_OK || pl.kind != LINE_DIRECTIVE) return 0;
    if (pl.body.directive.type != DATA_DIRECTIVE) return 0;
    if (pl.body.directive.operands.data.count != 3) return 0;
    if (pl.body.directive.operands.data.values[0] != 1) return 0;

    /* .string directive */
    err = parse_line(line2, &pl);
    if (err != ERROR_OK || pl.body.directive.type != STRING_DIRECTIVE) return 0;
    if (strcmp(pl.body.directive.operands.string_val, "hello") != 0) return 0;

    /* .entry directive */
    err = parse_line(line3, &pl);
    if (err != ERROR_OK || pl.body.directive.type != ENTRY_DIRECTIVE) return 0;
    if (strcmp(pl.body.directive.operands.symbol_name, "symbol1") != 0) return 0;

    /* .extern directive */
    err = parse_line(line4, &pl);
    if (err != ERROR_OK || pl.body.directive.type != EXTERN_DIRECTIVE) return 0;
    if (strcmp(pl.body.directive.operands.symbol_name, "symbol2") != 0) return 0;

    /* Invalid directive */
    err = parse_line(line5, &pl);
    if (err != ERROR_INVALID_DIRECTIVE) return 0;

    return 1;
}

static int test_matrix_operations(void) {
    parsed_line pl;
    error_code_t err;
    char line1[] = ".mat [2][3] 1,2,3,4,5,6";
    char line2[] = "mov matrix1[r1][r2], matrix1[r1][r2]";

    /* Matrix definition */
    err = parse_line(line1, &pl);
    if (err != ERROR_OK || pl.body.directive.type != MATRIX_DIRECTIVE) return 0;
    if (pl.body.directive.operands.mat.rows != 2) return 0;
    if (pl.body.directive.operands.mat.cols != 3) return 0;
    if (pl.body.directive.operands.mat.cells[0] != 1) return 0;
    if (pl.body.directive.operands.mat.cells[5] != 6) return 0;

    /* Matrix access */
    err = parse_line(line2, &pl);
    if (err != ERROR_OK) return 0;
    if (pl.body.operation.source_op.mode != MATRIX_ACCESS) return 0;
    if (strcmp(pl.body.operation.source_op.value.label, "matrix1") != 0) return 0;
    if (pl.body.operation.source_op.row_reg != 1) return 0;
    if (pl.body.operation.source_op.col_reg != 2) return 0;

    return 1;
}

static int test_error_cases(void) {
    parsed_line pl;
    error_code_t err;
    char line1[] = "mov r1, r2, r3";
    char line2[] = "mov r1";
    char line3[] = "mov #abc, r1";
    char line4[] = ".string hello";
    char line5[] = "stop extra";

    /* Too many operands */
    err = parse_line(line1, &pl);
    if (err != ERROR_TOO_MANY_OPERANDS) return 0;

    /* Wrong operand count */
    err = parse_line(line2, &pl);
    if (err != ERROR_INVALID_OPERAND_COUNT_FOR_COMMAND) return 0;

    /* Invalid immediate format */
    err = parse_line(line3, &pl);
    if (err != ERROR_INVALID_NUMBER_FORMAT) return 0;

    /* Invalid string format */
    err = parse_line(line4, &pl);
    if (err != ERROR_INVALID_STRING_FORMAT) return 0;

    /* Trailing characters */
    err = parse_line(line5, &pl);
    if (err != ERROR_INVALID_OPERAND_COUNT_FOR_COMMAND) return 0;

    return 1;
}

static int test_whitespace_handling(void) {
    parsed_line pl;
    error_code_t err;
    char line1[] = "  mylabel:   mov   r1,   r2  ";
    char line2[] = "\tmov\tr1,\tr2\t";

    /* Extra whitespace */
    err = parse_line(line1, &pl);
    if (err != ERROR_OK) return 0;
    if (strcmp(pl.label, "mylabel") != 0) return 0;
    if (pl.body.operation.opcode != MOV_OP) return 0;

    /* Tabs and spaces mixed */
    err = parse_line(line2, &pl);
    if (err != ERROR_OK) return 0;
    if (pl.body.operation.opcode != MOV_OP) return 0;

    return 1;
}

static int test_comments(void) {
    parsed_line pl;
    error_code_t err;
    char line1[] = "mov r1, r2 ; this is a comment";
    char line2[] = "label1: ; comment only";

    /* Instruction with comment */
    err = parse_line(line1, &pl);
    if (err != ERROR_OK) return 0;
    if (pl.body.operation.opcode != MOV_OP) return 0;

    /* Label with comment */
    err = parse_line(line2, &pl);
    if (err != ERROR_INVALID_OPERAND_SYNTAX) return 0; /* Label with no body */

    return 1;
}

int main(void) {
    printf("Running Line Parser Tests\n");
    printf("========================\n");

    RUN_TEST(test_empty_lines);
    RUN_TEST(test_labels);
    RUN_TEST(test_instructions);
    RUN_TEST(test_operands);
    RUN_TEST(test_directives);
    RUN_TEST(test_matrix_operations);
    RUN_TEST(test_error_cases);
    RUN_TEST(test_whitespace_handling);
    RUN_TEST(test_comments);

    printf("\nResults: %d/%d tests passed\n", passed_tests, test_count);

    return (passed_tests == test_count) ? 0 : 1;
}