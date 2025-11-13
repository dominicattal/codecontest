#include "run.h"
#include "state.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
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

Run* run_create(int team_id, int language_id, int problem_id, const char* code, int code_length)
{
    Run* run = malloc(sizeof(Run));
    run->team_id = team_id;
    run->language_id = language_id;
    run->problem_id = problem_id;
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

static void set_run_response(Run* run, const char* response)
{
    int n = strlen(response);
    run->response_length = n;
    run->response = malloc((n+1) * sizeof(char));
    memcpy(run->response, response, n+1);
}

static bool compile(const char* code_path, const char* exec_path, const char* compile_path, Run* run)
{
    const char* compile;
    char command[256];
    char* response;
    bool success;
    ProcessID* pid;

    compile = ctx.languages[run->language_id].compile;

    sprintf(command, compile, code_path, exec_path);
    pid = process_create(command, NULL, compile_path);
    process_wait(pid);
    success = process_success(pid);
    process_destroy(pid);

    if (success)
        return true;

    run->status = RUN_COMPILATION_ERROR;
    response = "Compilation failed";
    set_run_response(run, response);

    return false;
}

static bool validate(size_t mem_limit, double time_limit, int testcase, const char* validator, const char* execute_command, const char* in_path, const char* out_path, Run* run)
{
    ProcessID* pid;
    char response[512];
    time_t start, cur;
    size_t mem;
    bool status;

    status = true;
    printf("%s\n%s %s\n", execute_command, in_path, out_path);
    pid = process_create(execute_command, in_path, out_path);
    time(&start);
    cur = start;
    while (process_running(pid)) {
        mem = process_memory(pid);
        //printf("%lld %lld\n", mem, mem_limit);
        if (mem > mem_limit) {
            run->status = RUN_MEM_LIMIT_EXCEEDED;
            sprintf(response, "Memory limit exceeded on testcase %d", testcase);
            set_run_response(run, response);
            goto fail;
        }
        time(&cur);
        //printf("%f %f\n", difftime(cur,start), time_limit);
        if (difftime(cur, start) > time_limit) {
            run->status = RUN_TIME_LIMIT_EXCEEDED;
            sprintf(response, "Time limit exceeded on testcase %d", testcase);
            set_run_response(run, response);
            goto fail;
        }
    }
    process_destroy(pid);

    return true;

    pid = process_create(validator, NULL, NULL);
    process_wait(pid);
    status = process_success(pid);
    process_destroy(pid);
    
    if (!status) {
        run->status = RUN_WRONG_ANSWER;
        sprintf(response, "Wrong answer on testcase %d", testcase);
        set_run_response(run, response);
        goto fail;
    }
    
    return true;

fail:
    process_destroy(pid);
    return false;
}

static void handle_run(Run* run)
{
    Problem* problem;
    Language* language;
    const char* username;
    const char* extension;
    const char* problem_dir;
    char bin_dir[100];
    char run_dir[100];
    char out_dir[100];
    char exec_path[128];
    char code_path[128];
    char compile_path[128];
    char in_path[128];
    char out_path[128];
    char execute_command[256];
    char validate_command[256];
    char* response;
    int i;

    problem = &ctx.problems[run->problem_id];
    language = &ctx.languages[run->language_id];
    run->status = RUN_RUNNING;
    username = ctx.teams[run->team_id].username;
    extension = language->extension;
    problem_dir = problem->dir;
    printf("%d\n", ctx.problems[run->problem_id].num_testcases);
    printf("Compiling run %d\n", run->id);
    sprintf(bin_dir, "%s/bin", problem_dir);
    sprintf(run_dir, "%s/runs", problem_dir);
    sprintf(exec_path, "%s/%s-%d.exe", bin_dir, username, run->id);
    sprintf(compile_path, "%s/%s-%d.compile", run_dir, username, run->id);
    sprintf(code_path, "%s/%s-%d%s", run_dir, username, run->id, extension);
    if (!create_dir(bin_dir, 0777))
        goto server_error;
    if (!create_dir(run_dir, 0777))
        goto server_error;
    if (!create_file(code_path, run->code, run->code_length))
        goto server_error;
    if (!compile(code_path, exec_path, compile_path, run))
        goto fail;
    sprintf(out_dir, "%s/runs/%s-%d", problem_dir, username, run->id);
    if (!create_dir(out_dir, 0777))
        goto server_error;
    for (i = 0; i < problem->num_testcases; i++) {
        sprintf(in_path, "%s/cases/%d.in", problem_dir, i);
        sprintf(out_path, "%s/%d.output", out_dir, i);
        sprintf(execute_command, language->execute, exec_path);
        sprintf(validate_command, problem->validator, i, out_path);
        if (!validate(problem->mem_limit, problem->time_limit, i, validate_command, execute_command, in_path, out_path, run))
            goto fail;
    }
    run->status = RUN_SUCCESS;
    run->response_length = 0;
    run->response = malloc(sizeof(char));
    run->response[0] = '\0';
    sem_post(&run->signal);
    return;

fail:
    sem_post(&run->signal);
    return;

server_error:
    response = "server error";
    run->status = RUN_SERVER_ERROR;
    set_run_response(run, response);
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
