#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/macro.h"
#include "../include/util_hash.h"
#include "../include/util_vec.h"
#include "../include/globals.h"

/*
 * =====================================================================================
 * Filename:  preprocessor.c
 * Description: Preprocessor for assembly-like files that handles macro definitions.
 * This preprocessor reads an input file, processes macro definitions, and expands
 * macro calls in the output file.
 * It uses a hash table to store macro definitions and a dynamic vector to store
 * the lines of each macro's body.
 * =====================================================================================
 */

/* --- Private Helper Functions --- */

/* Checks if a given name is a reserved keyword.
 * return TRUE if the name is reserved, FALSE otherwise.
 */
static bool_t is_reserved_keyword(const char* name) {
    int i;
    const char* reserved_keywords[] = {
        "mov", "cmp", "add", "sub", "not", "clr", "lea", "inc", "dec",
        "jmp", "bne", "red", "prn", "jsr", "rts", "stop",
        "data", "string", "mat", "entry", "extern",
        "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
        "mcro", "endmcro",
        NULL /* Sentinel to mark the end of the array */
    };
    for (i = 0; reserved_keywords[i] != NULL; i++) {
        if (strcmp(reserved_keywords[i], name) == 0) {
            return TRUE;
        }
    }
    return FALSE;
}

/* Creates a new, empty macro object.
 * Returns a pointer to the newly created macro_t object, or NULL on failure.
 */
static macro_t* create_macro(const char* name) {
    macro_t* macro = malloc(sizeof(macro_t));
    if (!macro) return NULL;

    macro->name = dupstr(name);
    if (!macro->name) {
        free(macro);
        return NULL;
    }
    vec_create(&macro->body, sizeof(char*));
    return macro;
}

/* Frees all memory associated with a macro object.
 */
static void destroy_macro(void* m) {
    macro_t* macro = m;
    size_t i;
    if (!macro) return;

    free(macro->name);

    /* Free each line stored in the body vector */
    for (i = 0; i < macro->body.len; i++) {
        char* line = *(char**)vec_get(&macro->body, i);
        free(line);
    }

    vec_destroy(&macro->body);
    free(macro);
}

/* Adds a line of text to the macro's body.
 * Returns 0 on success, -1 on failure.
 */
static int add_line_to_macro(macro_t* m, const char* line) {
    char* line_copy;
    if (!m || !line) return -1;

    line_copy = dupstr(line);
    if (!line_copy) return -1;

    if (vec_push(&m->body, &line_copy) != 0) {
        free(line_copy);
        return -1;
    }
    return 0;
}

/* --- Main Preprocessor Function --- */

int preprocess_file(const char *input_path, const char *output_path) {
    FILE *as_file, *am_file;
    char line[MAX_LINE_LENGTH ];
    char line_copy[MAX_LINE_LENGTH ];
    int line_num = 0;
    bool_t success = TRUE;

    hash_table_t *macro_table;
    bool_t in_macro_definition = FALSE;
    macro_t *current_macro = NULL;

    char *token;
    char *macro_name;
    macro_t *macro_to_expand;
    size_t i;

    macro_table = hash_create(0); /* Use default capacity */
    if (!macro_table) {
        printf("Error: Failed to create macro table.\n");
        return -1;
    }

    as_file = fopen(input_path, "r");
    if (!as_file) {
        printf("Error: Cannot open input file '%s'\n", input_path);
        hash_destroy(macro_table, destroy_macro);
        return -1;
    }

    am_file = fopen(output_path, "w");
    if (!am_file) {
        fprintf(stderr, "Error: Cannot open output file '%s'\n", output_path);
        fclose(as_file);
        hash_destroy(macro_table, destroy_macro);
        return -1;
    }

    /* Read the input file line by line and process it.*/
    while (fgets(line, sizeof(line), as_file)) {
        line_num++;
        strcpy(line_copy, line); /* strtok modifies the string, so we use a copy */

        token = strtok(line_copy, " \t\n\r");
        if (!token) {
            /* Empty or whitespace-only line */
            if (!in_macro_definition) {
                fputs(line, am_file);
            }
            continue;
        }
        /* Check for macro definition or end */
        if (strcmp(token, mcro) == 0) {
            in_macro_definition = TRUE;

            macro_name = strtok(NULL, " \t\n\r");
            if (!macro_name) {
                printf("Error line %d: Missing macro name after 'mcro'.\n", line_num);
                success = FALSE;
                continue;
            }
            if (is_reserved_keyword(macro_name)) {
                printf("Error line %d: Macro name '%s' is a reserved keyword.\n", line_num, macro_name);
                success = FALSE;
                continue;
            }
            if (strtok(NULL, " \t\n\r") != NULL) {
                printf("Error line %d: Token found after macro definition.\n", line_num);
                success = FALSE;
                continue;
            }

            current_macro = create_macro(macro_name);
            hash_put(macro_table, macro_name, current_macro);
        } else if (strcmp(token, mcrend) == 0) {
            if (strtok(NULL, " \t\n\r") != NULL) {
                printf("Error line %d: Token found after 'mcrend'.\n", line_num);
                success = FALSE;
            }
            in_macro_definition = FALSE;
            current_macro = NULL;
        } else if (in_macro_definition) {
            add_line_to_macro(current_macro, line);
        } else {
            /* Not in a macro definition, check for macro call */
            macro_to_expand = hash_get(macro_table, token);
            if (macro_to_expand) {
                for (i = 0; i < macro_to_expand->body.len; i++) {
                    char *macro_line = *(char **) vec_get(&macro_to_expand->body, i); /* Get the line from the macro body */
                    fputs(macro_line, am_file);
                }
            } else {
                /* Regular line, write to output */
                fputs(line, am_file);
            }
        }
    }

    fclose(as_file);
    fclose(am_file);
    hash_destroy(macro_table, destroy_macro);

    if (!success) {
        remove(output_path); /* Delete the .am file if errors occurred */
        return -1;
    }

    return 0;
}
