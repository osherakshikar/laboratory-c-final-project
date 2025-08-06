#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../include/util_vec.h"

/* Test data structures */
typedef struct {
    int x, y;
} point_t;

/* Helper function to compare integers */
int compare_int(const void *a, const void *b) {
    int ia = *(const int*)a;
    int ib = *(const int*)b;
    return (ia > ib) - (ia < ib);
}

/* Test vec_create */
void test_vec_create() {
    vec_t v;

    printf("Testing vec_create...\n");

    /* Test normal creation */
    vec_create(&v, sizeof(int));
    assert(v.data == NULL);
    assert(v.len == 0);
    assert(v.cap == 0);
    assert(v.elem_sz == sizeof(int));

    /* Test with NULL pointer */
    vec_create(NULL, sizeof(int));  /* Should not crash */

    /* Test with zero element size */
    vec_create(&v, 0);  /* Should not crash */

    printf("✓ vec_create tests passed\n");
}

/* Test vec_push */
void test_vec_push() {
    vec_t v;
    int val1 = 42;
    int i;
    int expected;

    printf("Testing vec_push...\n");

    vec_create(&v, sizeof(int));

    /* Test pushing to empty vector */
    assert(vec_push(&v, &val1) == 0);
    assert(v.len == 1);
    assert(v.cap == INIT_VEC_SIZE);
    assert(*(int*)vec_get(&v, 0) == 42);

    /* Test pushing multiple elements */
    for (i = 2; i <= 10; i++) {
        assert(vec_push(&v, &i) == 0);
        assert(v.len == (size_t)i);
    }

    /* Test capacity expansion (should trigger realloc) */
    for (i = 11; i <= 20; i++) {
        assert(vec_push(&v, &i) == 0);
    }
    assert(v.len == 20);
    assert(v.cap >= 20);

    /* Verify all elements are correct */
    for (i = 0; i < 10; i++) {
        expected = (i == 0) ? 42 : i + 1;
        assert(*(int*)vec_get(&v, (size_t)i) == expected);
    }
    for (i = 10; i < 20; i++) {
        assert(*(int*)vec_get(&v, (size_t)i) == i + 1);
    }

    /* Test NULL parameters */
    assert(vec_push(NULL, &val1) == -1);
    assert(vec_push(&v, NULL) == -1);

    vec_destroy(&v);
    printf("✓ vec_push tests passed\n");
}

/* Test vec_get */
void test_vec_get() {
    vec_t v;
    int i;
    int *ptr;

    printf("Testing vec_get...\n");

    vec_create(&v, sizeof(int));

    /* Test getting from empty vector */
    assert(vec_get(&v, 0) == NULL);

    /* Add some elements */
    for (i = 0; i < 5; i++) {
        vec_push(&v, &i);
    }

    /* Test valid indices */
    for (i = 0; i < 5; i++) {
        ptr = (int*)vec_get(&v, (size_t)i);
        assert(ptr != NULL);
        assert(*ptr == i);
    }

    /* Test out of bounds */
    assert(vec_get(&v, 5) == NULL);
    assert(vec_get(&v, 100) == NULL);

    /* Test NULL vector */
    assert(vec_get(NULL, 0) == NULL);

    vec_destroy(&v);
    printf("✓ vec_get tests passed\n");
}

/* Test vec_destroy */
void test_vec_destroy() {
    vec_t v;
    int i;

    printf("Testing vec_destroy...\n");

    vec_create(&v, sizeof(int));

    /* Add some elements */
    for (i = 0; i < 10; i++) {
        vec_push(&v, &i);
    }

    /* Destroy and verify cleanup */
    vec_destroy(&v);
    assert(v.data == NULL);
    assert(v.len == 0);
    assert(v.cap == 0);
    assert(v.elem_sz == 0);

    /* Test destroying NULL vector */
    vec_destroy(NULL);  /* Should not crash */

    /* Test destroying already destroyed vector */
    vec_destroy(&v);  /* Should not crash */

    printf("✓ vec_destroy tests passed\n");
}

/* Test with different data types */
void test_different_types() {
    vec_t str_vec;
    vec_t point_vec;
    char *strings[] = {"hello", "world", "test"};
    point_t points[] = {{1, 2}, {3, 4}, {5, 6}};
    int i;
    char **ptr_str;
    point_t *ptr_point;

    printf("Testing with different data types...\n");

    /* Test with strings */
    vec_create(&str_vec, sizeof(char*));

    for (i = 0; i < 3; i++) {
        vec_push(&str_vec, &strings[i]);
    }

    for (i = 0; i < 3; i++) {
        ptr_str = (char**)vec_get(&str_vec, (size_t)i);
        assert(ptr_str != NULL);
        assert(strcmp(*ptr_str, strings[i]) == 0);
    }

    vec_destroy(&str_vec);

    /* Test with structures */
    vec_create(&point_vec, sizeof(point_t));

    for (i = 0; i < 3; i++) {
        vec_push(&point_vec, &points[i]);
    }

    for (i = 0; i < 3; i++) {
        ptr_point = (point_t*)vec_get(&point_vec, (size_t)i);
        assert(ptr_point != NULL);
        assert(ptr_point->x == points[i].x);
        assert(ptr_point->y == points[i].y);
    }

    vec_destroy(&point_vec);

    printf("✓ Different data types tests passed\n");
}

/* Test large data sets */
void test_large_dataset() {
    vec_t v;
    const int large_size = 10000;
    int i;
    int *ptr;

    printf("Testing with large dataset...\n");

    vec_create(&v, sizeof(int));

    /* Push large number of elements */
    for (i = 0; i < large_size; i++) {
        assert(vec_push(&v, &i) == 0);
    }

    assert(v.len == (size_t)large_size);

    /* Verify all elements */
    for (i = 0; i < large_size; i++) {
        ptr = (int*)vec_get(&v, (size_t)i);
        assert(ptr != NULL);
        assert(*ptr == i);
    }

    vec_destroy(&v);

    printf("✓ Large dataset tests passed\n");
}

/* Test edge cases */
void test_edge_cases() {
    vec_t v;
    char large_elem[1000];
    char *retrieved;
    int i;

    printf("Testing edge cases...\n");

    /* Initialize array */
    for (i = 0; i < 1000; i++) {
        large_elem[i] = 0;
    }

    /* Test with very large element size */
    vec_create(&v, 1000);
    large_elem[0] = 'A';
    large_elem[999] = 'Z';

    assert(vec_push(&v, large_elem) == 0);
    retrieved = (char*)vec_get(&v, 0);
    assert(retrieved[0] == 'A');
    assert(retrieved[999] == 'Z');

    vec_destroy(&v);

    printf("✓ Edge cases tests passed\n");
}

int main() {
    printf("Running vector tests...\n\n");

    test_vec_create();
    test_vec_push();
    test_vec_get();
    test_vec_destroy();
    test_different_types();
    test_large_dataset();
    test_edge_cases();

    printf("\nAll tests passed!\n");
    return 0;
}