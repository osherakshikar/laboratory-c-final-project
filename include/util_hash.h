#ifndef UTIL_HASH_H
#define UTIL_HASH_H
#include <stddef.h>
/*
 * =====================================================================================
 * Filename: util_hash.h
 * Description: Header file for a generic hash table implementation using the djb2 hash
 * function and collision resolution with the Chaining method.
 * =====================================================================================
 */

#define HASH_STARTING_VAL 5381 /* The initial value for the hash function, commonly used in djb2 */
#define DJ_SHIFT 5 /* The number of bits to shift left in the hash function */
#define INITIAL_CAPACITY 32 /* The initial capacity of the hash table */

/**
 * A hash entry structure that represents a key-value pair in the hash table.
 * It contains a key, a value, and a pointer to the next entry for chaining in case of collisions.
 */
typedef struct hash_entry_t {
    char *key;
    void *value;
    struct hash_entry_t *next; /* for chaining */
} hash_entry_t;

/**
 * A hash table structure that uses chaining for collision resolution.
 * It contains an array of pointers to hash_entry_t, which represent the entries in the table.
 * Each entry contains a key-value pair and a pointer to the next entry in case of a collision.
 */
typedef struct {
    size_t capacity;
    size_t size;
    hash_entry_t **tbl;
} hash_table_t;

/**
 * Creates a new hash table with the specified capacity.
 * The capacity should be a power of 2.
 *
 * @param pow2_cap The initial capacity of the hash table, should be a power of 2
 * @return Pointer to the newly created hash table, or NULL if memory allocation fails
 */

hash_table_t *hash_create(size_t pow2_cap);

/**
 * Destroys the hash table and frees all allocated memory.
 * If destroy_val is not NULL, it will be called for each value in the hash table.
 *
 * @param ht Pointer to the hash table to destroy
 * @param destroy_val Function pointer to a function that destroys the value, can be NULL
 */
void hash_destroy(hash_table_t *ht, void (*destroy_val)(void *));

/**
 * Puts a key-value pair into the hash table.
 * If the key already exists, it updates the value.
 * Old value is not freed, caller is responsible.
 *
 * @param ht Pointer to the hash table
 * @param key The key to insert or update
 * @param value The value associated with the key
 * @return 0 on success, -1 on failure
 */
int hash_put(hash_table_t *ht, const char *key, void *value);

/**
 * Gets the value associated with a key in the hash table.
 *
 * @param ht Pointer to the hash table
 * @param key The key to look up
 * @return Pointer to the value associated with the key, or NULL if not found
 */
void *hash_get(const hash_table_t *ht, const char *key);

/**
 * Removes a key-value pair from the hash table.
 * If destroy_val is not NULL, it will be called for the value before removing it.
 *
 * @param ht Pointer to the hash table
 * @param key The key to remove
 * @param destroy_val Function pointer to a function that destroys the value, can be NULL
 * @return 0 on success, -1 on failure
 */
int hash_remove(hash_table_t *ht, const char *key, void (*destroy_val)(void *));

/**
 * Computes the size of the hash table.
 *
 * @param ht Pointer to the hash table
 * @return The number of entries in the hash table
 */
size_t hash_size(const hash_table_t *ht);

/**
 * Gets the next entry in the hash table after the current entry.
 * If current is NULL, it returns the first entry in the table.
 * If there are no more entries, it returns NULL.
 *
 * @param ht Pointer to the hash table
 * @param current Pointer to the current hash entry, or NULL to start from the beginning
 * @return Pointer to the next hash entry, or NULL if no more entries
 */
hash_entry_t *hash_get_next(hash_table_t *ht, const hash_entry_t *current);

#endif
