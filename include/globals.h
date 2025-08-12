#ifndef GLOBALS_H
#define GLOBALS_H
#include <stdlib.h>
#include <string.h>

/**
 * @enum bool_t
 * @brief Boolean type definition for true/false values.
 */
typedef enum {
    FALSE = 0,
    TRUE = 1
} bool_t;

char *dupstr(const char *str);

#ifndef MAX_STRING_LEN
#define MAX_STRING_LEN 80      /* fits any single input line */
#endif

#ifndef MAX_DATA_ITEMS
#define MAX_DATA_ITEMS 32      /* .data numbers per line */
#endif

#ifndef MAX_MATRIX_ROWS
#define MAX_MATRIX_ROWS 16
#endif
#ifndef MAX_MATRIX_COLS
#define MAX_MATRIX_COLS 16
#endif
#define MAX_MATRIX_CELLS (MAX_MATRIX_ROWS * MAX_MATRIX_COLS)

#define MAX_LINE_LENGTH 82 /* 80 chars + newline / terminator */
#define MAX_LABEL_LENGTH  31  /* 30 chars + terminator */

#endif
