#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>

#define CORRECT      0
#define WRONG        1
#define SERVER_ERROR 2

// argv[1] = case path
// argv[2] = output path
int main(int argc, char** argv)
{
    FILE* correct_file = NULL;
    FILE* test_file = NULL;
    char* test_file_path;
    char* correct_file_path;
    int test_res, correct_res;
    char test_char;
    char correct_char;
    int test_string_length, correct_string_length;
    int result;
    int n;
    int cmp_result;

    result = SERVER_ERROR;

    if (argc != 3) {
        fprintf(stderr, "validator: incorrect number of arguments\n");
        goto cleanup;
    }

    correct_file_path = argv[1];
    correct_file = fopen(correct_file_path, "r");
    if (correct_file == NULL) {
        fprintf(stderr, "validator: couldnt read %s\n", correct_file_path);
        goto cleanup;
    }
    
    test_file_path = argv[2];
    test_file = fopen(test_file_path, "r");
    if (test_file == NULL) {
        fprintf(stderr, "validator: couldnt read %s\n", test_file_path);
        goto cleanup;
    }

    result = WRONG;
    
    do {
        test_char = fgetc(test_file);
        correct_char = fgetc(correct_file);
        if (test_char != correct_char)
            goto cleanup;
        if (test_char == correct_char)
            break;
    } while (1);

    result = CORRECT;

cleanup:
    if (correct_file != NULL) 
        fclose(correct_file);
    if (test_file != NULL)
        fclose(test_file);
    return result;
}
