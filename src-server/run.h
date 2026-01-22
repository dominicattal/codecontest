#ifndef PROBLEM_H
#define PROBLEM_H

#include <semaphore.h>
#include <stdbool.h>

typedef enum {
    RUN_IDLE,
    RUN_ENQUEUED,
    RUN_COMPILING,
    RUN_RUNNING,
    RUN_SUCCESS,
    RUN_COMPILATION_ERROR,
    RUN_RUNTIME_ERROR,
    RUN_TIME_LIMIT_EXCEEDED,
    RUN_MEM_LIMIT_EXCEEDED,
    RUN_WRONG_ANSWER,
    RUN_SERVER_ERROR,
    RUN_DEAD
} RunEnum;

typedef struct Run Run;

typedef struct Run {
    Run* next;
    const char* filename;
    const char* code;
    char* response;
    sem_t run_to_server_signal;
    sem_t server_to_run_signal;
    RunEnum status;
    int language_id;
    int team_id;
    int problem_id;
    int code_length;
    int response_length;
    int id;
    bool async;
} Run;

// create new run
Run*    run_create(const char* filename, int team_id, int language_id, int problem_id, const char* code, int code_length, bool async);

// add run to the queue
void    run_enqueue(Run* run);

// block execution until run changes state
void    run_wait(Run* run);

// signal run that it can continue executing
void    run_post(Run* run);

// if something goes wrong in cli client handler, safelty stops run
// run must be waited, otherwise will introduce race conditions in run handler
void    run_die(Run* run);

// free run memory
void    run_destroy(Run* run);

// thread that handles runs. can have several threads active at once
void*   run_daemon(void* vargp);

#endif
