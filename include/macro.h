#ifndef MACRO_H
#define MACRO_H

#include "util_vec.h"

/*
 * =====================================================================================
 * Filename:  macro.h
 * Description: Defines the data structure for a macro, which includes its name
 * and a dynamic vector to store the lines of its body.
 * =====================================================================================
 */

#define mcro "mcro"  /* Macro start */
#define mcrend "mcrend"
/**
 * @struct macro_t
 * @brief Represents a single macro definition.
 */
typedef struct {
    char *name;     /* The name of the macro */
    vec_t body;     /* A dynamic vector (vec_t) to store the lines (char*) of the macro's body */
} macro_t;

/**
 * @brief Preprocesses an assembly-like file, expanding macros and writing the result to an output file.
 *
 * @param input_path The path to the input file containing macro definitions.
 * @param output_path The path to the output file where the processed content will be written.
 * @return int Returns 0 on success, or -1 on failure.
 */
int preprocess_file(const char *input_path, const char *output_path);
#endif /* MACRO_H */
