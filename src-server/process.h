#ifndef PROCESS_H
#define PROCESS_H

#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>

typedef enum {
    PROCESS_SUCCESS  = 0,
    PROCESS_FAILED   = 1,
    PROCESS_ERROR    = 2
} ProcessEnum;

typedef struct Process {
    char** argv;
    pthread_t wait_thread;
    pid_t pid;
    ProcessEnum exit_status;
    int status;
    int fd_in, fd_out;
    bool running;
} Process;

typedef struct {
    Process* process1;
    Process* process2;
} ProcessPair;

Process* process_create(const char* command, FILE* infile, FILE* outfile, FILE* errfile);
ProcessPair process_pair_create(const char* command1, const char* command2);
void process_wait(Process* process);
size_t process_memory(Process* process);
void process_destroy(Process* process);
bool process_success(Process* process);
bool process_failed(Process* process);
bool process_error(Process* process);

#endif
