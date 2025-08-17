#ifndef SECOND_PASS_H
#define SECOND_PASS_H
#include "globals.h"
#include "util_vec.h"

/*
 * =====================================================================================
 * Filename:  second_pass.h
 * Description: Header file for the second pass of the assembler.
 * This pass generates machine code, creates object files, and handles external symbols.
 * It encodes instructions, resolves symbols, and writes the output files.
 * =====================================================================================
 */


/* pack the first word: [opcode 9..6][src 5..4][dst 3..2][ARE 1..0] */
#define FIRST_WORD(op, src_mode, dst_mode, are) \
(((op) << 6) | ((src_mode) << 4) | ((dst_mode) << 2) | (are))

#define ARE_A 0 /* absolute 00 */
#define ARE_E 1 /* external 01 */
#define ARE_R 2 /* relocate 10 */

/* set low 2 bits (ARE) of a word */
#define WORD_SET_ARE(w, are) do { (w) = (WORD)(((w) & ~0x0003) | ((are) & 0x3)); } while(0)

typedef unsigned short WORD;/* store 10-bit words in 16 bits */

/* struct ext_usage_t defines an external symbol usage
 * It contains the name of the external symbol and its absolute address in the code image.
 * This is used to track where external symbols are referenced in the code.
 */
typedef struct ext_usage {
    char name[MAX_LABEL_LENGTH];
    int  address; /* absolute address of the label word */
} ext_usage_t;

/* struct second_pass_ctx_t defines the context for the second pass of the assembler.
 * It contains the code and data images, their current positions, and a vector list of external symbols.
 * The code image is used to store machine code instructions, while the data image stores data directives.
 */
typedef struct {
    WORD code_image[IMAGE_LENGTH];
    WORD data_image[IMAGE_LENGTH];
    int  code_pos;
    int  data_pos;
    vec_t ext_list; /* vector list of all external symbols*/
} second_pass_ctx_t;

/**
 * @brief Performs the second pass of the assembler
 *
 * Generates machine code, creates .ob file with base-4 encoded instructions,
 * creates .ent file for entry symbols, and creates .ext file for external symbols
 *
 * @param input_path Path to the preprocessed assembly file
 * @param file_name Base name for output files
 * @param symtab Symbol table from first pass
 * @return 0 on success, -1 on failure
 */
int second_pass(const char *input_path, const char *file_name, symbol_table_t *symtab);

#endif
