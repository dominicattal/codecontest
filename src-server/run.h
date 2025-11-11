#ifndef PROBLEM_H
#define PROBLEM_H

#include <semaphore.h>

typedef enum {
    RUN_IDLE,
    RUN_ENQUEUED,
    RUN_RUNNING,
    RUN_SUCCESS,
    RUN_FAILED
} RunEnum;

typedef struct Run Run;

typedef struct Run {
    const char* language;
    const char* code;
    char* response;
    int response_length;
    int problem_id;
    RunEnum status;
    Run* next;
} Run;

typedef struct {
    Run* head;
    Run* tail;
} RunQueue;

Run* run_create(const char* language, const char* code);
void run_enqueue(Run* run);
void run_wait(Run* run);
void run_destroy(Run* run);

#endif
