#ifndef UTIL_HASH_H
#define UTIL_HASH_H
#include <stddef.h>

typedef struct {
    char *key;
    void *value;
    int state; /* 0=empty, 1=full, -1=deleted */
} hash_entry_t;

typedef struct {
    size_t cap, size;
    hash_entry_t *tbl;
} hash_table_t;

hash_table_t *hash_create(size_t pow2_cap);

void hash_destroy(hash_table_t *, void (*destroy_val)(void *));

int hash_put(hash_table_t *, const char *, void *);

void *hash_get(const hash_table_t *, const char *);

int hash_remove(hash_table_t *, const char *, void (*destroy_val)(void *));
#endif
