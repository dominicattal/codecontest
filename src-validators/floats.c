#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <unistd.h>
#include <math.h>
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
    double test_num, correct_num;
    int result;
    int n;
    char c;

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
        correct_res = fscanf(correct_file, "%lf", &correct_num);
        test_res = fscanf(test_file, "%lf", &test_num);
        if (fabs(test_num - correct_num) > 1e-6)
            goto cleanup;
    } while (correct_res != EOF && test_res != EOF);

    if (correct_res != EOF || test_res != EOF)
        goto cleanup;

    result = CORRECT;

cleanup:
    if (correct_file != NULL) 
        fclose(correct_file);
    if (test_file != NULL)
        fclose(test_file);
    return result;
}
