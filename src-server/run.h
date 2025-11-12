#ifndef PROBLEM_H
#define PROBLEM_H

#include <semaphore.h>

typedef enum {
    RUN_IDLE,
    RUN_ENQUEUED,
    RUN_RUNNING,
    RUN_SUCCESS,
    RUN_FAILED,
    RUN_SERVER_ERROR
} RunEnum;

typedef struct Run Run;

typedef struct Run {
    const char* language;
    const char* code;
    char* response;
    int code_length;
    int response_length;
    int problem_id;
    int id;
    sem_t signal;
    RunEnum status;
    Run* next;
} Run;

// create new run
Run*    run_create(const char* language, const char* code, int code_length);

// add run to the queue
void    run_enqueue(Run* run);

// block execution until the run finishes
void    run_wait(Run* run);

// free run memory
void    run_destroy(Run* run);

// thread that handles runs. can have several threads active at once
void*   run_daemon(void* vargp);

#endif
