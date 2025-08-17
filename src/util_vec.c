#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "../include/util_vec.h"

#include <stdio.h>

/*
 * =====================================================================================
 * Filename:  util_vec.c
 * Description: Implementation of a generic dynamic vector (dynamic array).
 * This structure can grow automatically as elements are added.
 * =====================================================================================
 */

void vec_create(vec_t *v, const size_t elem_size) {
    if (!v || elem_size == 0) {
        return;
    }
    v->data = NULL;
    v->len = 0;
    v->cap = 0;
    v->elem_sz = elem_size;
}

int vec_push(vec_t *v, const void *elem) {
    size_t new_capacity;
    void *new_data;
    char *dest;

    if (!v || !elem) {
        return -1;
    }

    if (v->len >= v->cap) {
        new_capacity = (v->cap == 0) ? INIT_VEC_SIZE : v->cap * 2;
        new_data = realloc(v->data, new_capacity * v->elem_sz);

        if (!new_data) {
            printf("Error: Memory allocation failed while resizing vector.\n");
            return -1;
        } else {
            v->data = new_data;
            v->cap = new_capacity;
        }
    }

    /* Calculate the destination address for the new element
       Cast to char* to perform byte-level pointer arithmetic */
    dest = (char *) v->data + (v->len * v->elem_sz);

    /* Copy the element to the vector */
    memcpy(dest, elem, v->elem_sz);
    v->len++;

    return 0;
}

void *vec_get(const vec_t *v, size_t idx) {
    char *base;

    if (!v || idx >= v->len) {
        return NULL;
    }

    base = (char *) v->data;
    return base + (idx * v->elem_sz);
}

void vec_destroy(vec_t *v) {
    if (!v) return;
    
    if (v->data) {
        free(v->data);
        v->data = NULL;
    }
    v->len = 0;
    v->cap = 0;
    v->elem_sz = 0;
}
