#ifndef UTIL_VEC_H
#define UTIL_VEC_H
#include <stddef.h>

typedef struct { void *data; size_t len, cap, elem_sz; } vec_t;
void  vec_init(vec_t *v, size_t elem_sz);
int   vec_push(vec_t *v, const void *elem);
void *vec_get(const vec_t *v, size_t idx);
void  vec_free(vec_t *v);
#endif