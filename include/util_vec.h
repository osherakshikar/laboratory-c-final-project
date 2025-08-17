#ifndef UTIL_VEC_H
#define UTIL_VEC_H
#include <stddef.h>

/*
 * =====================================================================================
 * Filename:  util_vec.h
 * Description: Header file for a generic dynamic vector implementation.
 * This structure can grow automatically as elements are added.
 * =====================================================================================
 */

#define INIT_VEC_SIZE 8  /* Initial size of the vector */

/**
 * A vector structure that represents a dynamic array.
 * It contains a pointer to the data array, its length, capacity, and size of each element.
 */
typedef struct {
    void *data; /* Pointer to the data array */
    size_t len;
    size_t cap;
    size_t elem_sz; /* Size of each element in bytes */
} vec_t;

/**
 * Initializes a vector structure.
 *
 * @param v Pointer to the vector structure to initialize
 * @param elem_size Size of each element in bytes
 */
void vec_create(vec_t *v, size_t elem_size);

/**
 * Destroys a vector and frees all allocated memory.
 *
 * @param v Pointer to the vector structure to destroy
 */
void vec_destroy(vec_t *v);

/**
 * Adds an element to the vector. If the vector is full, it will resize it.
 *
 * @param v Pointer to the vector structure
 * @param elem Pointer to the element to add
 * @return 0 on success, -1 on failure
 */
int vec_push(vec_t *v, const void *elem);

/**
 * Retrieves an element from the vector by index.
 *
 * @param v Pointer to the vector structure
 * @param idx Index of the element to retrieve
 * @return Pointer to the element, or NULL if index is out of bounds
 */
void *vec_get(const vec_t *v, size_t idx);

#endif