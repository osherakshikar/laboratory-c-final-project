#ifndef MACRO_H
#define MACRO_H

#include "util_vec.h"
#include "util_hash.h"
#include "globals.h"
#include <stddef.h>

#define mcro "mcro"  /* Macro start */
#define mcrend "mcrend"

/*
 * =====================================================================================
 * Filename:  macro.h
 * Description: Defines the data structure for a macro, which includes its name
 * and a dynamic vector to store the lines of its body.
 * =====================================================================================
 */

/**
 * @struct macro_t
 * @brief Represents a single macro definition.
 */
typedef struct {
    char *name;     /* The name of the macro */
    vec_t body;     /* A dynamic vector (vec_t) to store the lines (char*) of the macro's body */
} macro_t;


int preprocess_file(const char *input_path, const char *output_path);
#endif /* MACRO_H */
