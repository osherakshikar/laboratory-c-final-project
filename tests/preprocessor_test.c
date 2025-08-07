#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Include the header for the function we are testing */
#include "../include/macro.h"
/* We also need the type definitions */
#include "../include/globals.h"

/* --- Test Runner Helper Functions --- */

/* Helper function to create a temporary file with specific content */
void create_test_file(const char* filename, const char* content) {
    FILE* fp = fopen(filename, "w");
    if (fp) {
        fputs(content, fp);
        fclose(fp);
    }
}

/* Helper function to read the content of a file into a string */
char* read_file_content(const char* filename) {
    FILE* fp = fopen(filename, "r");
    char* content = NULL;
    long length;

    if (fp) {
        fseek(fp, 0, SEEK_END);
        length = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        content = malloc(length + 1);
        if (content) {
            fread(content, 1, length, fp);
            content[length] = '\0';
        }
        fclose(fp);
    }
    return content;
}

/* The main test runner function */
void run_test(const char* test_name, const char* input_content, const char* expected_output, int expected_return) {
    const char* input_filename = "test_input.as";
    const char* output_filename = "test_output.am";
    int return_value;
    char* actual_output;

    printf("Running test: %s... ", test_name);

    /* 1. Set up the test environment */
    create_test_file(input_filename, input_content);

    /* 2. Run the function being tested */
    return_value = preprocess_file(input_filename, output_filename);

    /* 3. Check the results */
    if (return_value != expected_return) {
        printf("FAIL (Expected return %d, got %d)\n", expected_return, return_value);
    } else {
        if (expected_return == 0) { /* Success case */
            actual_output = read_file_content(output_filename);
            if (actual_output && strcmp(actual_output, expected_output) == 0) {
                printf("PASS\n");
            } else {
                printf("FAIL (Output mismatch)\n");
                printf("Expected:\n---\n%s\n---\n", expected_output);
                printf("Got:\n---\n%s\n---\n", actual_output ? actual_output : "NULL");
            }
            free(actual_output);
        } else { /* Error case */
            /* Check that the output file was not created (or was removed) */
            FILE* fp = fopen(output_filename, "r");
            if (fp) {
                fclose(fp);
                printf("FAIL (Output file was created on error)\n");
            } else {
                printf("PASS (Function failed as expected)\n");
            }
        }
    }

    /* 4. Clean up */
    remove(input_filename);
    remove(output_filename);
}


/* --- Main Function - Test Cases --- */

int main() {
    printf("--- Starting Preprocessor Tests ---\n");

    /* Test Case 1: Simple macro expansion */
    run_test(
        "Simple Expansion",
        "mcro my_inc\ninc r1\nendmcro\nmy_inc\n",
        "inc r1\n",
        0
    );

    /* Test Case 2: No macros in file */
    run_test(
        "No Macros",
        "mov r1, r2\nadd #5, r3\n",
        "mov r1, r2\nadd #5, r3\n",
        0
    );

    /* Test Case 3: Error - macro name is a reserved word */
    run_test(
        "Reserved Word Error",
        "mcro mov\nsub r1, r1\nendmcro\n",
        NULL, /* We don't expect any output */
        -1    /* We expect the function to fail */
    );

    /* Test Case 4: Error - extraneous text after mcro definition */
    run_test(
        "Extraneous text after mcro",
        "mcro my_macro some_junk\ninc r1\nendmcro\n",
        NULL,
        -1
    );

    /* Test Case 5: Multiple macros and comments */
    run_test(
        "Multiple Macros and Comments",
        "; This is a test file\n\nmcro setup\nmov #1, r1\nendmcro\n\nmcro teardown\nmov #0, r1\nendmcro\n\nsetup\n; some action\nteardown\n",
        "; This is a test file\n\nmov #1, r1\n; some action\nmov #0, r1\n",
        0
    );

    printf("--- Preprocessor Tests Finished ---\n");

    return 0;
}
