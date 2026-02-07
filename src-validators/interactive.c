#include <stdio.h>
#include <string.h>

#define CORRECT      0
#define WRONG        1
#define SERVER_ERROR 2

#define LINE_BUFFER_SIZE 4096

// argv[1] = case path (name).in
int main(int argc, char** argv)
{
    FILE* case_file = NULL;
    const char* case_file_path;
    char line[LINE_BUFFER_SIZE];
    char* res;
    int n, guess;

    if (argc != 2) {
        fprintf(stderr, "validator: incorrect number of args\n");
        return SERVER_ERROR;
    }

    case_file_path = argv[1];
    case_file = fopen(case_file_path, "r");
    fscanf(case_file, "%d", &n);
    fclose(case_file);

    while (1) {
        res = fgets(line, LINE_BUFFER_SIZE-1, stdin);
        if (res == NULL) {
            fprintf(stderr, "validator: unexpected EOF\n");
            return SERVER_ERROR;
        } 
        if (line[0] == '?') {
            sscanf(line, "? %d", &guess);
            if (guess < n)
                puts("higher");
            else if (guess > n)
                puts("lower");
            else
                puts("equal");
            fflush(stdout);
        } else if (line[0] == '!') {
            sscanf(line, "! %d", &guess);
            if (guess == n)
                break;
            return WRONG;
        } else {
            return WRONG;
        }
    }

    return CORRECT;
}
