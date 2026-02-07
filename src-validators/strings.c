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

int get_next_string_length(FILE* file)
{
    long pos;
    int length = 0;
    char c = fgetc(file);
    while (isspace(c) && c != EOF)
        c = fgetc(file);
    if (c == EOF)
        return EOF;
    ungetc(c, file);
    pos = ftell(file);
    while (!isspace(c) && c != EOF) {
        length++;
        c = fgetc(file);
    }
    fseek(file, pos, SEEK_SET);
    return length-1;
}

char* get_next_string(FILE* file, int length)
{
    char* buf = malloc((length+1) * sizeof(char));
    for (int i = 0; i < length; i++)
        buf[i] = fgetc(file);
    buf[length] = '\0';
    return buf;
}

// argv[1] = case path
// argv[2] = output path
int main(int argc, char** argv)
{
    FILE* correct_file = NULL;
    FILE* test_file = NULL;
    char* test_file_path;
    char* correct_file_path;
    int test_res, correct_res;
    char* test_string;
    char* correct_string;
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
        test_string_length = get_next_string_length(test_file);
        correct_string_length = get_next_string_length(correct_file);
        if (test_string_length != correct_string_length)
            goto cleanup;
        if (test_string_length == EOF)
            break;
        test_string = get_next_string(test_file, test_string_length);
        correct_string = get_next_string(correct_file, correct_string_length);
        cmp_result = strcmp(correct_string, test_string);
        free(test_string);
        free(correct_string);
        if (cmp_result != 0)
            goto cleanup;
    } while (1);

    result = CORRECT;

cleanup:
    if (correct_file != NULL) 
        fclose(correct_file);
    if (test_file != NULL)
        fclose(test_file);
    return result;
}
