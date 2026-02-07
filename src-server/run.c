#include "run.h"
#include "main.h"
#include "process.h"
#include <networking.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/time.h>
#include <wait.h>
#include <dirent.h>
#include <errno.h>

#define TOKEN_BUFFER_LENGTH 1024

typedef enum {
    DATA_DIR,
    PROBLEM_DIR,        
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
    ANSWER_PATH,
    OUTPUT_DIR,
    OUTPUT_PATH,
    RUNTIME_DIR,
    RUNTIME_PATH,
    TESTCASE,
    COMPILE_COMMAND,
    EXECUTE_COMMAND,
    VALIDATE_COMMAND,
    LETTER,
    NUM_COMMAND_TOKENS,
    INVALID_COMMAND_TOKEN
} CommandToken;

static char* tok_map[NUM_COMMAND_TOKENS] = {
    "DATA_DIR",
    "PROBLEM_DIR",
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
    "ANSWER_PATH",
    "OUTPUT_DIR",
    "OUTPUT_PATH",
    "RUNTIME_DIR",
    "RUNTIME_PATH",
    "TESTCASE",
    "COMPILE_COMMAND",
    "EXECUTE_COMMAND",
    "VALIDATE_COMMAND",
    "LETTER"
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
    log(INFO, "%s=%s", tok_map[tok], tb->buffers[tok]);
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

static void send_run_to_web_clients(Run* run)
{
    Packet* packet;
    Team* team;
    Language* lang;
    Problem* problem;
    int buf_len;
    char* buffer;
    char* fmt;

    problem = &ctx.problems[run->problem_id];
    lang = &ctx.languages[run->language_id];
    team = &ctx.teams[run->team_id];

    fmt = "%d\r%d\r%d\r%c\r%s\r%s\r%s\r%d\r%d";
    buf_len = snprintf(NULL, 0, fmt, run->id, run->status, run->testcase, problem->letter, 
                       problem->name, lang->name, team->username, run->time, run->memory); 
    buffer = malloc((buf_len+1) * sizeof(char));
    snprintf(buffer, buf_len+1, fmt, run->id, run->status, run->testcase, problem->letter, 
             problem->name, lang->name, team->username, run->time, run->memory);
    packet = packet_create(WEB_PACKET_TEXT, buf_len, buffer);
    socket_send_web_all(ctx.web_net_ctx, packet);
    free(buffer);
    packet_destroy(packet);
}

static void set_run_response(Run* run, const char* response)
{
    if (run->response != NULL)
        free(run->response);
    int n = strlen(response);
    run->response_length = n+1;
    run->response = malloc((n+1) * sizeof(char));
    memcpy(run->response, response, n+1);
}

static bool set_run_status(Run* run, RunEnum status)
{
    bool res;
    run->status = status;
    res = db_exec("UPDATE runs SET status=%d WHERE id=%d", run->status, run->id);
    send_run_to_web_clients(run);
    return res;
}

static bool set_run_testcase(Run* run, int testcase)
{
    bool res;
    run->testcase = testcase;
    res = db_exec("UPDATE runs SET testcase=%d WHERE id=%d", run->testcase, run->id);
    send_run_to_web_clients(run);
    return res;
}

static bool set_run_stats(Run* run, int time, int time_limit, int memory, int memory_limit)
{
    bool res;
    run->time = (time < time_limit) ? time : time_limit;
    run->memory = (memory < memory_limit) ? memory : memory_limit;
    res = db_exec("UPDATE runs SET time=%d, memory=%d WHERE id=%d", run->time, run->memory, run->id);
    send_run_to_web_clients(run);
    return res;
}

Run* run_create(const char* filename, int team_id, int language_id, int problem_id, const char* code, int code_length, bool async)
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
    run->problem_id = problem_id;
    run->testcase = 0;
    run->time = 0;
    run->memory = 0;
    run->status = RUN_IDLE;
    run->next = NULL;
    run->async = async;
    pthread_mutex_lock(&num_runs_mutex);
    run->id = ctx.num_runs++;
    db_exec("INSERT INTO runs (id, team_id, problem_id, language_id, testcase, status, timestamp, time, memory)"
            "VALUES (%d, %d, %d, %d, 0, 0, 0, 0, 0)",
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

static bool compile(TokenBuffers* tb, Language* language, Run* run)
{
    bool success = true;
    Process* process;
    FILE* outfile;

    outfile = fopen(get_token_value(tb, COMPILE_PATH), "w");
    set_token_value_parse(tb, COMPILE_COMMAND, language->compile);
    //log(INFO, "compile run %d: %s", run->id, get_token_value(tb, COMPILE_COMMAND));
    process = process_create(get_token_value(tb, COMPILE_COMMAND), NULL, outfile, outfile);
    process_wait(process);
    success = process_success(process);
    process_destroy(process);
    fclose(outfile);

    if (!success) {
        set_run_status(run, RUN_COMPILATION_ERROR);
        set_run_response(run, "Compilation failed");
        return false;
    }

    if (remove(get_token_value(tb, COMPILE_PATH)) == -1)
        log(WARNING, "Could not remove compile path %s errno=%d", get_token_value(tb, COMPILE_PATH), errno);

    return true;
}

static int timeval_diff(struct timeval tv1, struct timeval tv2)
{
    return (tv1.tv_sec - tv2.tv_sec) * 1000 + (tv1.tv_usec - tv2.tv_usec) / 1000;
}

#define VALIDATE_SUCCESS    0
#define VALIDATE_FAILED     1
#define VALIDATE_ERROR      2

static int validate_without_pipe(TokenBuffers* tb, Language* language, Problem* problem, Run* run, int testcase)
{
    Process* validate = NULL;
    Process* execute = NULL;
    char response[512];
    size_t mem;
    long pos;
    struct timeval start, cur;
    FILE* infile = NULL;
    FILE* outfile = NULL;
    FILE* errfile = NULL;
    int validate_status;

    validate_status = VALIDATE_SUCCESS;

    set_token_value_parse(tb, EXECUTE_COMMAND, language->execute);
    infile = fopen(get_token_value(tb, CASE_PATH), "r");
    if (infile == NULL) {
        log(ERROR, "Could not open infile %s", get_token_value(tb, OUTPUT_PATH));
        goto error;
    }
    outfile = fopen(get_token_value(tb, OUTPUT_PATH), "w+");
    if (outfile == NULL) {
        log(ERROR, "Could not open outfile %s", get_token_value(tb, OUTPUT_PATH));
        goto error;
    }
    errfile = fopen(get_token_value(tb, RUNTIME_PATH), "w");
    if (errfile == NULL) {
        log(ERROR, "Could not open errfile %s", get_token_value(tb, RUNTIME_PATH));
        goto error;
    }
    execute = process_create(get_token_value(tb, EXECUTE_COMMAND), infile, outfile, errfile);
    //log(INFO, "execute run %d no pipe: %s infile=%s outfile=%s errfile=%s", run->id, get_token_value(tb, EXECUTE_COMMAND), get_token_value(tb, CASE_PATH), get_token_value(tb, OUTPUT_PATH), get_token_value(tb, RUNTIME_PATH));
    if (execute == NULL)
        goto error;
    gettimeofday(&start, NULL);
    cur = start;
    mem = 0;
    while (execute->running) {
        mem = process_memory(execute);
        run->memory = (mem > run->memory) ? mem : run->memory;
        if (run->memory > problem->mem_limit) {
            set_run_status(run, RUN_MEM_LIMIT_EXCEEDED);
            sprintf(response, "Memory limit exceeded on testcase %d", testcase);
            goto fail;
        }
        gettimeofday(&cur, NULL);
        run->time = timeval_diff(cur, start);
        if (run->time > problem->time_limit) {
            set_run_status(run, RUN_TIME_LIMIT_EXCEEDED);
            sprintf(response, "Time limit exceeded on testcase %d", testcase);
            goto fail;
        }
    }
    if (process_error(execute))
        goto error;

    //int s = execute->status;
    //log(INFO, "%d %d %d %d %d %d %d %d", WIFEXITED(s), WEXITSTATUS(s), WIFSIGNALED(s), WTERMSIG(s), WCOREDUMP(s), WIFSTOPPED(s), WSTOPSIG(s), WIFCONTINUED(s));
    if (WIFSIGNALED(execute->status) || WSTOPSIG(execute->status)) {
        set_run_status(run, RUN_RUNTIME_ERROR);
        sprintf(response, "Runtime error on testcase %d", testcase);
        goto fail;
    }
    process_destroy(execute);
    execute = NULL;
    fclose(infile);
    infile = NULL;
    fclose(outfile);
    outfile = NULL;

    set_token_value_parse(tb, VALIDATE_COMMAND, problem->validate);
    validate = process_create(get_token_value(tb, VALIDATE_COMMAND), NULL, NULL, NULL);
    //log(INFO, "validate run %d no pipe: %s", run->id, get_token_value(tb, VALIDATE_COMMAND));
    process_wait(validate);
    if (process_error(validate))
        goto error;
    if (process_failed(validate)) {
        set_run_status(run, RUN_WRONG_ANSWER);
        sprintf(response, "Wrong answer on testcase %d", testcase);
        goto fail;
    }
    process_destroy(validate);
    validate = NULL;
    set_run_stats(run, run->time, problem->time_limit, run->memory, problem->mem_limit);
    goto destroy;

fail:
    validate_status = VALIDATE_FAILED;
    set_run_stats(run, run->time, problem->time_limit, run->memory, problem->mem_limit);
    set_run_response(run, response);
    goto destroy;

error:
    validate_status = VALIDATE_ERROR;
    set_run_stats(run, 0, 0, 0, 0);
    goto destroy;

destroy:
    if (execute != NULL)
        process_destroy(execute);
    if (validate != NULL)
        process_destroy(validate);
    if (infile != NULL)
        fclose(infile);
    if (outfile != NULL)
        fclose(outfile);
    if (errfile != NULL) {
        pos = ftell(errfile);
        fclose(errfile);
        if (pos == 0 && remove(get_token_value(tb, RUNTIME_PATH)) == -1)
            log(WARNING, "Could not remove output path %s errno=%d", get_token_value(tb, RUNTIME_PATH), errno);
    }
    if (remove(get_token_value(tb, OUTPUT_PATH)) == -1)
        log(WARNING, "Could not remove output path %s errno=%d", get_token_value(tb, OUTPUT_PATH), errno);
    return validate_status;
}

static int validate_with_pipe(TokenBuffers* tb, Language* language, Problem* problem, Run* run, int testcase)
{
    ProcessPair process_pair;
    Process* validate;
    Process* execute;
    char response[512];
    size_t mem;
    struct timeval start, cur, extra;

    set_token_value_parse(tb, EXECUTE_COMMAND, language->execute);
    set_token_value_parse(tb, VALIDATE_COMMAND, problem->validate);
    process_pair = process_pair_create(get_token_value(tb, VALIDATE_COMMAND), get_token_value(tb, EXECUTE_COMMAND));
    validate = process_pair.process1;
    execute = process_pair.process2;
    if (validate == NULL)
        goto error;
    if (execute == NULL)
        goto error;
    gettimeofday(&start, NULL);
    cur = start;
    mem = 0;
    while (execute->running && validate->running) {
        mem = process_memory(execute);
        run->memory = (mem > run->memory) ? mem : run->memory;
        if (run->memory > problem->mem_limit) {
            set_run_status(run, RUN_MEM_LIMIT_EXCEEDED);
            sprintf(response, "Memory limit exceeded on testcase %d", testcase);
            goto fail;
        }
        gettimeofday(&cur, NULL);
        run->time = timeval_diff(cur, start);
        if (run->time > problem->time_limit) {
            set_run_status(run, RUN_TIME_LIMIT_EXCEEDED);
            sprintf(response, "Time limit exceeded on testcase %d", testcase);
            goto fail;
        }
    }

    if (!validate->running) {
        if (process_error(validate))
            goto error;
        while (execute->running) {
            mem = process_memory(execute);
            run->memory = (mem > run->memory) ? mem : run->memory;
            if (run->memory > problem->mem_limit) {
                set_run_status(run, RUN_MEM_LIMIT_EXCEEDED);
                sprintf(response, "Memory limit exceeded on testcase %d", testcase);
                goto fail;
            }
            gettimeofday(&extra, NULL);
            run->time = timeval_diff(extra, start);
            if (run->time > problem->time_limit) {
                cur = extra;
                set_run_status(run, RUN_TIME_LIMIT_EXCEEDED);
                sprintf(response, "Time limit exceeded on testcase %d", testcase);
                goto fail;
            }
        }
    }

    //int s = execute->status;
    //log(INFO, "%d %d %d %d %d %d %d %d", WIFEXITED(s), WEXITSTATUS(s), WIFSIGNALED(s), WTERMSIG(s), WCOREDUMP(s), WIFSTOPPED(s), WSTOPSIG(s), WIFCONTINUED(s));
    if (WIFSIGNALED(execute->status) || WSTOPSIG(execute->status)) {
        set_run_status(run, RUN_RUNTIME_ERROR);
        sprintf(response, "Runtime error on testcase %d", testcase);
        goto fail;
    }

    while (validate->running) {
        gettimeofday(&extra, NULL);
        if (timeval_diff(extra, start) > problem->time_limit) {
            set_run_status(run, RUN_WRONG_ANSWER);
            sprintf(response, "Wrong answer on testcase %d", testcase);
            goto fail;
        }
    }

    if (!process_success(validate)) {
        if (process_error(validate))
            goto error;
        set_run_status(run, RUN_WRONG_ANSWER);
        sprintf(response, "Wrong answer on testcase %d", testcase);
        goto fail;
    }

    process_destroy(execute);
    process_destroy(validate);
    set_run_stats(run, run->time, problem->time_limit, run->memory, problem->mem_limit);
    return VALIDATE_SUCCESS;

fail:
    set_run_stats(run, run->time, problem->time_limit, run->memory, problem->mem_limit);
    set_run_response(run, response);
    process_destroy(execute);
    process_destroy(validate);
    return VALIDATE_FAILED;
    
error:
    set_run_stats(run, 0, 0, 0, 0);
    process_destroy(execute);
    process_destroy(validate);
    return VALIDATE_ERROR;
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
    int testcase;
    int validate_res;

    db_exec("UPDATE runs SET status=1 WHERE id=%d", run->id);

    problem = &ctx.problems[run->problem_id];
    language = &ctx.languages[run->language_id];

    // default values
    set_token_value(tb, DATA_DIR, "data");
    set_token_value(tb, PROBLEM_DIR, "%s", problem->dir);
    set_token_value(tb, CASE_DIR, "%s", problem->testcases_dir);
    set_token_value(tb, RUN_DIR_NAME, "runs");
    set_token_value(tb, RUN_DIR, "%s/%s/%s",
        get_token_value(tb, DATA_DIR),
        get_token_value(tb, PROBLEM_DIR),
        get_token_value(tb, RUN_DIR_NAME));
    set_token_value(tb, COMPILE_DIR_NAME, "runs");
    set_token_value(tb, COMPILE_DIR, "%s/%s/%s",
        get_token_value(tb, DATA_DIR),
        get_token_value(tb, PROBLEM_DIR),
        get_token_value(tb, COMPILE_DIR_NAME));
    set_token_value(tb, BIN_DIR_NAME, "bin");
    set_token_value(tb, BIN_DIR, "%s/%s/%s", 
        get_token_value(tb, DATA_DIR), 
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
    set_token_value(tb, OUTPUT_DIR, "%s/tmp",
        get_token_value(tb, DATA_DIR));
    set_token_value(tb, RUNTIME_DIR, "%s",
        get_token_value(tb, COMPILE_DIR));
    set_token_value(tb, LETTER, "%c", ctx.problems[run->problem_id].letter);

    // override
    try_override(language->object, tb, CODE_DIR);
    try_override(language->object, tb, CODE_PATH);

    if (!find_dir(get_token_value(tb, CASE_DIR))) {
        log(ERROR, "[%d] Couldn't find case directory", run->id);
        goto server_error;
    }
    if (!create_dir(get_token_value(tb, RUN_DIR))) {
        log(ERROR, "[%d] Couldn't create run directory", run->id);
        goto server_error;
    }
    if (!create_dir(get_token_value(tb, COMPILE_DIR))) {
        log(ERROR, "[%d] Couldn't create compile directory", run->id);
        goto server_error;
    }
    if (!create_dir(get_token_value(tb, BIN_DIR))) {
        log(ERROR, "[%d] Couldn't create bin directory", run->id);
        goto server_error;
    }
    if (!create_dir(get_token_value(tb, OUTPUT_DIR))) {
        log(ERROR, "[%d] Couldn't create output directory", run->id);
        goto server_error;
    }
    if (!create_dir(get_token_value(tb, CODE_DIR))) {
        log(ERROR, "[%d] Couldn't create code directory", run->id);
        goto server_error;
    }
    if (!create_dir(get_token_value(tb, RUNTIME_DIR))) {
        log(ERROR, "[%d] Couldn't create error directory", run->id);
        goto server_error;
    }
    if (!create_file(get_token_value(tb, CODE_PATH), run->code, run->code_length)) {
        log(ERROR, "[%d] Couldn't create code file %s", run->id, get_token_value(tb, CODE_PATH));
        goto server_error;
    }
    set_run_status(run, RUN_COMPILING);
    if (!compile(tb, language, run))
        goto fail;
    set_run_status(run, RUN_RUNNING);
    for (testcase = 0; testcase < problem->num_testcases; testcase++) {
        set_token_value(tb, TESTCASE, "%d", testcase);
        set_token_value(tb, CASE_PATH, "%s/%s.in",
                problem->testcases_dir,
                problem->testcases[testcase].name);
        set_token_value(tb, ANSWER_PATH, "%s/%s.ans",
                problem->testcases_dir,
                problem->testcases[testcase].name);
        set_token_value(tb, OUTPUT_PATH, "%s/%s-%s-%s.output",
                get_token_value(tb, OUTPUT_DIR),
                get_token_value(tb, TEAM_NAME),
                get_token_value(tb, RUN_ID),
                get_token_value(tb, TESTCASE));
        set_token_value(tb, RUNTIME_PATH, "%s/%s-%s.runtime",
            get_token_value(tb, COMPILE_DIR),
            get_token_value(tb, BASENAME),
            get_token_value(tb, TESTCASE));
        set_run_testcase(run, testcase);
        db_exec("UPDATE runs SET status=3, testcase=%d WHERE id=%d", testcase, run->id);
        if (problem->pipe)
            validate_res = validate_with_pipe(tb, language, problem, run, testcase);
        else
            validate_res = validate_without_pipe(tb, language, problem, run, testcase);
        if (validate_res == VALIDATE_FAILED)
            goto fail;
        else if (validate_res == VALIDATE_ERROR)
            goto server_error;
    }
    set_run_status(run, RUN_SUCCESS);
    set_run_response(run, "");
    goto deconstruct_run;

fail:
    goto deconstruct_run;

server_error:
    log(ERROR, "server error");
    print_all_tokens(tb);
    set_run_status(run, RUN_SERVER_ERROR);
    set_run_response(run, "server error");
    sem_post(&run->run_to_server_signal);
    goto deconstruct_run;

deconstruct_run:
    if (!run->async)
        sem_post(&run->run_to_server_signal);
    else
        run_destroy(run);
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
