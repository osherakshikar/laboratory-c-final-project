#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/macro.h"

/**
 * @brief Checks if a given name is a reserved keyword.
 * @param name The name to check.
 * @return TRUE if the name is reserved, FALSE otherwise.
 */
static bool_t is_reserved_keyword(const char* name) {
    int i;
    /* An array of all reserved keywords in the assembly language. */
    static const char* reserved_keywords[] = {
        /* Instructions */
        "mov", "cmp", "add", "sub", "not", "clr", "lea", "inc", "dec",
        "jmp", "bne", "red", "prn", "jsr", "rts", "stop",
        /* Directives (without the dot) */
        "data", "string", "mat", "entry", "extern",
        /* Register names */
        "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
        /* Macro definition keywords */
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

/*
 * Creates a new, empty macro object.
 * name The name of the macro. The function will create a copy of this string.
 * A pointer to the newly created macro_t object, or NULL on failure.
 */
static macro_t *create_macro(const char *name) {
    macro_t *macro = (macro_t *) malloc(sizeof(macro_t));
    if (!macro) {
        return NULL; /* Memory allocation failed */
    }
    macro->name = dupstr(name); /* Duplicate the name string */
    if (!macro->name) {
        free(macro); /* Free the macro object if name allocation fails */
        return NULL; /* Memory allocation failed */
    }
    vec_create(&macro->body, sizeof(char *)); /* Initialize the body vector */
    if (macro->body.data == NULL) {
        free(macro->name); /* Free the name if vector creation fails */
        free(macro); /* Free the macro object */
        return NULL; /* Memory allocation failed */
    }
    return macro; /* Return the newly created macro object */
}


/*
 * Frees all memory associated with a macro object.
 * This includes the name and all the lines stored in its body vector.
 * m A pointer to the macro_t object to be destroyed.
 */
void destroy_macro(void *m);

/*
 * Adds a line of text to the macro's body.
 *  m A pointer to the macro_t object.
 *  line The line of text to add. The function will create a copy.
 */
int add_line_to_macro(macro_t *m, const char *line);

/*
 * @brief Preprocesses a file by expanding macros defined in it.
 * This function reads an input file, expands any macros found, and writes the
 * result to an output file.
 * @param input_path The path to the input file containing macro definitions.
 * @param output_path The path to the output file where the preprocessed content will be written.
 * @return 0 on success, -1 on failure.
 */
int preprocess_file(const char *input_path, const char *output_path);


char *expand_macros(char *expanded_line str, hash_table_t *hash_table);

int preprocess_file(const char *input_path, const char *output_path) {
    hash_table_t *macro_table;
    macro_t *current_macro;
    char *expanded_line;
    char macro_name[MAX_LINE_LENGTH];
    char line[MAX_LINE_LENGTH];
    FILE *as_file;
    FILE *am_file;
    bool_t in_macro_definition;

    /* Initialize the macro table and current macro */
    in_macro_definition = FALSE;
    current_macro = NULL;
    macro_table = hash_create(INITIAL_CAPACITY);

    /* Open the input and output files */
    as_file = fopen(input_path, "r");
    if (!as_file) {
        printf("Error: Cannot open input file '%s'\n", input_path);
        return -1;
    }
    am_file = fopen(output_path, "w");
    if (!am_file) {
        printf("Error: Cannot open output file '%s'\n", output_path);
        fclose(as_file);
        return -1;
    }

    while (fgets(line, sizeof(line), as_file)) {
        if (strlen(line) == MAX_LINE_LENGTH && strpbrk(line, "\n") == NULL) {
            fclose(as_file);
            fclose(am_file);
            printf("Error: Line too long, no newline character found.\n");
            return -1; /* Line is too long, no newline character */
        }

        if (strncmp(line, mcro, strlen(mcro)) == 0) {
            /* Start of macro definition */
            in_macro_definition = TRUE;
            strcpy(macro_name, line + strlen(mcro) + 1) ; /* Skip "mcro " */
            current_macro = create_macro(macro_name);
            if (!current_macro) {
                fclose(as_file);
                fclose(am_file);
                return -1; /* Failed to create macro */
            }
        }

        else if (strncmp(line, mcrend, strlen(mcrend)) == 0) {
            /* End of macro definition */
            if (!in_macro_definition || !current_macro) {
                printf("Error: No macro definition to end.\n");
                fclose(as_file);
                fclose(am_file);
                return -1; /* No macro definition to end */
            }
            in_macro_definition = FALSE;
            hash_put(macro_table, current_macro->name, current_macro);
            current_macro = NULL; /* Reset current macro */
        }

        else if (in_macro_definition) {
            /* Inside a macro definition, add line to the current macro */
            if (add_line_to_macro(current_macro, line) != 0) {
                fclose(as_file);
                fclose(am_file);
                return -1; /* Failed to add line to macro */
            }
        }

        else {
            /* Not inside a macro definition, process normally */
            expanded_line = expand_macros(line, macro_table);
            fprintf(am_file, "%s", expanded_line ? expanded_line : line); /* Write expanded or original line */
        }
    }

    return 0; /* Successfully read the file */
}
