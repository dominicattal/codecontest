#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>

#define CORRECT      0
#define WRONG        1
#define SERVER_ERROR 2

// argv[1] = testcase #
// argv[2] = file containing test
int main(int argc, char** argv)
{
    FILE* correct_file = NULL;
    FILE* test_file = NULL;
    char* exe_dirname;
    char test_file_path[512];
    char correct_file_path[512];
    int testcase;
    int test_res, correct_res;
    int test_num, correct_num;
    int result;
    int n;
    char c;

    result = SERVER_ERROR;

    if (argc != 2) {
        fprintf(stderr, "validator: incorrect number of arguments\n");
        goto cleanup;
    }

    exe_dirname = dirname(argv[0]);
    testcase = atoi(argv[1]); 
    sprintf(test_file_path, "%s/cases/%d.in", exe_dirname, testcase);
    test_file = fopen(test_file_path, "r");
    if (test_file == NULL) {
        fprintf(stderr, "validator: couldnt read %s\n", test_file_path);
        goto cleanup;
    }

    sprintf(correct_file_path, "%s/cases/%d.ans", exe_dirname, testcase);
    correct_file = fopen(correct_file_path, "r");
    if (correct_file == NULL) {
        fprintf(stderr, "validator: couldnt read %s\n", correct_file_path);
        goto cleanup;
    }
    test_res = fscanf(test_file, "%d", &n);
    if (test_res == EOF) {
        fprintf(stderr, "validator: nothing in test file\n");
        goto cleanup;
    }

    result = WRONG;

    fprintf(stdout, "%d\n", n);
    fflush(stdout);
    correct_res = fscanf(correct_file, "%d", &correct_num);
    while (correct_res != EOF) {
        test_res = scanf("%d", &test_num);
        if (test_num != correct_num)
            goto cleanup;
        correct_res = fscanf(correct_file, "%d", &correct_num);
    }

    // hacky way to test if client sends more text than required, 
    // i couldn't figure out a better way to do this
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
    do { c = fgetc(stdin); } while (isspace(c));
    if (c != EOF) 
        goto cleanup;

    result = CORRECT;

cleanup:
    if (test_file != NULL) 
        fclose(test_file);
    if (correct_file != NULL) 
        fclose(correct_file);
    return result;
}
