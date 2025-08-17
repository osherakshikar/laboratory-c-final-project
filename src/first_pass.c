#include "../include/symbol_table.h"
#include "../include/line_parser.h"
#include "../include/globals.h"
#include <stdio.h>
#include <string.h>

/*
 * =====================================================================================
 * Filename: first_pass.c
 * Description: First pass of the assembler that processes assembly-like files,
 * parses lines, and builds a symbol table. It handles labels, directives, and
 * operations, and calculates instruction and data sizes.
 * This module is responsible for parsing the input file, identifying labels,
 * directives, and operations, and updating the instruction counter (IC) and data counter (DC).
 * It also manages the symbol table, ensuring that labels are defined correctly and
 * that no duplicate labels are created.
 * =====================================================================================
 */

/* Number of extra words contributed by a single operand */
static int get_extra_words_for_operand(const operand_t *op) {
    if (!op) return 0;
    switch (op->mode) {
        case IMMEDIATE: case DIRECT: return 1; /* immediate value word, label/relocation word */
        case MATRIX_ACCESS: return 2; /* label word + register word */
        case REGISTER_DIRECT: return 1; /* single reg word */
        default: return 0;
    }
}

/* Total words for an instruction, including the opcode word */
static int calc_instruction_words(const parsed_line *pl) {
    int words = 1; /* opcode */
    int extra = 0;
    const int n = pl->body.operation.n_operands;
    const operand_t *src = (n >= 1) ? &pl->body.operation.source_op : NULL;
    const operand_t *dst = (n >= 2) ? &pl->body.operation.dest_op : NULL;

    if (src) extra += get_extra_words_for_operand(src);
    if (dst) extra += get_extra_words_for_operand(dst);

    /* if both operands are registers, they share ONE register word */
    if (n == 2 && src && dst && src->mode == REGISTER_DIRECT && dst->mode == REGISTER_DIRECT) {
        extra -= 1; /* remove the double count */
    }
    return words + extra;
}

/* Return the number of words needed for a directive */
static int calc_directive_words(const parsed_line *pl) {
    const matrix_def_t *m;
    switch (pl->body.directive.type) {
        case DATA_DIRECTIVE:
            return pl->body.directive.operands.data.count;
        case STRING_DIRECTIVE:
            /* store string including the terminating '\0' */
            return (int) strlen(pl->body.directive.operands.string_val) + 1;
        case MATRIX_DIRECTIVE: {
            m = &pl->body.directive.operands.mat;
            return m->rows * m->cols;
        }
        case ENTRY_DIRECTIVE:
        case EXTERN_DIRECTIVE:
        default:
            return 0; /* no space in memory image */
    }
}

/* After we know IC_final, push all DATA symbols after code */
static void rebase_data_symbols(symbol_table_t *st, int ic_final) {
    /* Add only IC (not ADDRESS_BASE); data symbols already include ADDRESS_BASE */
    symtab_bump_data_addresses(st, ic_final);
}

/* Public API Functions Implementation */

int first_pass(const char *input_path, symbol_table_t *symtab) {
    FILE *fp;
    char line_buf[MAX_LINE_LENGTH];
    parsed_line pl; /* parsed line to used every iteration */
    int line_no = 0;
    int ic = 0; /* instruction counter for code starts at address_base+0 */
    int dc = 0; /* data counter */
    int errors = 0;
    hash_entry_t *it = NULL;
    symbol_t *symbol = NULL;
    error_code_t st;
    int ok;
    char *name;

    if (!input_path || !symtab) return -1;

    fp = fopen(input_path, "r");
    if (!fp) {
        print_error_file(input_path, ERROR_CANNOT_OPEN_FILE, 0);
        return -1;
    }

    while (fgets(line_buf, sizeof(line_buf), fp)) {
        line_no++;

        memset(&pl, 0, sizeof(pl));
        st = parse_line(line_buf, &pl);
        if (st != ERROR_OK) {
            /* parsing error already categorised */
            print_error_file(input_path, st, line_no);
            errors++;
            continue;
        }

        /* skip empty lines and comments */
        if (pl.kind == LINE_EMPTY_OR_COMMENT) {
            continue;
        }
        /* check if there is a label define it according to the statement kind */
        if (pl.label[0]) {
            if (pl.kind == LINE_OPERATION) {
                /* code label lives at the address of the first word of the instruction */
                if (!symtab_insert(symtab, pl.label, ADDRESS_BASE + ic, SYM_CODE)) {
                    print_error_file(input_path, ERROR_DUPLICATE_LABEL_DEFINITION, line_no);
                    errors++;
                }
            } else if (pl.kind == LINE_DIRECTIVE) {
                switch (pl.body.directive.type) {
                    case DATA_DIRECTIVE:
                    case STRING_DIRECTIVE:
                    case MATRIX_DIRECTIVE:
                        /* insert directive label as data symbol */
                        if (!symtab_insert(symtab, pl.label, ADDRESS_BASE + dc, SYM_DATA)) {
                            print_error_file(input_path, ERROR_DUPLICATE_LABEL_DEFINITION, line_no);
                            errors++;
                        }
                        break;
                    case ENTRY_DIRECTIVE:
                    case EXTERN_DIRECTIVE:
                        /* label before .entry/.extern ignore it */
                        break;
                }
            }
        }

        /* handle the statement body to update ic */
        if (pl.kind == LINE_OPERATION) {
            ic += calc_instruction_words(&pl);
            continue;
        }

        /* directives dc handle */
        switch (pl.body.directive.type) {
            case DATA_DIRECTIVE:
            case STRING_DIRECTIVE:
            case MATRIX_DIRECTIVE:
                dc += calc_directive_words(&pl);
                break;

            case EXTERN_DIRECTIVE:
                /* record an extern symbol (address 0, flagged as extern). */
                name = pl.body.directive.operands.symbol_name;
                if (!symtab_insert(symtab, name, 0, SYM_EXTERN)) {
                    /* if it already exists as code/data or was .entry – reject */
                    symbol = symtab_lookup(symtab, name);
                    if (symbol && (symbol->flags & SYM_ENTRY)) {
                        print_error_file(input_path, ERROR_EXTERNAL_SYMBOL_CANNOT_BE_ENTRY, line_no);
                    } else {
                        print_error_file(input_path, ERROR_DUPLICATE_LABEL_DEFINITION, line_no);
                    }
                    errors++;
                }
                break;

            case ENTRY_DIRECTIVE:
                /* mark as entry now if it stays undefined after pass-1 error later */
                name = pl.body.directive.operands.symbol_name;
                if (!symtab_insert(symtab, name, 0, SYM_ENTRY)) {
                    symbol = symtab_lookup(symtab, name);
                    if (symbol && (symbol->flags & SYM_EXTERN)) {
                        print_error_file(input_path, ERROR_EXTERNAL_SYMBOL_CANNOT_BE_ENTRY, line_no);
                    } else {
                        print_error_file(input_path, ERROR_DUPLICATE_ENTRY_DECLARATION, line_no);
                    }
                    errors++;
                }
                break;
        }
    }

    fclose(fp);

    /* rebase data symbols so they start right after the code image. */
    rebase_data_symbols(symtab, ic);

    /* final validation every .entry must also be defined (code/data) and must not be extern */
    it = NULL; /* reset iterator for final validation */
    while ((symbol = symtab_iter_next(symtab, &it)) != NULL) {
        ok = (symbol->flags & SYM_ENTRY) != 0;
        st = (symbol->flags & (SYM_CODE | SYM_DATA)) != 0;
        line_no = (symbol->flags & SYM_EXTERN) != 0;

        if (ok && !st) {
            print_error_file(input_path, ERROR_ENTRY_SYMBOL_NOT_DEFINED, 0);
            errors++;
        }
        if (ok && line_no) {
            /* should have been caught earlier, but keep it robust */
            print_error_file(input_path, ERROR_EXTERNAL_SYMBOL_CANNOT_BE_ENTRY, 0);
            errors++;
        }
    }

    return errors;
}
