#include "process.h"
#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <wait.h>
#include <fcntl.h>
#include <unistd.h>

typedef struct Process {
    char** argv;
    pthread_t wait_thread;
    pid_t pid;
    ProcessEnum exit_status;
    int status;
    int fd_in, fd_out;
} Process;

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

Process* process_create(const char* command, FILE* infile, FILE* outfile, FILE* errfile)
{
    Process* process;
    pid_t pid;
    int fd_in, fd_out, fd_err;

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
        if (infile != NULL) {
            fd_in = fileno(infile);
            if (fd_in < 0)
                goto fail;
            dup2(fd_in, STDIN_FILENO);
        }
        if (outfile != NULL) {
            fd_out = fileno(outfile);
            if (fd_out < 0)
                goto fail;
            dup2(fd_out, STDOUT_FILENO);
        }
        if (errfile != NULL) {
            fd_err = fileno(errfile);
            if (fd_err < 0)
                goto fail;
            dup2(fd_err, STDERR_FILENO);
        }
        if (execvp("/bin/bash", process->argv))
            goto fail;
    }
    process->pid = pid;
    pthread_create(&process->wait_thread, NULL, process_handler_daemon, process);

    return process;

fail:
    return NULL;
}

ProcessPair process_pair_create(const char* command1, const char* command2)
{
    // add error messages here
    ProcessPair res = (ProcessPair) { NULL, NULL };
    Process* p1;
    Process* p2;
    pid_t pid;
    int pipe_p1_p2[2];
    int pipe_p2_p1[2];

    if (pipe(pipe_p1_p2) == -1) {
        log(ERROR, "Failed to create process1 to process2 pipe");
        return res;
    }
    if (pipe(pipe_p2_p1) == -1) {
        log(ERROR, "Failed to create process2 to process1 pipe");
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
    log(ERROR, "Failed to create process pair");
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

void process_wait(Process* process)
{
    pthread_join(process->wait_thread, 0);
}

// return process memory in kilobytes
size_t process_memory(Process* process)
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

void process_destroy(Process* process)
{
    if (process == NULL)
        return;
    if (process->fd_out != -1)
        close(process->fd_out);
    if (process->fd_in != -1)
        close(process->fd_in);
    kill(process->pid, SIGTERM);
    pthread_kill(process->wait_thread, 69);
    free(process->argv);
    free(process);
}

bool process_success(Process* process)
{
    return process->exit_status == PROCESS_SUCCESS;
}

bool process_failed(Process* process)
{
    return process->exit_status == PROCESS_FAILED;
}

bool process_error(Process* process)
{
    return process->exit_status == PROCESS_ERROR;
}

bool process_running(Process* process)
{
    return process->exit_status == PROCESS_RUNNING;
}

int networking_get_last_error(void)
{
    return 0;
}
