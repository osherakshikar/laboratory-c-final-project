#ifndef LINE_PARSER_H
#define LINE_PARSER_H
#include "globals.h"
#include "errors.h"

/*
 * =====================================================================================
 * Filename:  line_parser.h
 * Description: Defines the data structures representing a single parsed line of assembly
 * code.
 * =====================================================================================
 */

/* enum op_code_t defines the operation codes for the assembly instructions.
 * Each operation code corresponds to a specific instruction in the assembly language.
 */
typedef enum {
    MOV_OP, CMP_OP, ADD_OP, SUB_OP, LEA_OP, CLR_OP, NOT_OP, INC_OP,
    DEC_OP, JMP_OP, BNE_OP, JSR_OP, RED_OP, PRN_OP, RTS_OP, STOP_OP,
    UNKNOWN_OP = -1
} op_code_t;

/* enum addressing_mode_t defines the different addressing modes
 * that can be used for operands in assembly instructions.
 * Each mode specifies how the operand is accessed or interpreted.
 */
typedef enum {
    IMMEDIATE = 0, /* #number */
    DIRECT = 1, /* LABEL */
    MATRIX_ACCESS = 2, /* LABEL[rX][rY] */
    REGISTER_DIRECT = 3 /* r0..r7 */
} addressing_mode_t;

/* enum directive_t defines the different types of directives
 * that can appear in assembly code. Each directive specifies a specific
 * operation or data definition in the assembly language.
 */
typedef enum {
    DATA_DIRECTIVE,
    STRING_DIRECTIVE,
    MATRIX_DIRECTIVE,
    ENTRY_DIRECTIVE,
    EXTERN_DIRECTIVE
} directive_t;

/* enum line_kind_t defines the kind of line being parsed.
 * It can be an empty line, a comment, a directive, or an operation.
 * This helps categorize the parsed line for further processing.
 */
typedef enum {
    LINE_EMPTY_OR_COMMENT, /* blank or whitespace only or starts with '; */
    LINE_DIRECTIVE, /* .data, .string, .mat, .entry, .extern  */
    LINE_OPERATION /* valid instruction opcode                */
} line_kind_t;

/* struct int_array_t defines an array of integers
 * with a maximum size defined by MAX_DATA_ITEMS.
 * It is used to store data for the .data directive.
 */
typedef struct {
    int values[MAX_DATA_ITEMS];
    int count; /* 0..MAX_DATA_ITEMS */
} int_array_t;

/* struct matrix_def_t defines a matrix with a maximum number of rows and columns.
 * It contains an array of integers representing the cells of the matrix.
 */
typedef struct {
    int rows; /* 1..MAX_MATRIX_ROWS  */
    int cols; /* 1..MAX_MATRIX_COLS  */
    int cells[MAX_MATRIX_CELLS]; /*  */
} matrix_def_t;

/* struct operand_t defines an operand in an assembly instruction.
 * It contains the addressing mode, the value (immediate, register, or label),
 * and optional registers for matrix access.
 */
typedef struct {
    addressing_mode_t mode; /* addressing method */
    union {
        int immediate_value; /* valid when mode == IMMEDIATE */
        int reg_num; /* 0‑7, when mode == REGISTER_DIRECT */
        char label[MAX_LABEL_LENGTH]; /* direct/matrix base label */
    } value;
    int row_reg; /* register for matrix row, when mode == MATRIX_ACCESS */
    int col_reg; /* register for matrix column, when mode == MATRIX_ACCESS */
} operand_t;

/* struct parsed_line defines a single parsed line of assembly code.
 * It contains the kind of line, an optional label, and the body of the line,
 * which can be either a directive or an operation.
 */
typedef struct {
    line_kind_t kind; /* what was detected */
    char label[MAX_LABEL_LENGTH]; /* label name if any */

    union {
        struct {
            directive_t type; /* .data / .string / .mat / .entry / .extern */
            union {
                int_array_t data; /* for .data directive */
                char string_val[MAX_STRING_LEN]; /* for .string directive */
                matrix_def_t mat; /* for .mat directive */
                char symbol_name[MAX_LABEL_LENGTH]; /* for .entry and .extern directives */
            } operands; /* text of the operands */
        } directive;

        struct {
            op_code_t opcode; /* instruction opcode */
            int n_operands; /* number of operands, 0..2 */
            operand_t source_op;
            operand_t dest_op; /* may be empty if not used */
        } operation;
    } body; /* body of the line */
} parsed_line;

/**
 * Parses a single line of assembly code.
 * The line is expected to be null-terminated.
 * The function fills the out parameter with the parsed data.
 * Returns an error code indicating success or failure.
 *
 * @param line Pointer to the line to parse
 * @param out Pointer to the parsed_line structure to fill
 * @return error_code_t indicating the result of the parsing operation
 */
error_code_t parse_line(char *line, parsed_line *out);
#endif
