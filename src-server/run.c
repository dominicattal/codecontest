#include "run.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <process.h>

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

static int num_runs;

Run* run_create(const char* language, const char* code, int code_length)
{
    Run* run = malloc(sizeof(Run));
    run->language = language;
    run->code = code;
    run->code_length = code_length;
    run->response = NULL;
    run->response_length = 0;
    run->problem_id = 0;
    run->id = num_runs++;
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

static bool create_dir(const char* path, mode_t mode)
{
    int result;
#ifdef __WIN32
    result = mkdir(path);
#elif __linux__
    result = mkdir(path, 0777);
#else
    result = 2;
#endif
    if (result == 0 || errno == EEXIST)
        return true;
    return false;
}

static bool create_file(const char* path, const char* code, int code_length)
{
    FILE* file;
    size_t ret;
    bool status = true;
    file = fopen(path, "w");
    if (file == NULL)
        return false;
    ret = fwrite(code, sizeof(char), code_length, file);
    if (ret < (size_t)code_length)
        status = false;
    fclose(file);
    return status;
}

static bool compile(const char* bin_dir, const char* run_dir, const char* path, Run* run)
{
    char command[256];
    char outfile[256];
    char* response;
    int n;
    bool success;
    ProcessID* pid;

    sprintf(command, "python3 compilepython.py %s %s/%d.out", path, bin_dir, run->id);
    sprintf(outfile, "%s/%d.compile", run_dir, run->id);
    puts(bin_dir);
    puts(run_dir);
    puts(path);
    puts(command);
    puts(outfile);
    pid = process_create(command, outfile);
    process_wait(pid);
    success = process_success(pid);
    process_destroy(pid);

    if (success)
        return true;

    response = "Compilation failed";
    n = strlen(response);
    run->response_length = n;
    run->response = malloc((n+1) * sizeof(char));
    memcpy(run->response, response, n+1);

    return false;
}

static void handle_run(Run* run)
{
    char bin_dir[100];
    char run_dir[100];
    char path[128];
    char* response;
    int n;
    run->status = RUN_RUNNING;
    puts("Compiling");
    sprintf(bin_dir, "problem1/bin");
    if (!create_dir(bin_dir, 0777))
        goto server_error;
    sprintf(run_dir, "problem1/runs");
    if (!create_dir(run_dir, 0777))
        goto server_error;
    sprintf(path, "%s/%d.py", run_dir, run->id);
    if (!create_file(path, run->code, run->code_length))
        goto server_error;
    if (!compile(bin_dir, run_dir, path, run))
        goto fail;

    run->status = RUN_SUCCESS;
    run->response_length = 0;
    run->response = malloc(sizeof(char));
    run->response[0] = '\0';
    sem_post(&run->signal);
    return;

fail:
    run->status = RUN_FAILED;
    sem_post(&run->signal);
    return;

server_error:
    response = "server error";
    n = strlen(response);
    puts(response);
    run->status = RUN_SERVER_ERROR;
    run->response_length = n;
    run->response = malloc((n+1) * sizeof(char));
    memcpy(run->response, response, n+1);
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
