#include "run.h"
#include <stdlib.h>
#include <string.h>

static RunQueue run_queue;

Run* run_create(const char* language, const char* code)
{
    Run* run = malloc(sizeof(Run));
    run->language = language;
    run->code = code;
    run->response = "this is a test";
    run->response_length = strlen(run->response);
    run->problem_id = 0;
    run->status = RUN_IDLE;
    run->next = NULL;
    return run;
}

void run_enqueue(Run* run)
{
    if (run_queue.tail == NULL) {
        run_queue.head = run_queue.tail = run;
        return;
    }
    run_queue.tail->next = run;
    run_queue.tail = run;
}

void run_wait(Run* run)
{
}

void run_destroy(Run* run)
{
    //free(run->response);
    free(run);
}
