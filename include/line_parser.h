#ifndef LINE_PARSER_H
#define LINE_PARSER_H
#include "globals.h"
#include "errors.h"

/*
 * =====================================================================================
 *
 * Filename:  line_parser.h
 *
 * Description: Defines the data structures representing a single parsed line of assembly
 * code.
 *
 * =====================================================================================
 */

 /* move cmp sub add lea, 2 oparand
  * clr not inc dec jmp bne jsr red prn , 1 operand
  * rts stop, 0 operand */

typedef enum {
    MOV_OP, CMP_OP, ADD_OP, SUB_OP, LEA_OP, CLR_OP, NOT_OP, INC_OP,
    DEC_OP, JMP_OP, BNE_OP, JSR_OP, RED_OP, PRN_OP, RTS_OP, STOP_OP,
    UNKNOWN_OP = -1
} op_code_t;

typedef enum {
    IMMEDIATE = 0, /* #number            */
    DIRECT = 1, /* LABEL              */
    MATRIX_ACCESS = 2, /* LABEL[rX][rY]      */
    REGISTER_DIRECT = 3 /* r0..r7             */
} addressing_mode_t;

typedef enum {
    DATA_DIRECTIVE,
    STRING_DIRECTIVE,
    MATRIX_DIRECTIVE,
    ENTRY_DIRECTIVE,
    EXTERN_DIRECTIVE
} directive_t;

typedef enum {
    LINE_EMPTY_OR_COMMENT, /* blank or whitespace only or starts with '; */
    LINE_DIRECTIVE, /* .data, .string, .mat, .entry, .extern  */
    LINE_OPERATION /* valid instruction opcode                */
} line_kind_t;


typedef struct {
    int values[MAX_DATA_ITEMS];
    int count; /* 0..MAX_DATA_ITEMS */
} int_array_t;

/* .mat directive definition (not addressing) */
typedef struct {
    int rows; /* 1..MAX_MATRIX_ROWS  */
    int cols; /* 1..MAX_MATRIX_COLS  */
    int cells[MAX_MATRIX_CELLS]; /*  */
} matrix_def_t;

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

typedef struct {
    line_kind_t kind; /* what was detected */

    char label[MAX_LABEL_LENGTH]; /* label name, if any */

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

/* returns ERROR_OK on success; otherwise an error code */
error_code_t parse_line(char *line, parsed_line *out);


#endif /* LINE_PARSER_H */
