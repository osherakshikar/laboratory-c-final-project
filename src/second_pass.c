#include "../include/line_parser.h"
#include "../include/globals.h"
#include "../include/symbol_table.h"
#include "../include/second_pass.h"
#include "../include/errors.h"
#include "../include/util_vec.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
/*
 * =====================================================================================
 * Filename:  second_pass.c
 * Description: Second pass of the assembler that generates machine code,
 * creates object files, and handles external symbols.
 * It encodes instructions, resolves symbols, and writes the output files.
 * This module processes the parsed lines from the first pass, encodes them into
 * machine code, and manages external symbol usage.
 * =====================================================================================
 */

/* The base address for the code image.
 * It is used to calculate the absolute addresses of instructions and data.
 */
static void word_to_base4(WORD w, char *out, const int length) {
    int i, d;
    for (i = length - 2; i >= 0; --i) {
        d = w & 3; /* 2 bits mask */
        out[i] = (char) ('a' + d); /* 0-a, 1-b, 2-c, 3-d */
        w >>= 2; /* shift right by 2 bits to mask next 2 bits */
    }
    out[length - 1] = '\0';
}

/* Adds an external symbol usage to the context.
 * It stores the name and the address where the symbol is used.
 */
static void add_extern(second_pass_ctx_t *ctx, const char *name, const int addr) {
    ext_usage_t u;
    size_t n;

    n = strlen(name);
    memcpy(u.name, name, n);
    u.name[n] = '\0';
    u.address = addr;
    vec_push(&ctx->ext_list, &u);
}

/* encodes an operand into the code image.
 * It handles different addressing modes and returns the number of words used.
 * It returns -1 on error (e.g., symbol not found).
 */
static int encode_operand(second_pass_ctx_t *ctx, const operand_t *op, symbol_table_t *st,
                         const int addr_of_next_word, int is_source) {
    WORD w;
    symbol_t *sym;

    switch (op->mode) {
        case IMMEDIATE:
            w = (WORD) ((op->value.immediate_value) << 2);
            WORD_SET_ARE(w, ARE_A);
            ctx->code_image[ctx->code_pos++] = w;
            return 1;

        case DIRECT:
            sym = symtab_lookup(st, op->value.label);
            if (!sym) return -1;
            if (sym->flags & SYM_EXTERN) {
                add_extern(ctx, op->value.label, addr_of_next_word);
                w = 0;
                WORD_SET_ARE(w, ARE_E);
            } else {
                w = (WORD) ((sym->address) << 2);
                WORD_SET_ARE(w, ARE_R);
            }
            ctx->code_image[ctx->code_pos++] = w;
            return 1;

        case MATRIX_ACCESS:
            sym = symtab_lookup(st, op->value.label);
            if (!sym) return -1;
            if (sym->flags & SYM_EXTERN) {
                add_extern(ctx, op->value.label, addr_of_next_word);
                w = 0;
                WORD_SET_ARE(w, ARE_E);
            } else {
                w = (WORD) ((sym->address) << 2);
                WORD_SET_ARE(w, ARE_R);
            }
            ctx->code_image[ctx->code_pos++] = w;

            /* row bits 6..9, col bits 2..5, are=a */
            w = (WORD) ((op->row_reg << 6) | (op->col_reg << 2));
            WORD_SET_ARE(w, ARE_A);
            ctx->code_image[ctx->code_pos++] = w;
            return 2;

        case REGISTER_DIRECT:
            /* source: bits 6-9, destination: bits 2-5 */
            if (is_source) {
                w = (WORD) ((op->value.reg_num << 6) | ARE_A);
            } else {
                w = (WORD) ((op->value.reg_num << 2) | ARE_A);
            }
            ctx->code_image[ctx->code_pos++] = w;
            return 1;

        default:
            return 0;
    }
}

/* encodes an instruction into the code image.
 * It handles the opcode, addressing modes, and operands.
 * It returns 0 on success, or -1 on error.
 */
static int encode_instruction(second_pass_ctx_t *ctx, parsed_line *pl, symbol_table_t *st) {
    WORD first_word;
    WORD reg_word;
    operand_t *src;
    operand_t *dst;
    int n_ops;
    int used;

    src = &pl->body.operation.source_op;
    dst = &pl->body.operation.dest_op;
    n_ops = pl->body.operation.n_operands;

    if (n_ops == 0) {
        first_word = FIRST_WORD((pl->body.operation.opcode), 0, 0, ARE_A);
        ctx->code_image[ctx->code_pos++] = first_word;
        return 0; /* no operands, just the opcode */
    }

    /* first word opcode + addressing modes (are=00 for first line) */
    first_word = FIRST_WORD((pl->body.operation.opcode), (n_ops == 2) ? src->mode : 0, (n_ops > 1) ? dst->mode : src->mode, ARE_A);
    ctx->code_image[ctx->code_pos++] = first_word;

    /* if both operands are registers, one shared reg word (src 6..9, dst 2..5) */
    if (n_ops == 2 && src->mode == REGISTER_DIRECT && dst->mode == REGISTER_DIRECT) {
        reg_word = (WORD) ((src->value.reg_num << 6) | ((dst->value.reg_num) << 2) | ARE_A);
        ctx->code_image[ctx->code_pos++] = reg_word;
        return 0;
    }

    /* source extras first */
    if (n_ops >= 1) {
        used = encode_operand(ctx, src, st, ctx->code_pos + ADDRESS_BASE, 1);
        if (used < 0) return -1;
    }

    /* destination extras */
    if (n_ops >= 2) {
        used = encode_operand(ctx, dst, st, ctx->code_pos + ADDRESS_BASE, 0);
        if (used < 0) return -1;
    }
    return 0;
}

/* encodes a parsed line into the code image.
 * It handles operations and directives, encoding them into machine code.
 * It also manages external symbols and their usage.
 */
static void encode_data(second_pass_ctx_t *ctx, parsed_line *pl) {
    int i;
    WORD w;
    matrix_def_t *m;
    const char *s;

    switch (pl->body.directive.type) {
        case DATA_DIRECTIVE:
            for (i = 0; i < pl->body.directive.operands.data.count; ++i) {
                w = (WORD) (pl->body.directive.operands.data.values[i]);
                ctx->data_image[ctx->data_pos++] = w;
            }
            break;

        case STRING_DIRECTIVE:
            s = pl->body.directive.operands.string_val;
            while (*s) {
                w = (WORD) (*s);
                ctx->data_image[ctx->data_pos++] = w;
                ++s;
            }
            w = 0; /* null termnaitor */
            ctx->data_image[ctx->data_pos++] = w;
            break;

        case MATRIX_DIRECTIVE:
            m = &pl->body.directive.operands.mat;
            for (i = 0; i < m->rows * m->cols; ++i) {
                w = (WORD) (m->cells[i]);
                ctx->data_image[ctx->data_pos++] = w;
            }
            break;
        default:
            break;
    }
}

/* Write the object file (.ob)
 * It contains the code image and data image in base-4 format.
 * The first line contains the code length and data length in base-4.
 * Each subsequent line contains the address and the corresponding word in base-4.
 */
static int write_ob_file(const char *base_name, const second_pass_ctx_t *ctx) {
    char *path;
    FILE *fp;
    char b4_line[6];
    char b4_address[5];
    char b4_code_length[4];
    char b4_data_length[3];
    int i;

    path = create_file_path(base_name, ".ob");
    if (!path) return -1;

    fp = fopen(path, "w");
    if (!fp) {
        free(path);
        return -1;
    }

    /* write code length and data length */
    word_to_base4(ctx->code_pos, b4_code_length, sizeof(b4_code_length));
    fprintf(fp, "%s ", b4_code_length);
    word_to_base4(ctx->data_pos, b4_data_length, sizeof(b4_data_length));
    fprintf(fp, "%s\n", b4_data_length);

    for (i = 0; i < ctx->code_pos; ++i) {
        word_to_base4(ADDRESS_BASE + i, b4_address, sizeof(b4_address));
        word_to_base4(ctx->code_image[i], b4_line, sizeof(b4_line));
        fprintf(fp, "%s\t%s\n", b4_address, b4_line);
    }
    for (i = 0; i < ctx->data_pos; ++i) {
        word_to_base4(ctx->data_image[i], b4_line, sizeof(b4_line));
        word_to_base4(ADDRESS_BASE + i + ctx->code_pos , b4_address, sizeof(b4_address));
        fprintf(fp, "%s\t%s\n", b4_address, b4_line);
    }

    fclose(fp);
    free(path);
    return 0;
}

/* write the entry symbols file (.ent)
 * It contains the name of the entry symbol and its absolute address in the code image.
 * This is used to track where entry symbols are defined in the code.
 */
static int write_ent_file(const char *base_name, symbol_table_t *st) {
    char *path;
    FILE *fp;
    hash_entry_t *it;
    symbol_t *sym;
    int has_any;
    char b4_address[5]; /* 4 digits + null terminator */

    path = create_file_path(base_name, ".ent");
    if (!path) return -1;

    /* first pass: see if there are any entries */
    has_any = 0;
    it = NULL;
    for (it = hash_get_next(st, NULL); it; it = hash_get_next(st, it)) {
        sym = (symbol_t *) (it->value);
        if (sym && (sym->flags & SYM_ENTRY)) {
            has_any = 1;
            break;
        }
    }
    if (!has_any) {
        free(path);
        return 0;
    }

    fp = fopen(path, "w");
    if (!fp) {
        free(path);
        return -1;
    }

    /* second pass: print them */
    for (it = hash_get_next(st, NULL); it; it = hash_get_next(st, it)) {
        sym = (symbol_t *) (it->value);
        if (sym && (sym->flags & SYM_ENTRY)) {
            word_to_base4(sym->address, b4_address, sizeof(b4_address));
            fprintf(fp, "%s\t%s\n", sym->name, b4_address );
        }
    }

    fclose(fp);
    free(path);
    return 0;
}

/* write the external symbols file (.ext)
 * It contains the name of the external symbol and its absolute address in the code image.
 * This is used to track where external symbols are referenced in the code.
 */
static int write_ext_file(const char *base_name, const second_pass_ctx_t *ctx) {
    char *path;
    FILE *fp;
    const ext_usage_t *u;
    size_t i;
    char b4_address[5];

    if (ctx->ext_list.len == 0) return 0;

    path = create_file_path( base_name, ".ext");
    if (!path) return -1;

    fp = fopen(path, "w");
    if (!fp) {
        free(path);
        return -1;
    }

    for (i = 0; i < ctx->ext_list.len; i++) {
        u = (ext_usage_t *) vec_get(&ctx->ext_list, i);
        word_to_base4(u->address, b4_address, sizeof(b4_address));
        fprintf(fp, "%s\t%s\n", u->name, b4_address);
    }

    fclose(fp);
    free(path);
    return 0;
}

int second_pass(const char *input_path, const char *file_name, symbol_table_t *symtab) {
    second_pass_ctx_t ctx;
    FILE *fp;
    char line_buf[MAX_LINE_LENGTH];
    parsed_line pl;
    error_code_t st;
    int error_flag = 0;
    int line_no = 0;

    memset(&ctx, 0, sizeof(ctx)); /* zero init */
    vec_create(&ctx.ext_list, sizeof(ext_usage_t)); /* initialize vector for external usage tracking */

    if (!input_path || !symtab) return -1;

    fp = fopen(input_path, "r");
    if (!fp) {
        print_error_file(file_name, ERROR_CANNOT_OPEN_FILE, 0);
        return -1;
    }

    while (fgets(line_buf, sizeof(line_buf), fp)) {
        line_no++;
        st = parse_line(line_buf, &pl);
        if (st != ERROR_OK) continue;

        if (pl.kind == LINE_OPERATION) {
            error_flag = encode_instruction(&ctx, &pl, symtab);
            if (error_flag < 0) {
                fclose(fp);
                vec_destroy(&ctx.ext_list);
                print_error_file(file_name, ERROR_UNDEFINED_SYMBOL_USED, line_no);
                return -1;
            }
        } else if (pl.kind == LINE_DIRECTIVE) {
            if (pl.body.directive.type == DATA_DIRECTIVE || pl.body.directive.type == STRING_DIRECTIVE || pl.body.directive.type == MATRIX_DIRECTIVE) {
                encode_data(&ctx, &pl);
            }
        }
    }

    fclose(fp);

    /* write outputs */
    if (write_ob_file(file_name, &ctx) != 0 ||
        write_ent_file(file_name, symtab) != 0 ||
        write_ext_file(file_name, &ctx) != 0) {
        vec_destroy(&ctx.ext_list);
        print_error(ERROR_WRITE_FAILED);
        return -1;
    }

    vec_destroy(&ctx.ext_list);
    return 0;
}