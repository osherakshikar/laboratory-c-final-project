#include <stdlib.h>
#include <string.h>
#include "../include/symbol_table.h"
#include "../include/globals.h"


/*
 * =====================================================================================
 * Filename:  symbol_table.c
 * Description: Implementation of a symbol table for the assembler.
 * This table stores symbols with their names, addresses, and flags.
 * It allows insertion, lookup, and iteration over symbols.
 * Conflicts are checked to ensure that symbols do not have incompatible flags.
 * =====================================================================================
 */

static int check_symbol_conflicts(int existing_flags, int new_flags) {
    if ((new_flags & (SYM_CODE | SYM_DATA)) && (existing_flags & (SYM_CODE | SYM_DATA))) return 1;
    if ((new_flags & (SYM_CODE | SYM_DATA)) && (existing_flags & SYM_EXTERN)) return 1;
    if ((new_flags & SYM_EXTERN) && (existing_flags & (SYM_CODE | SYM_DATA))) return 1;
    if ((new_flags & SYM_ENTRY) && (existing_flags & SYM_ENTRY)) return 1;
    if ((new_flags & SYM_ENTRY) && (existing_flags & SYM_EXTERN)) return 1;
    if ((new_flags & SYM_EXTERN) && (existing_flags & SYM_ENTRY)) return 1;
    return 0;
}

int symtab_insert(symbol_table_t *st, const char *name, const int address, const int add_flags) {
    symbol_t *s;
    if (!st || !name) return 0;

    s = (symbol_t *) hash_get(st, name);
    if (s) {
        if (check_symbol_conflicts(s->flags, add_flags)) return 0;

        /* update existing symbol if it data/code */
        if (add_flags & (SYM_CODE | SYM_DATA)) s->address = address;
        s->flags |= add_flags;
        return 1;
    }

    /* create new symbol */
    s = (symbol_t *) malloc(sizeof(*s));
    if (!s) return 0;

    strncpy(s->name, name, MAX_LABEL_LENGTH - 1);
    s->name[MAX_LABEL_LENGTH - 1] = '\0';
    s->address = address;
    s->flags = add_flags;

    if (hash_put(st, s->name, s) != 0) {
        free(s);
        return 0;
    }
    return 1;
}

symbol_table_t *symtab_create(void) {
    return hash_create(INITIAL_CAPACITY);
}

void symtab_destroy(symbol_table_t *st) {
    if (st) hash_destroy(st, free); /* properly free symbol_t structures */
}

symbol_t *symtab_lookup(symbol_table_t *st, const char *name) {
    return st ? (symbol_t *) hash_get(st, name) : NULL;
}

void symtab_bump_data_addresses(symbol_table_t *st, const int ic_final) {
    hash_entry_t *it = NULL;
    symbol_t *s = NULL;
    if (!st) return;
    for (it = hash_get_next(st, NULL); it; it = hash_get_next(st, it)) {
        s = (symbol_t *) it->value;
        if (s && (s->flags & SYM_DATA)) s->address += ic_final;
    }
}

symbol_t *symtab_iter_next(symbol_table_t *st, hash_entry_t **iter) {
    if (!iter) return NULL;

    if (!*iter) {
        /* start iteration from beginning */
        *iter = st ? hash_get_next(st, NULL) : NULL;
    } else {
        /* continue iteration */
        *iter = hash_get_next(st, *iter);
    }
    return *iter ? (symbol_t *) (*iter)->value : NULL;
}

