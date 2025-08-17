#include <stdio.h>
#include <stdlib.h>
#include "../include/macro.h"
#include "../include/symbol_table.h"
#include "../include/second_pass.h"
#include "../include/errors.h"
int main(int argc, char *argv[]) {
    int i;
    int overall_result = 0;
    char *as_path;
    char *am_path;
    symbol_table_t *symbol_table;

    if (argc < 2) {
        print_error(ERROR_CANNOT_OPEN_FILE);
        printf("Usage: %s <file1> <file2> ... <fileN>\n", argv[0]);
        return 1;
    }

    for (i = 1; i < argc; i++) {
        as_path = NULL;
        am_path = NULL;
        symbol_table = NULL;

        /* create file paths */
        as_path = create_file_path(argv[i], ".as");
        am_path = create_file_path(argv[i], ".am");

        if (!as_path || !am_path) {
            print_error(ERROR_CANNOT_OPEN_FILE);
            if (as_path) free(as_path);
            if (am_path) free(am_path);
            overall_result = 1;
            printf("Failed to process file: %s\n", argv[i]);
            continue;
        }

        /* preprocessing */
        printf("Processing file: %s\n", as_path);
        if (preprocess_file(as_path, am_path) != 0) {
            print_error(ERROR_FAILED_PREPROCESSING);
            free(as_path);
            free(am_path);
            overall_result = 1;
            printf("Failed to process file: %s\n", argv[i]);
            continue;
        }
        printf("Pre-processing successful. Output file: %s\n", am_path);

        /* first pass */
        printf("Starting first pass on: %s\n", am_path);
        symbol_table = symtab_create();
        if (!symbol_table) {
            print_error(ERROR_MEMORY_ALLOCATION_FAILED);
            free(as_path);
            free(am_path);
            overall_result = 1;
            printf("Failed to process file: %s\n", argv[i]);
            continue;
        }

        if (first_pass(am_path, symbol_table) != 0) {
            print_error(ERROR_FIRST_PASSED);
            free(as_path);
            free(am_path);
            symtab_destroy(symbol_table);
            overall_result = 1;
            printf("Failed to process file: %s\n", argv[i]);
            continue;
        }
        printf("First pass completed successfully.\n");

        /* second pass */
        printf("Starting second pass on: %s\n", am_path);
        if (second_pass(am_path, argv[i], symbol_table) != 0) {
            print_error(ERROR_WRITE_FAILED);
            free(as_path);
            free(am_path);
            symtab_destroy(symbol_table);
            overall_result = 1;
            printf("Failed to process file: %s\n", argv[i]);
            continue;
        }

        printf("Second pass completed successfully\n");
        /* clean up resources for this file */
        free(as_path);
        free(am_path);
        symtab_destroy(symbol_table);

        printf("Processed file: %s\n", argv[i]);
    }

    printf("Assembly complete\n");
    return overall_result;
}
