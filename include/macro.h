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
 * @brief Creates a new macro object.
 *
 * Allocates memory for a macro_t structure and initializes its fields.
 *
 * @param name The name of the macro.
 * @return A pointer to the newly created macro_t object, or NULL on failure.
 */
int preprocess_file(const char *input_path, const char *output_path);
#endif /* MACRO_H */
