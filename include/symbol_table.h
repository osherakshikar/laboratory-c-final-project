#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H
#include "globals.h"
#include "util_hash.h"

/*
 * =====================================================================================
 * Filename:  symbol_table.h
 * Description: Header file for the symbol table used in the assembler.
 * This file defines the data structures and functions for managing symbols in the assembler.
 * It includes the symbol_t structure, which represents a symbol with its name, address, and flags.
 * The symbol table is implemented as a hash table for efficient symbol lookup and insertion.
 * The file also defines functions for creating, destroying, and manipulating the symbol table.
 * =====================================================================================
 */

/* bit flags for symbols */
#define SYM_CODE   (1<<0) /* Code symbol: function or instruction 0x01 */
#define SYM_DATA   (1<<1) /* Data symbol: variable or constant 0x02 */
#define SYM_ENTRY  (1<<2) /* Entry point symbol: used in the second pass 0x04 */
#define SYM_EXTERN (1<<3) /* External symbol: defined in another module 0x08 */

/* struct to define symbol_t table */
typedef struct symbol {
    char name[MAX_LABEL_LENGTH]; /* 30 chars + '\0' */
    int address; /* word address */
    int flags; /* SYM_* bitmask */
} symbol_t;

typedef hash_table_t symbol_table_t; /* symbol table is a hash table of symbol_t structures */

/**
 * @brief Create a new symbol table.
 *
 * Allocates memory for a new symbol table and initializes it.
 * The symbol table is implemented as a hash table.
 *
 * @return Pointer to the newly created symbol_table_t structure.
 */
symbol_table_t *symtab_create(void);

/**
 * @brief Destroy the symbol table and free all allocated memory.
 *
 * @param st Pointer to the symbol table to destroy.
 */
void symtab_destroy(symbol_table_t *st);

/**
 * @brief Lookup a symbol by name in the symbol table.
 *
 * @param st Pointer to the symbol table.
 * @param name Name of the symbol to look up.
 * @return Pointer to the symbol_t structure if found, NULL if not found.
 */
symbol_t *symtab_lookup(symbol_table_t *st, const char *name);

/**
 * @brief Insert a new symbol into the symbol table.
 *
 * @param st Pointer to the symbol table.
 * @param name Name of the symbol (must be unique).
 * @param address Address of the symbol (for code/data symbols).
 * @param add_flags Flags for the symbol (SYM_CODE, SYM_DATA, SYM_ENTRY, SYM_EXTERN).
 * @return 1 on success, 0 on failure (duplicate label, conflict with existing flags)..
 */
int symtab_insert(symbol_table_t *st, const char *name, const int address, const int add_flags);

/**
 * @brief Bump the addresses of data symbols in the symbol table.
 *
 * this function adjusts the addresses of all data symbols
 * by the final instruction count (ic_final) to account for the size of code.
 *
 * @param st Pointer to the symbol table.
 * @param ic_final Final instruction count from the first pass.
 */
void symtab_bump_data_addresses(symbol_table_t *st, int ic_final);

/**
 * @brief Iterate over the symbols in the symbol table.
 *
 * This function allows iterating over all symbols in the symbol table.
 * It returns the next symbol in the iteration order.
 *
 * @param st Pointer to the symbol table.
 * @param iter Pointer to a hash_entry_t pointer that keeps track of the current position.
 *             Should be initialized to NULL for the first call.
 * @return Pointer to the next symbol_t structure, or NULL if there are no more symbols.
 */
symbol_t *symtab_iter_next(symbol_table_t *st, hash_entry_t **iter);

/**
 * @brief Performs the first pass of the assembler
 *
 * Parses the assembly file, builds the symbol table, and calculates instruction and data sizes.
 * It also checks for duplicate labels and entry/extern conflicts.
 *
 * @param input_path Path to the assembly file (.am)
 * @param symbol_table Pointer to the symbol table to populate
 * @return 0 on success, -1 on failure
 */
int first_pass(const char *input_path, symbol_table_t *symbol_table);
#endif
