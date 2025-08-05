#ifndef TYPES_H
#define TYPES_H
/* * =====================================================================================
 *
 * Filename:  types.h
 *
 * Description: Defines basic types and constants used throughout the assembler.
 *
 * =====================================================================================
 */

/**
 * @enum bool_t
 * @brief Boolean type definition for true/false values.
 */
typedef enum {
    FALSE = 0,
    TRUE = 1
} bool_t;

typedef unsigned short word_t; /* 15‑bit word + ARE bits encoded */

#define MAX_LABEL_LEN 31
#endif
