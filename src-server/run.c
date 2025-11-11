#include "run.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

typedef struct {
    Run* head;
    Run* tail;
    pthread_mutex_t mutex;
} RunQueue;

static RunQueue run_queue = {
    .head = NULL,
    .tail = NULL,
    .mutex = PTHREAD_MUTEX_INITIALIZER
};

Run* run_create(const char* language, const char* code)
{
    Run* run = malloc(sizeof(Run));
    run->language = language;
    run->code = code;
    run->response = NULL;
    run->response_length = 0;
    run->problem_id = 0;
    run->status = RUN_IDLE;
    run->next = NULL;
    sem_init(&run->signal, 0, 1);
    return run;
}

void run_enqueue(Run* run)
{
    pthread_mutex_lock(&run_queue.mutex);
    if (run_queue.tail == NULL) {
        run_queue.head = run_queue.tail = run;
    } else {
        run_queue.tail->next = run;
        run_queue.tail = run;
    }
    sem_wait(&run->signal);
    pthread_mutex_unlock(&run_queue.mutex);
}

static Run* run_dequeue(void)
{
    Run* run = NULL;
    pthread_mutex_lock(&run_queue.mutex);
    if (run_queue.head != NULL) {
        run = run_queue.head;
        if (run->next == NULL)
            run_queue.head = run_queue.tail = NULL;
        else
            run_queue.head = run->next;
        run->next = NULL;
    }
    pthread_mutex_unlock(&run_queue.mutex);
    return run;
}

void run_wait(Run* run)
{
    sem_wait(&run->signal);
}

void run_destroy(Run* run)
{
    sem_destroy(&run->signal);
    free(run->response);
    free(run);
}

#include <windows.h>
static void handle_run(Run* run)
{
    puts("Running");
    Sleep(5000);
    static char* response = "you failed lol";
    int n = strlen(response);
    run->status = RUN_FAILED;
    run->response = malloc((n+1) * sizeof(char));
    memcpy(run->response, response, n+1);
    run->response_length = n;
    sem_post(&run->signal);
}

void* run_daemon(void* vargp)
{
    Run* run;
    while (1) {
        run = run_dequeue();
        if (run == NULL)
            continue;
        handle_run(run);
    }
    return NULL;
}
