#include "run.h"
#include "state.h"
#include <networking.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <stdbool.h>
#include <wait.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
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
    LETTER,
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
    "VALIDATE_COMMAND",
    "LETTER"
};

typedef enum {
    PROCESS_SUCCESS  = 0,
    PROCESS_WRONG    = 1,
    PROCESS_ERROR    = 2,
    PROCESS_RUNNING  = 3
} ProcessEnum;

typedef struct {
    char buffers[NUM_COMMAND_TOKENS][TOKEN_BUFFER_LENGTH+1];
} TokenBuffers;

typedef struct {
    Run* head;
    Run* tail;
    pthread_mutex_t mutex;
} RunQueue;

typedef struct Process {
    char** argv;
    pthread_t wait_thread;
    pid_t pid;
    ProcessEnum exit_status;
    int status;
    int fd_in, fd_out;
} Process;

typedef struct {
    Process* process1;
    Process* process2;
} ProcessPair;

static RunQueue run_queue = {
    .head = NULL,
    .tail = NULL,
    .mutex = PTHREAD_MUTEX_INITIALIZER
};

static pthread_mutex_t num_runs_mutex = PTHREAD_MUTEX_INITIALIZER;

static void* process_handler_daemon(void* vargp)
{
    Process* process;
    int status;
    process = vargp;
    waitpid(process->pid, &status, 0);
    process->status = status;
    process->exit_status = WEXITSTATUS(status);
    return NULL;
}

static Process* process_create(const char* command, const char* infile_path, const char* outfile_path)
{
    Process* process;
    pid_t pid;
    int fd_in, fd_out;

    process = malloc(sizeof(Process));
    process->argv = malloc(4 * sizeof(char*));
    process->argv[0] = "/bin/bash/";
    process->argv[1] = "-c";
    process->argv[2] = (char*)command;
    process->argv[3] = NULL;
    process->exit_status = PROCESS_RUNNING;
    process->fd_in = process->fd_out = -1;
    pid = fork();
    if (pid == -1) {
        goto fail;
    } else if (pid == 0) {
        if (infile_path != NULL) {
            fd_in = open(infile_path, O_RDONLY, S_IRUSR);
            if (fd_in < 0)
                goto fail;
            dup2(fd_in, 0);
            close(fd_in);
        }
        if (outfile_path != NULL) {
            fd_out = open(outfile_path, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
            if (fd_out < 0)
                goto fail;
            dup2(fd_out, 1);
            dup2(fd_out, 2);
            close(fd_out);
        }
        execvp("/bin/bash", process->argv);
    }
    process->pid = pid;
    pthread_create(&process->wait_thread, NULL, process_handler_daemon, process);

    return process;

fail:
    puts("Something went horribly wrong");
    return NULL;
}

static ProcessPair process_pair_create(const char* command1, const char* command2)
{
    // add error messages here
    ProcessPair res = (ProcessPair) { NULL, NULL };
    Process* p1;
    Process* p2;
    pid_t pid;
    int pipe_p1_p2[2];
    int pipe_p2_p1[2];

    if (pipe(pipe_p1_p2) == -1) {
        puts("Failed to create process1 to process2 pipe");
        return res;
    }
    if (pipe(pipe_p2_p1) == -1) {
        puts("Failed to create process2 to process1 pipe");
        return res;
    }

    p1 = malloc(sizeof(Process));
    p1->argv = malloc(4 * sizeof(char*));
    p1->argv[0] = "/bin/bash";
    p1->argv[1] = "-c";
    p1->argv[2] = (char*)command1;
    p1->argv[3] = NULL;
    p1->exit_status = PROCESS_RUNNING;
    pid = fork();
    if (pid == -1) {
        goto fail;
    } else if (pid == 0) {
        dup2(pipe_p1_p2[1], STDOUT_FILENO);
        dup2(pipe_p2_p1[0], STDIN_FILENO);
        if (execvp("/bin/bash", p1->argv))
            goto fail;
    }
    p1->pid = pid;
    pthread_create(&p1->wait_thread, NULL, process_handler_daemon, p1);

    p2 = malloc(sizeof(Process));
    p2->argv = malloc(4 * sizeof(char*));
    p2->argv[0] = "/bin/bash";
    p2->argv[1] = "-c";
    p2->argv[2] = (char*)command2;
    p2->argv[3] = NULL;
    p2->exit_status = PROCESS_RUNNING;
    pid = fork();
    if (pid == -1) {
        goto fail;
    } else if (pid == 0) {
        dup2(pipe_p2_p1[1], STDOUT_FILENO);
        dup2(pipe_p1_p2[0], STDIN_FILENO);
        if (execvp("/bin/bash", p2->argv))
            goto fail;
    }
    p2->pid = pid;
    pthread_create(&p2->wait_thread, NULL, process_handler_daemon, p2);

    res.process1 = p1;
    res.process2 = p2;
    p1->fd_in = pipe_p2_p1[0];
    p1->fd_out = pipe_p1_p2[1];
    p2->fd_in = pipe_p1_p2[0];
    p2->fd_out = pipe_p2_p1[1];

    return res;

fail:
    puts("failed to create process pair");
    if (p1) {
        kill(p2->pid, SIGTERM);
        free(p1->argv);
        free(p1);
    }
    if (p2) {
        kill(p2->pid, SIGTERM);
        free(p2->argv);
        free(p2);
    }
    close(pipe_p1_p2[0]);
    close(pipe_p1_p2[1]);
    close(pipe_p2_p1[0]);
    close(pipe_p2_p1[1]);

    return (ProcessPair) { NULL, NULL };
}

static void process_wait(Process* process)
{
    pthread_join(process->wait_thread, 0);
}

// return process memory in kilobytes
static size_t process_memory(Process* process)
{
    pid_t pid = process->pid;
    int num_pages = 0, dummy;
    int path_len;
    size_t page_size;
    char* path;
    char* fmt = "/proc/%d/statm";
    path_len = snprintf(NULL, 0, fmt, pid);
    path = malloc((path_len+1) * sizeof(char));
    snprintf(path, path_len+1, fmt, pid);
    FILE* fptr = fopen(path, "r");
    if (fptr != NULL) {
        fscanf(fptr, "%d %d", &dummy, &num_pages);
        fclose(fptr);
    }
    free(path);
    page_size = getpagesize();
    return num_pages * page_size / 1000;
}

static void process_destroy(Process* process)
{
    if (process->fd_out != -1)
        close(process->fd_out);
    if (process->fd_in != -1)
        close(process->fd_in);
    kill(process->pid, SIGTERM);
    pthread_kill(process->wait_thread, 69);
    free(process->argv);
    free(process);
}

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

static bool compile(TokenBuffers* tb, Language* language, Run* run)
{
    bool success;
    Process* process;

    set_token_value_parse(tb, COMPILE_COMMAND, language->compile);
    process = process_create(get_token_value(tb, COMPILE_COMMAND), NULL, get_token_value(tb, COMPILE_PATH));
    process_wait(process);
    success = process->exit_status == PROCESS_SUCCESS;
    process_destroy(process);

    if (success)
        return true;

    set_run_status(run, RUN_COMPILATION_ERROR);
    set_run_response(run, "Compilation failed");

    return false;
}

static int timeval_diff(struct timeval tv1, struct timeval tv2)
{
    return (tv1.tv_sec - tv2.tv_sec) * 1000 + (tv1.tv_usec - tv2.tv_usec) / 1000;
}

#define VALIDATE_SUCCESS    0
#define VALIDATE_FAILED     1
#define VALIDATE_ERROR      2

static int validate(TokenBuffers* tb, Language* language, Problem* problem, Run* run, int testcase)
{
    ProcessPair process_pair;
    Process* validate;
    Process* execute;
    char response[512];
    size_t mem;
    struct timeval start, cur, extra;

    set_token_value_parse(tb, EXECUTE_COMMAND, language->execute);
    set_token_value_parse(tb, VALIDATE_COMMAND, problem->validate);
    puts(get_token_value(tb, VALIDATE_COMMAND));
    process_pair = process_pair_create(get_token_value(tb, VALIDATE_COMMAND), get_token_value(tb, EXECUTE_COMMAND));
    if (process_pair.process1 == NULL)
        goto error;
    validate = process_pair.process1;
    execute = process_pair.process2;
    gettimeofday(&start, NULL);
    cur = start;
    mem = 0;
    while (execute->exit_status == PROCESS_RUNNING && validate->exit_status == PROCESS_RUNNING) {
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

    if (validate->exit_status != PROCESS_RUNNING) {
        if (validate->exit_status == PROCESS_ERROR)
            goto error;
        while (execute->exit_status == PROCESS_RUNNING) {
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

    int s = execute->status;
    printf("%d %d %d %d %d %d %d %d\n", WIFEXITED(s), WEXITSTATUS(s), WIFSIGNALED(s), WTERMSIG(s), WCOREDUMP(s), WIFSTOPPED(s), WSTOPSIG(s), WIFCONTINUED(s));
    if (WSTOPSIG(execute->status)) {
        set_run_status(run, RUN_RUNTIME_ERROR);
        sprintf(response, "Runtime error on testcase %d", testcase);
        goto fail;
    }

    while (validate->exit_status == PROCESS_RUNNING) {
        gettimeofday(&extra, NULL);
        if (timeval_diff(extra, start) > problem->time_limit) {
            set_run_status(run, RUN_WRONG_ANSWER);
            sprintf(response, "Wrong answer on testcase %d", testcase);
            goto fail;
        }
    }

    if (validate->exit_status != PROCESS_SUCCESS) {
        if (validate->exit_status == PROCESS_ERROR)
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
    set_token_value(tb, LETTER, "%c", ctx.problems[run->problem_id].letter);

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
    //if (!create_dir(get_token_value(tb, OUTPUT_DIR))) {
    //    printf("[%d] Couldn't create output directory\n", run->id);
    //    goto server_error;
    //}
    if (!create_dir(get_token_value(tb, CODE_DIR))) {
        printf("[%d] Couldn't create code directory\n", run->id);
        goto server_error;
    }
    if (!create_file(get_token_value(tb, CODE_PATH), run->code, run->code_length)) {
        printf("[%d] Couldn't create code file\n", run->id);
        goto server_error;
    }
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
        set_run_testcase(run, testcase);
        db_exec("UPDATE runs SET status=3, testcase=%d WHERE id=%d", testcase, run->id);
        validate_res = validate(tb, language, problem, run, testcase);
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
    puts("server error");
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
