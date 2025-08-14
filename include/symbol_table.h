#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H
#include <stddef.h>
#include "globals.h"
#include "util_hash.h"

/* Bit flags for symbols */
#define SYM_CODE   (1<<0) /* Code symbol: function or instruction 0x01 */
#define SYM_DATA   (1<<1) /* Data symbol: variable or constant 0x02 */
#define SYM_ENTRY  (1<<2) /* Entry point symbol: used in the second pass 0x04 */
#define SYM_EXTERN (1<<3) /* External symbol: defined in another module 0x08 */

/** Symbol table structure */
typedef struct symbol {
    char name[MAX_LABEL_LENGTH]; /* 30 chars + '\0' */
    int address; /* word address */
    int flags; /* SYM_* bitmask */
} symbol_t;

/** Symbol table type definition */
typedef hash_table_t symbol_table_t;

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
 * @return 1 on success, 0 on failure (duplicate label, conflict with existing flags).
 * - Cannot have both SYM_CODE and SYM_DATA flags.
 * - Cannot have SYM_EXTERN and SYM_CODE/SYM_DATA at the same time.
 * - Cannot have SYM_ENTRY and SYM_EXTERN at the same time.
 * - Cannot have multiple SYM_ENTRY flags for the same symbol.
 * - If the symbol already exists, it updates the address and flags.
 */
int symtab_insert(symbol_table_t *st, const char *name, const int address, const int add_flags);

/**
 * Add final instruction counter (IC_final) to all data addresses.
 * This is used to rebase data symbols after the first pass.
 * The ic_final parameter is the final instruction counter value.
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

/**
 * @brief Performs the second pass of the assembler
 *
 * Generates machine code, creates .ob file with base-4 encoded instructions,
 * creates .ent file for entry symbols, and creates .ext file for external symbols
 *
 * @param input_path Path to the preprocessed assembly file (.am)
 * @param output_base Base name for output files (without extension)
 * @param symbol_table Symbol table from first pass
 * @return 0 on success, -1 on failure
 */
int second_pass(const char *input_path, const char *output_base, symbol_table_t *symbol_table);

#endif /* SYMBOL_TABLE_H */
