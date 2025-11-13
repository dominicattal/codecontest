#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>

#define CORRECT      0
#define WRONG        1
#define SERVER_ERROR 2

// argv[1] = testcase #
// argv[2] = file containing test
int main(int argc, char** argv)
{
    FILE* correct_file;
    FILE* test_file;
    char correct_file_path[512];
    int testcase;
    int test_res, correct_res;
    int test_num, correct_num;
    int result;

    if (argc != 3) 
        return SERVER_ERROR;

    testcase = atoi(argv[1]); 
    test_file = fopen(argv[2], "r");
    if (test_file == NULL) {
        return SERVER_ERROR;
    }

    sprintf(correct_file_path, "%s\\cases\\%d.ans", dirname(argv[0]), testcase);
    correct_file = fopen(correct_file_path, "r");
    if (correct_file == NULL) {
        fclose(test_file);
        return SERVER_ERROR;
    }

    result = CORRECT;
    test_res = fscanf(test_file, "%d", &test_num);
    correct_res = fscanf(correct_file, "%d", &correct_num);
    while (test_res != EOF && correct_res != EOF) {
        if (test_num != correct_num) {
            result = WRONG;
            break;
        }
        test_res = fscanf(test_file, "%d", &test_num);
        correct_res = fscanf(correct_file, "%d", &correct_num);
    }

    if (test_res != correct_res)
        result = WRONG;

    fclose(test_file);
    fclose(correct_file);
    return result;
}
