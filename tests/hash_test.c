#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "../include/util_hash.h"

#define RUN_TEST(test_func) do { \
printf("  Running %s... ", #test_func); \
test_func(); \
printf("PASSED\n"); \
} while(0)

static int destroy_count = 0;
static void count_destroy(void *ptr) {
    (void)ptr; /* Suppress unused parameter warning */
    destroy_count++;
}

void create_hash_table_with_valid_capacity(void) {
    hash_table_t *ht = hash_create(16);
    assert(ht != NULL);
    assert(hash_size(ht) == 0);
    hash_destroy(ht, NULL);
}

void create_hash_table_with_non_power_of_two_capacity(void) {
    hash_table_t *ht = hash_create(10);
    assert(ht != NULL);
    assert(hash_size(ht) == 0);
    hash_destroy(ht, NULL);
}

void create_hash_table_with_small_capacity(void) {
    hash_table_t *ht = hash_create(2);
    assert(ht != NULL);
    assert(hash_size(ht) == 0);
    hash_destroy(ht, NULL);
}

void put_single_key_value_pair(void) {
    hash_table_t *ht = hash_create(16);
    int value = 42;
    int result = hash_put(ht, "key1", &value);
    assert(result == 0);
    assert(hash_size(ht) == 1);
    hash_destroy(ht, NULL);
}

void put_multiple_key_value_pairs(void) {
    hash_table_t *ht = hash_create(16);
    int value1 = 1, value2 = 2, value3 = 3;
    hash_put(ht, "key1", &value1);
    hash_put(ht, "key2", &value2);
    hash_put(ht, "key3", &value3);
    assert(hash_size(ht) == 3);
    hash_destroy(ht, NULL);
}

void put_with_null_hash_table(void) {
    int value = 42;
    int result = hash_put(NULL, "key", &value);
    assert(result == -1);
}

void put_with_null_key(void) {
    hash_table_t *ht = hash_create(16);
    int value = 42;
    int result = hash_put(ht, NULL, &value);
    assert(result == -1);
    hash_destroy(ht, NULL);
}

void put_update_existing_key(void) {
    hash_table_t *ht = hash_create(16);
    int value1 = 42, value2 = 100;
    int *retrieved;
    hash_put(ht, "key1", &value1);
    {
        int result = hash_put(ht, "key1", &value2);
        assert(result == 0);
    }
    assert(hash_size(ht) == 1);
    retrieved = (int*)hash_get(ht, "key1");
    assert(*retrieved == 100);
    hash_destroy(ht, NULL);
}

void get_existing_key(void) {
    hash_table_t *ht = hash_create(16);
    int value = 42;
    int *retrieved;
    hash_put(ht, "key1", &value);
    retrieved = (int*)hash_get(ht, "key1");
    assert(retrieved != NULL);
    assert(*retrieved == 42);
    hash_destroy(ht, NULL);
}

void get_non_existing_key(void) {
    hash_table_t *ht = hash_create(16);
    void *result = hash_get(ht, "nonexistent");
    assert(result == NULL);
    hash_destroy(ht, NULL);
}

void get_from_empty_hash_table(void) {
    hash_table_t *ht = hash_create(16);
    void *result = hash_get(ht, "key1");
    assert(result == NULL);
    hash_destroy(ht, NULL);
}

void remove_existing_key(void) {
    hash_table_t *ht = hash_create(16);
    int value = 42;
    int result;
    hash_put(ht, "key1", &value);
    result = hash_remove(ht, "key1", NULL);
    assert(result == 0);
    assert(hash_size(ht) == 0);
    assert(hash_get(ht, "key1") == NULL);
    hash_destroy(ht, NULL);
}

void remove_non_existing_key(void) {
    hash_table_t *ht = hash_create(16);
    int result = hash_remove(ht, "nonexistent", NULL);
    assert(result == -1);
    hash_destroy(ht, NULL);
}

void remove_with_null_hash_table(void) {
    int result = hash_remove(NULL, "key", NULL);
    assert(result == -1);
}

void remove_with_null_key(void) {
    hash_table_t *ht = hash_create(16);
    int result = hash_remove(ht, NULL, NULL);
    assert(result == -1);
    hash_destroy(ht, NULL);
}

void remove_from_chain_head(void) {
    hash_table_t *ht = hash_create(2);
    int value1 = 1, value2 = 2;
    int result;
    hash_put(ht, "a", &value1);
    hash_put(ht, "c", &value2);
    result = hash_remove(ht, "c", NULL);
    assert(result == 0);
    assert(hash_size(ht) == 1);
    assert(hash_get(ht, "a") != NULL);
    hash_destroy(ht, NULL);
}

void remove_from_chain_middle(void) {
    hash_table_t *ht = hash_create(2);
    int value1 = 1, value2 = 2, value3 = 3;
    int result;
    hash_put(ht, "a", &value1);
    hash_put(ht, "c", &value2);
    hash_put(ht, "e", &value3);
    result = hash_remove(ht, "c", NULL);
    assert(result == 0);
    assert(hash_size(ht) == 2);
    assert(hash_get(ht, "a") != NULL);
    assert(hash_get(ht, "e") != NULL);
    hash_destroy(ht, NULL);
}

void destroy_null_hash_table(void) {
    hash_destroy(NULL, NULL);
}

void destroy_empty_hash_table(void) {
    hash_table_t *ht = hash_create(16);
    hash_destroy(ht, NULL);
}

void size_of_null_hash_table(void) {
    size_t size = hash_size(NULL);
    assert(size == 0);
}

void size_of_empty_hash_table(void) {
    hash_table_t *ht = hash_create(16);
    assert(hash_size(ht) == 0);
    hash_destroy(ht, NULL);
}

void handle_hash_collisions(void) {
    hash_table_t *ht = hash_create(2);
    int value1 = 1, value2 = 2, value3 = 3;
    hash_put(ht, "a", &value1);
    hash_put(ht, "c", &value2);
    hash_put(ht, "e", &value3);
    assert(hash_size(ht) == 3);
    assert(*(int*)hash_get(ht, "a") == 1);
    assert(*(int*)hash_get(ht, "c") == 2);
    assert(*(int*)hash_get(ht, "e") == 3);
    hash_destroy(ht, NULL);
}

void destroy_with_callback_function(void) {
    hash_table_t *ht = hash_create(16);
    int value1 = 1, value2 = 2;
    destroy_count = 0;
    hash_put(ht, "key1", &value1);
    hash_put(ht, "key2", &value2);
    hash_destroy(ht, count_destroy);
    assert(destroy_count == 2);
}

void remove_with_callback_function(void) {
    hash_table_t *ht = hash_create(16);
    int value = 42;
    destroy_count = 0;
    hash_put(ht, "key1", &value);
    hash_remove(ht, "key1", count_destroy);
    assert(destroy_count == 1);
    hash_destroy(ht, NULL);
}

void store_and_retrieve_string_values(void) {
    hash_table_t *ht = hash_create(16);
    char *str1 = "Hello";
    char *str2 = "World";
    hash_put(ht, "greeting", str1);
    hash_put(ht, "target", str2);
    assert(strcmp((char*)hash_get(ht, "greeting"), "Hello") == 0);
    assert(strcmp((char*)hash_get(ht, "target"), "World") == 0);
    hash_destroy(ht, NULL);
}

void store_null_values(void) {
    hash_table_t *ht = hash_create(16);
    hash_put(ht, "null_key", NULL);
    assert(hash_get(ht, "null_key") == NULL);
    assert(hash_size(ht) == 1);
    hash_destroy(ht, NULL);
}

int main(void) {
    printf("Running hash table tests...\n");

    RUN_TEST(create_hash_table_with_valid_capacity);
    RUN_TEST(create_hash_table_with_non_power_of_two_capacity);
    RUN_TEST(create_hash_table_with_small_capacity);
    RUN_TEST(put_single_key_value_pair);
    RUN_TEST(put_multiple_key_value_pairs);
    RUN_TEST(put_with_null_hash_table);
    RUN_TEST(put_with_null_key);
    RUN_TEST(put_update_existing_key);
    RUN_TEST(get_existing_key);
    RUN_TEST(get_non_existing_key);
    RUN_TEST(get_from_empty_hash_table);
    RUN_TEST(remove_existing_key);
    RUN_TEST(remove_non_existing_key);
    RUN_TEST(remove_with_null_hash_table);
    RUN_TEST(remove_with_null_key);
    RUN_TEST(remove_from_chain_head);
    RUN_TEST(remove_from_chain_middle);
    RUN_TEST(destroy_null_hash_table);
    RUN_TEST(destroy_empty_hash_table);
    RUN_TEST(size_of_null_hash_table);
    RUN_TEST(size_of_empty_hash_table);
    RUN_TEST(handle_hash_collisions);
    RUN_TEST(destroy_with_callback_function);
    RUN_TEST(remove_with_callback_function);
    RUN_TEST(store_and_retrieve_string_values);
    RUN_TEST(store_null_values);
    printf("All tests passed!\n");
    return 0;
}