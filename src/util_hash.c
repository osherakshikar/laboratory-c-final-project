#include "../include/util_hash.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * =====================================================================================
 * Filename: util_hash.c
 * Description: Implementation of a generic hash table using the djb2 hash
 * function and collision resolution with the Chaining method.
 * =====================================================================================
 */

/* --- Private Helper Functions --- */

/* The djb2 hash function.
 * Created by Dan Bernstein.
 * This is a simple and effective hash function that computes a hash value for a string.
 * str The input string to hash
 * Returns the computed hash value as an unsigned long
 */
static unsigned long djb2(const char *str) {
    unsigned long hash = HASH_STARTING_VAL;
    int c;

    while ((c = (unsigned char) *str++))
        hash = ((hash << DJ_SHIFT) + hash) + c; /* hash * 33(shift left 5 + 5381) + c */
    return hash;
}

/* Computes the next power of 2 greater than or equal to x.
 * This is used to ensure that the hash table's capacity is always a power of 2.
 * x The input value
 * Returns the next power of 2 greater than or equal to x
 */
static size_t next_pow2(const size_t x) {
    size_t p = 1;
    while (p < x) p <<= 1;
    return p;
}

/* Duplicates a string.
 * Allocates memory for the new string and copies the content of the original string into it.
 * str The input string to duplicate
 * Returns a pointer to the newly allocated string, or NULL if memory allocation fails
 */
static char *dupstr(const char *str) {
    char *dup = malloc(strlen(str) + 1);
    if (dup) {
        strcpy(dup, str);
    }
    return dup;
}

/* Public API Functions Implementation */

hash_table_t *hash_create(size_t pow2_cap) {
    hash_table_t *ht;

    ht = malloc(sizeof(hash_table_t));
    if (!ht) {
        printf("Error: Could not allocate memory for hash table\n");
        return NULL;
    }

    if (pow2_cap < 4) pow2_cap = INITIAL_CAPACITY;
    if ((pow2_cap & (pow2_cap - 1)) != 0) /* Not power of 2 */
        pow2_cap = next_pow2(pow2_cap);

    ht->capacity = pow2_cap;
    ht->size = 0;

    /* Allocate an array of pointers, and initialize all to NULL */
    ht->tbl = calloc(ht->capacity, sizeof(hash_entry_t *));
    if (!ht->tbl) {
        free(ht);
        printf("Error: Could not allocate memory for hash table entries\n");
        return NULL;
    }
    return ht;
}

void hash_destroy(hash_table_t *ht, void (*destroy_val)(void *)) {
    size_t i;
    hash_entry_t *entry, *next;

    if (!ht) return;

    /* Free all entries in the hash table */
    for (i = 0; i < ht->capacity; i++) {
        entry = ht->tbl[i];
        while (entry) {
            next = entry->next;
            free(entry->key);
            if (destroy_val) destroy_val(entry->value); /* Call the user-defined function to destroy the value */
            free(entry);
            entry = next;
        }
    }
    free(ht->tbl);
    free(ht);
}

int hash_put(hash_table_t *ht, const char *key, void *value) {
    size_t index;
    unsigned long hash;
    hash_entry_t *entry, *new_entry;

    if (!ht || !key) {
        printf("Error: Invalid hash table or key\n");
        return -1;
    }

    hash = djb2(key);
    index = hash & (ht->capacity - 1);

    entry = ht->tbl[index]; /* Get head of the chain */
    /* Check if the key already exists in the chain */
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            /* Key already exists, update value */
            entry->value = value;
            return 0;
        }
        entry = entry->next;
    }

    /* If key not found, crate new entry and add it to head of list */
    new_entry = malloc(sizeof(hash_entry_t));
    if (!new_entry) {
        printf("Error: Could not allocate memory for new hash entry\n");
        return -1;
    }
    new_entry->key = dupstr(key);
    if (!new_entry->key) {
        printf("Error: Could not allocate memory for new hash entry key\n");
        free(new_entry);
        return -1;
    }
    new_entry->value = value;
    new_entry->next = ht->tbl[index]; /* Point to the current head of the chain */
    ht->tbl[index] = new_entry; /* Insert at the head of the chain */
    ht->size++;

    return 0;
}

void *hash_get(const hash_table_t *ht, const char *key) {
    unsigned long hash;
    size_t index, mask;
    hash_entry_t *entry;

    hash = djb2(key);
    mask = ht->capacity - 1;
    index = hash & mask;

    if (!ht->tbl || !ht->tbl[index]) {
        return NULL; /* Hash table is empty or key not found */
    }
    for (entry = ht->tbl[index]; entry; entry = entry->next) {
        if (strcmp(entry->key, key) == 0) {
            return entry->value; /* Key found, return value */
        }
    }
    return NULL; /* Key not found */
}

int hash_remove(hash_table_t *ht, const char *key, void (*destroy_val)(void *)) {
    unsigned long hash;
    size_t index, mask;
    hash_entry_t *entry, *prev = NULL;

    if (!ht || !key) {
        printf("Error: Invalid hash table or key\n");
        return -1;
    }

    hash = djb2(key);
    mask = ht->capacity - 1;
    index = hash & mask;

    entry = ht->tbl[index]; /* Get head of the chain */
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            /* Key found, remove it */
            if (prev) {
                prev->next = entry->next; /* Bypass the entry to remove it */
            } else {
                ht->tbl[index] = entry->next; /* Remove from head of the chain */
            }
            free(entry->key);
            if (destroy_val) destroy_val(entry->value); /* Call the user defined function to destroy the value */
            free(entry);
            ht->size--;
            return 0; /* Success */
        }
        prev = entry;
        entry = entry->next;
    }
    return -1; /* Key not found */
}

size_t hash_size(const hash_table_t *ht) {
    if (!ht) {
        printf("Error: Invalid hash table\n");
        return 0;
    }
    return ht->size;
}

hash_entry_t *hash_get_next(hash_table_t *ht, const hash_entry_t *current) {
    size_t index = 0;

    if (!ht) {
        return NULL;
    }

    if (current) {
        /* if there's a next entry in the current chain, return it */
        if (current->next) {
            return current->next;
        }
        /* otherwise, find the next bucket to start searching from */
        index = (djb2(current->key) & (ht->capacity - 1)) + 1;
    }

    /* find the next non-empty bucket and return its first entry */
    for (; index < ht->capacity; index++) {
        if (ht->tbl[index]) {
            return ht->tbl[index];
        }
    }

    /* no more entries found */
    return NULL;
}
