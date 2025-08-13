#include <stdio.h>
#include <stdlib.h>
#include "../include/macro.h"
#include "../include/symbol_table.h"
#include "../include/first_pass.h"
#include "../include/second_pass.h"

/**
 * This is the main function that processes the input files.
 *
 * @param argc The number of command-line arguments.
 * @param argv An array of strings containing the command-line arguments.
 * @return Returns 0 on successful completion.
 */
int main(int argc, char *argv[]) {
    int i;
    char *base_name;
    char *input_path = NULL;
    char *output_path = NULL;
    symbol_table_t *symbol_table = NULL;

    printf("%d \n", argc);

    /* In main, after parsing command-line arguments */
    for (i = 1; i < argc; i++) {
        base_name = argv[i];

        /* Example: base_name = "ps" -> input_path = "ps.as", output_path = "ps.am" */
        input_path = create_file_path(base_name, ".as");
        output_path = create_file_path(base_name, ".am");

        if (!input_path || !output_path) {
            print_error(ERROR_CANNOT_OPEN_FILE);
            free(input_path);
            free(output_path);
            continue;
        }

        /* 2. Call the pre-processor */
        printf("Processing file: %s\n", input_path);
        if (preprocess_file(input_path, output_path) == 0) {
            printf("Pre-processing successful. Output file: %s\n", output_path);
        } else {
            print_error(ERROR_FAILED_PREPROCESSING);
            return 1;
        }

        /* 3. Call the first pass */
        input_path = output_path;
        output_path = create_file_path(base_name, ".ob");
        if (!output_path) {
            print_error(ERROR_CANNOT_OPEN_FILE);
            free(input_path);
            continue;
        }
        printf("Starting first pass on: %s\n", input_path);
        symbol_table = symtab_create();
        if (!symbol_table) {
            print_error(ERROR_MEMORY_ALLOCATION_FAILED); /* TODO: ERROR */
            free(input_path);
            free(output_path);
            continue;
        }

        if (first_pass(input_path, symbol_table) == 0) {
            printf("First pass completed successfully.\n");

            /* 4. Call the second pass */
            printf("Starting second pass...\n");
            if (second_pass(input_path, base_name, symbol_table) == 0) {
                printf("Second pass completed successfully. Generated files:\n");
                printf("  - %s.ob (machine code)\n", base_name);
                printf("  - %s.ent (entry symbols, if any)\n", base_name);
                printf("  - %s.ext (external symbols, if any)\n", base_name);
            } else {
                print_error(ERROR_WRITE_FAILED);
                continue;
            }
        } else {
            print_error(ERROR_FIRST_PASSED); /* TODO: Better error code */
            continue;
        }

        /* Clean up symbol table */
        symtab_destroy(symbol_table);

        /* TODO: Continue to the next stages (second pass, etc.) with the .am file */
        printf("Processed file: %s\n", input_path);
        free(input_path);
        free(output_path);
    }
    free(input_path);
    free(output_path);
    printf("end\n");
    return 0;
}
