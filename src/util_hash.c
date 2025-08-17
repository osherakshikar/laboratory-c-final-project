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
    if (!ht) return NULL;

    if (pow2_cap < 4) pow2_cap = INITIAL_CAPACITY;
    if ((pow2_cap & (pow2_cap - 1)) != 0) /* Not power of 2 */
        pow2_cap = next_pow2(pow2_cap);

    ht->capacity = pow2_cap;
    ht->size = 0;

    /* allocate an array of pointers, and initialize all to NULL */
    ht->tbl = calloc(ht->capacity, sizeof(hash_entry_t *));
    if (!ht->tbl) {
        free(ht);
        return NULL;
    }
    return ht;
}

void hash_destroy(hash_table_t *ht, void (*destroy_val)(void *)) {
    size_t i;
    hash_entry_t *entry, *next;

    if (!ht) return;

    /* free all entries in the hash table */
    for (i = 0; i < ht->capacity; i++) {
        entry = ht->tbl[i];
        while (entry) {
            next = entry->next;
            free(entry->key);
            if (destroy_val) destroy_val(entry->value); /* call the user-defined function to destroy the value */
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

    if (!ht || !key) return -1;

    hash = djb2(key);
    index = hash & (ht->capacity - 1);

    entry = ht->tbl[index]; /* get head of the chain */
    /* check if the key already exists in the chain */
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            /* key already exists, update value */
            entry->value = value;
            return 0;
        }
        entry = entry->next;
    }

    /* ff key not found crate new entry and add it to head of list */
    new_entry = malloc(sizeof(hash_entry_t));
    if (!new_entry) return -1;

    new_entry->key = dupstr(key);
    if (!new_entry->key) {
        free(new_entry);
        return -1;
    }
    new_entry->value = value;
    new_entry->next = ht->tbl[index]; /* point to the current head of the chain */
    ht->tbl[index] = new_entry; /* insert at the head of the chain */
    ht->size++;

    return 0;
}

void *hash_get(const hash_table_t *ht, const char *key) {
    unsigned long hash;
    size_t index, mask;
    hash_entry_t *entry;

    if (!ht || !key) return NULL;

    hash = djb2(key);
    mask = ht->capacity - 1;
    index = hash & mask;

    if (!ht->tbl || !ht->tbl[index]) return NULL; /* hash table is empty or key not found */

    for (entry = ht->tbl[index]; entry; entry = entry->next) {
        if (strcmp(entry->key, key) == 0) {
            return entry->value; /* key found return value */
        }
    }
    return NULL; /* key not found */
}

int hash_remove(hash_table_t *ht, const char *key, void (*destroy_val)(void *)) {
    unsigned long hash;
    size_t index, mask;
    hash_entry_t *entry, *prev = NULL;

    if (!ht || !key) return -1;


    hash = djb2(key);
    mask = ht->capacity - 1;
    index = hash & mask;

    entry = ht->tbl[index]; /* get head of the chain */
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            /* key found remove it */
            if (prev) {
                prev->next = entry->next; /* bypass the entry to remove it */
            } else {
                ht->tbl[index] = entry->next; /* remove from head of the chain */
            }
            free(entry->key);
            if (destroy_val) destroy_val(entry->value); /* call the user defined function to destroy the value */
            free(entry);
            ht->size--;
            return 0; /* success */
        }
        prev = entry;
        entry = entry->next;
    }
    return -1; /* key not found */
}

size_t hash_size(const hash_table_t *ht) {
    if (!ht) return 0;
    return ht->size;
}

hash_entry_t *hash_get_next(hash_table_t *ht, const hash_entry_t *current) {
    size_t index = 0;

    if (!ht || ht->size == 0) return NULL;

    if (current) {
        /* if there's a next entry in the current chain return it */
        if (current->next) {
            return current->next;
        }
        /* otherwise find the next bucket to start searching from */
        index = (djb2(current->key) & (ht->capacity - 1)) + 1;
    }
    /* find the next nonempty bucket and return its first entry */
    for (; index < ht->capacity; index++) {
        if (ht->tbl[index]) {
            return ht->tbl[index];
        }
    }
    /* no more entries found */
    return NULL;
}
