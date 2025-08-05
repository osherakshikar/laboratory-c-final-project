#ifndef LINE_PARSER_H
#define LINE_PARSER_H

/*
 * =====================================================================================
 *
 * Filename:  line_parser.h
 *
 * Description: Defines the data structures representing a single parsed line of assembly
 * code (the Abstract Syntax Tree node). It also declares the main
 * public function for parsing a line of text into these structures.
 *
 * =====================================================================================
 */

/*-----------------------------------------------------------------------------
 * ENUMERATIONS
 *-----------------------------------------------------------------------------*/

typedef enum {
    MOV_OP, CMP_OP, ADD_OP, SUB_OP, LEA_OP,
    NOT_OP, CLR_OP, INC_OP, DEC_OP, JMP_OP, BNE_OP, RED_OP, PRN_OP, JSR_OP,
    RTS_OP, STOP_OP,
    UNKNOWN_OP
} Opcode;

typedef enum {
    IMMEDIATE = 0,
    DIRECT = 1,
    MATRIX_ACCESS = 2,
    REGISTER_DIRECT = 3
} AddressingMode;

typedef enum {
    DATA_DIRECTIVE,
    STRING_DIRECTIVE,
    MATRIX_DIRECTIVE,
    ENTRY_DIRECTIVE,
    EXTERN_DIRECTIVE,
    UNKNOWN_DIRECTIVE
} DirectiveType;

/*-----------------------------------------------------------------------------
 * CORE DATA STRUCTURES
 *-----------------------------------------------------------------------------*/

typedef struct {
    AddressingMode mode;
    union {
        int immediate_value;
        char* label;
        int register_num;
    } value;
    int matrix_row_reg;
    int matrix_col_reg;
} Operand;

typedef struct {
    Opcode opcode;
    Operand* source_operand;
    Operand* dest_operand;
} InstructionNode;

typedef struct {
    DirectiveType type;
    union {
        struct {
            int* numbers;
            int count;
        } data;
        char* str;
        struct {
            int rows;
            int cols;
            int* initial_values;
            int value_count;
        } mat;
        char* label;
    } params;
} DirectiveNode;

typedef enum {
    INSTRUCTION_STATEMENT,
    DIRECTIVE_STATEMENT,
    COMMENT_OR_EMPTY_STATEMENT
} StatementType;

typedef struct StatementNode {
    char* label;
    int line_number;
    StatementType type;
    union {
        InstructionNode* instruction;
        DirectiveNode* directive;
    } content;
    struct StatementNode* next; /* To link statements into a full program AST */
} StatementNode;


/*-----------------------------------------------------------------------------
 * PUBLIC FUNCTION PROTOTYPE
 *-----------------------------------------------------------------------------*/

/**
 * @brief Parses a single line of assembly code into a StatementNode structure.
 *
 * This is the main function of the parser module. It analyzes the syntax,
 * validates it, and builds the corresponding data structure.
 *
 * @param line The string containing the line of code to parse.
 * @param line_number The original line number from the source file, for error reporting.
 * @return A pointer to a newly allocated StatementNode if parsing is successful,
 * or NULL if a syntax error is found.
 */
StatementNode* parse_line(char* line, int line_number);

#endif /* LINE_PARSER_H */
