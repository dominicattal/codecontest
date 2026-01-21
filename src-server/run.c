#include "run.h"
#include "state.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <process.h>
#include <stdarg.h>
#include <dirent.h>
#include <errno.h>

#define TOKEN_BUFFER_LENGTH 1024

typedef enum {
    PROBLEM_DIR,        
    CASE_DIR_NAME,
    CASE_DIR,
    RUN_DIR_NAME,
    RUN_DIR,
    COMPILE_DIR_NAME,
    COMPILE_DIR,
    BIN_DIR_NAME,
    BIN_DIR,
    TEAM_NAME,
    RUN_ID,
    RUN_PID,
    MINUTE,
    LANGUAGE_EXT,
    CODE_FILENAME,
    BASENAME,
    CODE_DIR,
    CODE_PATH,
    COMPILE_PATH,
    CASE_PATH,
    OUTPUT_DIR,
    OUTPUT_PATH,
    TESTCASE,
    COMPILE_COMMAND,
    EXECUTE_COMMAND,
    VALIDATE_COMMAND,
    NUM_COMMAND_TOKENS,
    INVALID_COMMAND_TOKEN
} CommandToken;

static char* tok_map[NUM_COMMAND_TOKENS] = {
    "PROBLEM_DIR",
    "CASE_DIR_NAME",
    "CASE_DIR",
    "RUN_DIR_NAME",
    "RUN_DIR",
    "COMPILE_DIR_NAME",
    "COMPILE_DIR",
    "BIN_DIR_NAME",
    "BIN_DIR",
    "TEAM_NAME",
    "RUN_ID",
    "RUN_PID",
    "MINUTE",
    "LANGUAGE_EXT",
    "CODE_FILENAME",
    "BASENAME",
    "CODE_DIR",
    "CODE_PATH",
    "COMPILE_PATH",
    "CASE_PATH",
    "OUTPUT_DIR",
    "OUTPUT_PATH",
    "TESTCASE",
    "COMPILE_COMMAND",
    "EXECUTE_COMMAND",
    "VALIDATE_COMMAND"
};

typedef struct {
    char buffers[NUM_COMMAND_TOKENS][TOKEN_BUFFER_LENGTH+1];
} TokenBuffers;

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

static pthread_mutex_t num_runs_mutex = PTHREAD_MUTEX_INITIALIZER;

static TokenBuffers* create_token_buffers(void)
{
    TokenBuffers* tb = malloc(sizeof(TokenBuffers));
    return tb;
}

static char* get_token_value(TokenBuffers* tb, CommandToken tok)
{
    return tb->buffers[tok];
}

static void set_token_value(TokenBuffers* tb, CommandToken tok, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vsprintf(tb->buffers[tok], fmt, args);
    va_end(args);
}

static CommandToken get_token_from_string(const char* str)
{
    for (int i = 0; i < NUM_COMMAND_TOKENS; i++)
        if (strcmp(str, tok_map[i]) == 0)
            return i;
    return INVALID_COMMAND_TOKEN;
}

static void set_token_value_parse(TokenBuffers* tb, CommandToken tok, const char* cmd)
{
    CommandToken matched;
    int i, j, k;
    bool in_token;
    char string[TOKEN_BUFFER_LENGTH];
    char* expanded;
    in_token = false;
    for (i = j = k = 0; cmd[i] != '\0'; i++) {
        if (!in_token) {
            if (cmd[i] == '[') {
                in_token = true;
                j = 0;
            } else {
                tb->buffers[tok][k++] = cmd[i];
            }
        } else {
            if (cmd[i] == ']') {
                in_token = false;
                string[j] = '\0';
                matched = get_token_from_string(string);
                if (matched != INVALID_COMMAND_TOKEN) {
                    expanded = get_token_value(tb, matched);
                    sprintf((char*)(&tb->buffers[tok])+k, "%s", expanded);
                    k += strlen(expanded);
                }
            } else {
                string[j++] = cmd[i];
            }
        }
    }
    tb->buffers[tok][k] = '\0';
}

static void print_token_value(TokenBuffers* tb, CommandToken tok)
{
    printf("%s=%s\n", tok_map[tok], tb->buffers[tok]);
}

static void print_all_tokens(TokenBuffers* tb)
{
    for (int i = 0; i < NUM_COMMAND_TOKENS; i++)
        print_token_value(tb, i);
}

static void delete_token_buffers(TokenBuffers* tb)
{
    free(tb);
}

Run* run_create(const char* filename, int team_id, int language_id, int problem_id, const char* code, int code_length)
{
    Run* run = malloc(sizeof(Run));
    run->filename = filename;
    run->team_id = team_id;
    run->language_id = language_id;
    run->problem_id = problem_id;
    run->code = code;
    run->code_length = code_length;
    run->response = NULL;
    run->response_length = 0;
    run->problem_id = 0;
    run->status = RUN_IDLE;
    run->next = NULL;
    pthread_mutex_lock(&num_runs_mutex);
    run->id = ctx.num_runs++;
    db_exec("INSERT INTO runs (id, team_id, problem_id, language_id, testcase, status) VALUES (%d, %d, %d, %d, 0, 0)",
            run->id, team_id, problem_id, language_id);
    pthread_mutex_unlock(&num_runs_mutex);
    sem_init(&run->run_to_server_signal, 0, 1);
    sem_init(&run->server_to_run_signal, 0, 1);
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
    sem_wait(&run->run_to_server_signal);
    sem_wait(&run->server_to_run_signal);
    pthread_mutex_unlock(&run_queue.mutex);
}

static Run* run_dequeue(void)
{
    // make this a semaphore to prevent busy waiting
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
    sem_wait(&run->run_to_server_signal);
}

void run_post(Run* run)
{
    sem_post(&run->server_to_run_signal);
}

void run_die(Run* run)
{
    run->status = RUN_DEAD;
    run_post(run);
}

void run_destroy(Run* run)
{
    sem_destroy(&run->server_to_run_signal);
    sem_destroy(&run->run_to_server_signal);
    free(run->response);
    free(run);
}

static bool find_dir(const char* path)
{
    DIR* dir;
    dir = opendir(path);
    if (dir) {
        closedir(dir);
        return true;
    }
    return false;
}

static bool create_dir(const char* path)
{
    int result;
#ifdef __WIN32
    result = mkdir(path);
#elif __linux__
    result = mkdir(path, 0777);
#else
    result = 2;
#endif
    // TODO: figure out why EBADF occurs here
    if (result == 0 || errno == EEXIST || errno == EBADF)
        return true;
    printf("Creted dir failed: %s %d\n", path, errno);
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
    run->response_length = n+1;
    run->response = malloc((n+1) * sizeof(char));
    memcpy(run->response, response, n+1);
}

static bool set_run_status(Run* run, RunEnum status)
{
    // update database here?
    run->status = status;
    return run->status != RUN_DEAD;
}


static bool compile(TokenBuffers* tb, Language* language, Run* run)
{
    char* response;
    bool success;
    Process* pid;

    puts("Compiling");
    set_token_value_parse(tb, COMPILE_COMMAND, language->compile);
    pid = process_create(get_token_value(tb, COMPILE_COMMAND), NULL, get_token_value(tb, COMPILE_PATH));
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

static double timeval_diff(struct timeval tv1, struct timeval tv2)
{
    return tv1.tv_sec - tv2.tv_sec + (double)(tv1.tv_usec-tv2.tv_usec)/1000000;
}

#define VALIDATE_SUCCESS    0
#define VALIDATE_FAILED     1
#define VALIDATE_ERROR      2

static bool validate(TokenBuffers* tb, Language* language, Problem* problem, Run* run, int testcase)
{
    Process* pid;
    char response[512];
    size_t mem;
    struct timeval start, cur;

    puts("Executing");
    set_token_value_parse(tb, EXECUTE_COMMAND, language->execute);
    pid = process_create(get_token_value(tb, EXECUTE_COMMAND), 
            get_token_value(tb, CASE_PATH), 
            get_token_value(tb, OUTPUT_PATH));
    gettimeofday(&start, NULL);
    cur = start;
    while (process_running(pid)) {
        mem = process_memory(pid);
        if (mem > problem->mem_limit) {
            set_run_status(run, RUN_MEM_LIMIT_EXCEEDED);
            sprintf(response, "Memory limit exceeded on testcase %d", testcase);
            goto fail;
        }
        gettimeofday(&cur, NULL);
        if (timeval_diff(cur, start) > problem->time_limit) {
            set_run_status(run, RUN_TIME_LIMIT_EXCEEDED);
            sprintf(response, "Time limit exceeded on testcase %d", testcase);
            goto fail;
        }
    }
    if (!process_success(pid)) {
        if (process_error(pid))
            return VALIDATE_ERROR;
        set_run_status(run, RUN_RUNTIME_ERROR);
        sprintf(response, "Runtime error on testcase %d", testcase);
        goto fail;
    }
    process_destroy(pid);

    puts("Validating");
    set_token_value_parse(tb, VALIDATE_COMMAND, problem->validate);
    pid = process_create(get_token_value(tb, VALIDATE_COMMAND), NULL, NULL);
    process_wait(pid);
    if (!process_success(pid)) {
        if (process_error(pid))
            return VALIDATE_ERROR;
        set_run_status(run, RUN_WRONG_ANSWER);
        sprintf(response, "Wrong answer on testcase %d", testcase);
        goto fail;
    }
    process_destroy(pid);
    return VALIDATE_SUCCESS;

fail:
    set_run_response(run, response);
    process_destroy(pid);
    return VALIDATE_FAILED;
}

static void try_override(JsonObject* object, TokenBuffers* tb, CommandToken tok)
{
    JsonValue* value;
    const char* string;
    string = tok_map[tok];
    value = json_get_value(object, string);
    if (value == NULL) 
        return;
    if (json_get_type(value) != JTYPE_STRING) 
        return;
    set_token_value_parse(tb, tok, json_get_string(value));
}

static void handle_run(TokenBuffers* tb, Run* run)
{
    Problem* problem;
    Language* language;
    char* response;
    int testcase;
    int validate_res;

    problem = &ctx.problems[run->problem_id];
    language = &ctx.languages[run->language_id];

    // default values
    set_token_value(tb, PROBLEM_DIR, "%s", problem->dir);
    set_token_value(tb, CASE_DIR_NAME, "cases");
    set_token_value(tb, CASE_DIR, "%s/%s",
        get_token_value(tb, PROBLEM_DIR),
        get_token_value(tb, CASE_DIR_NAME));
    set_token_value(tb, RUN_DIR_NAME, "runs");
    set_token_value(tb, RUN_DIR, "%s/%s",
        get_token_value(tb, PROBLEM_DIR),
        get_token_value(tb, RUN_DIR_NAME));
    set_token_value(tb, COMPILE_DIR_NAME, "runs");
    set_token_value(tb, COMPILE_DIR, "%s/%s",
        get_token_value(tb, PROBLEM_DIR),
        get_token_value(tb, COMPILE_DIR_NAME));
    set_token_value(tb, BIN_DIR_NAME, "bin");
    set_token_value(tb, BIN_DIR, "%s/%s", 
        get_token_value(tb, PROBLEM_DIR), 
        get_token_value(tb, BIN_DIR_NAME));
    set_token_value(tb, TEAM_NAME, "%s", ctx.teams[run->team_id].username);
    set_token_value(tb, RUN_ID, "%d", run->id);
    set_token_value(tb, RUN_PID, "%d", -1);
    set_token_value(tb, MINUTE, "%d", -1);
    set_token_value(tb, LANGUAGE_EXT, "%s", language->extension);
    set_token_value(tb, CODE_FILENAME, run->filename);
    set_token_value(tb, BASENAME, "%s-%s",
        get_token_value(tb, TEAM_NAME),
        get_token_value(tb, RUN_ID));
    set_token_value(tb, CODE_DIR, "%s",
        get_token_value(tb, RUN_DIR));
    set_token_value(tb, CODE_PATH, "%s/%s.%s",
        get_token_value(tb, CODE_DIR),
        get_token_value(tb, BASENAME),
        get_token_value(tb, LANGUAGE_EXT));
    set_token_value(tb, COMPILE_PATH, "%s/%s.compile",
        get_token_value(tb, COMPILE_DIR),
        get_token_value(tb, BASENAME));
    set_token_value(tb, OUTPUT_DIR, "%s/%s",
        get_token_value(tb, RUN_DIR),
        get_token_value(tb, BASENAME));

    // override
    try_override(language->object, tb, CODE_DIR);
    try_override(language->object, tb, CODE_PATH);

    if (!find_dir(get_token_value(tb, CASE_DIR))) {
        printf("[%d] Couldn't find case directory\n", run->id);
        goto server_error;
    }
    if (!create_dir(get_token_value(tb, RUN_DIR))) {
        printf("[%d] Couldn't create run directory\n", run->id);
        goto server_error;
    }
    if (!create_dir(get_token_value(tb, COMPILE_DIR))) {
        printf("[%d] Couldn't create compile directory\n", run->id);
        goto server_error;
    }
    if (!create_dir(get_token_value(tb, BIN_DIR))) {
        printf("[%d] Couldn't create bin directory\n", run->id);
        goto server_error;
    }
    if (!create_dir(get_token_value(tb, OUTPUT_DIR))) {
        printf("[%d] Couldn't create output directory\n", run->id);
        goto server_error;
    }
    if (!create_dir(get_token_value(tb, CODE_DIR))) {
        printf("[%d] Couldn't create code directory\n", run->id);
        goto server_error;
    }
    if (!create_file(get_token_value(tb, CODE_PATH), run->code, run->code_length)) {
        printf("[%d] Couldn't create code file\n", run->id);
        goto server_error;
    }
    set_run_response(run, "Compiling");
    set_run_status(run, RUN_COMPILING);
    if (!compile(tb, language, run))
        goto fail;
    set_run_status(run, RUN_RUNNING);
    for (testcase = 0; testcase < problem->num_testcases; testcase++) {
        set_token_value(tb, TESTCASE, "%d", testcase);
        set_token_value(tb, CASE_PATH, "%s/%s.in",
                get_token_value(tb, CASE_DIR),
                get_token_value(tb, TESTCASE));
        set_token_value(tb, OUTPUT_PATH, "%s/%s.output",
                get_token_value(tb, OUTPUT_DIR),
                get_token_value(tb, TESTCASE));
        validate_res = validate(tb, language, problem, run, testcase);
        if (validate_res == VALIDATE_FAILED)
            goto fail;
        else if (validate_res == VALIDATE_ERROR)
            goto server_error;
    }
    set_run_status(run, RUN_SUCCESS);
    set_run_response(run, "");
    sem_post(&run->run_to_server_signal);
    return;

fail:
    sem_post(&run->run_to_server_signal);
    return;

server_error:
    puts("");
    print_all_tokens(tb);
    response = "server error";
    run->status = RUN_SERVER_ERROR;
    set_run_response(run, response);
    sem_post(&run->run_to_server_signal);
    return;
}

void* run_daemon(void* vargp)
{
    Run* run;
    TokenBuffers* tb = create_token_buffers();
    while (!ctx.kill) {
        run = run_dequeue();
        if (run == NULL)
            continue;
        handle_run(tb, run);
    }
    delete_token_buffers(tb);
    return NULL;
}
