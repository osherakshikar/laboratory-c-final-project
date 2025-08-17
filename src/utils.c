#include <stdlib.h>
#include <string.h>
#include "../include/globals.h"


char *dupstr(const char *str) {
    char *dup = malloc(strlen(str) + 1);
    if (dup) {
        strcpy(dup, str);
    }
    return dup;
}

bool_t is_reserved_keyword(const char* name) {
    int i;
    const char* reserved_keywords[] = {
        "mov", "cmp", "add", "sub", "not", "clr", "lea", "inc", "dec",
        "jmp", "bne", "red", "prn", "jsr", "rts", "stop",
        ".data", ".string", ".mat", ".entry", ".extern",
        "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
        "mcro", "mcrend",
        NULL /* sentinel to mark the end of the array */
    };
    for (i = 0; reserved_keywords[i] != NULL; i++) {
        if (strcmp(reserved_keywords[i], name) == 0) {
            return TRUE;
        }
    }
    return FALSE;
}

char *create_file_path(const char *file_name, const char *ending) {
    char *c, *new_file_name;
    new_file_name = malloc(strlen(file_name) + strlen(ending) + 1);
    if (!new_file_name) {
        return NULL; /* memory allocation failed */
    }
    strcpy(new_file_name, file_name);
    /* deleting the file name if a '.' exists and forth */
    if ((c = strchr(new_file_name, '.')) != NULL) {
        *c = '\0';
    }
    /* adds the ending of the new file name */
    strcat(new_file_name, ending);
    return new_file_name;
}