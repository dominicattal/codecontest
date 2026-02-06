#include "process.h"
#include <stdio.h>

#ifdef __WIN32

#include <windows.h>
#include <psapi.h>
#include <io.h>

typedef struct Process {
    PROCESS_INFORMATION pi;
    HANDLE infile;
    HANDLE infile_rd;
} Process;

char* read_file(const char* file_path, int* size)
{
    FILE* file;
    char* buffer;
    int end, length;
    file = fopen(file_path, "r");
    if (file == NULL)
        goto fail;
    if (fseek(file, 0, SEEK_END) != 0)
        goto fail_close_file;
    end = ftell(file);
    if (end == -1L)
        goto fail_close_file;
    buffer = malloc((end+1)*sizeof(char));
    if (buffer == NULL)
        goto fail_close_file;
    if (fseek(file, 0, SEEK_SET) != 0)
        goto fail_free_buffer;
    length = fread(buffer, sizeof(char), end, file);
    *size = length+1;
    if (ferror(file) != 0)
        goto fail_free_buffer;
    fclose(file);
    buffer[length] = EOF;
    return buffer;

fail_free_buffer:
    free(buffer);
fail_close_file:
    fclose(file);
fail:
    return NULL;
}

Process* process_create(const char* command, const char* infile_path, const char* outfile_path, const char* errfile_path)
{
    STARTUPINFO si;
    Process* process;
    char* text;
    int size;
    DWORD written;
    SECURITY_ATTRIBUTES sa, saAttr;
    int outfd;
    HANDLE outfile_handle = NULL;
    HANDLE errfile_handle = NULL;
    HANDLE infile_wr;
    DWORD flags;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES); 
    saAttr.bInheritHandle = TRUE; 
    saAttr.lpSecurityDescriptor = NULL;
    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;
    flags = CREATE_NO_WINDOW;
    process = malloc(sizeof(Process));
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&process->pi, sizeof(process->pi));
    process->infile = NULL;
    process->infile_rd = NULL;
    si.cb = sizeof(si);
    si.dwFlags |= STARTF_USESTDHANDLES;
    if (infile_path != NULL) {
        CreatePipe(&process->infile_rd, &infile_wr, &saAttr, 0);
        SetHandleInformation(&infile_wr, HANDLE_FLAG_INHERIT, 0);
        text = read_file(infile_path, &size);
        if (text == NULL) {
            puts("text is null");
            goto fail_really_bad;
        }
        WriteFile(infile_wr, text, size, &written, NULL);
        free(text);
        CloseHandle(infile_wr);
        si.hStdInput = process->infile_rd;
    }
    if (outfile_path != NULL) {
        outfile_handle = CreateFile(outfile_path, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, &sa, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        si.hStdOutput = outfile_handle;
    }
    if (errfile_path != NULL) {
        errfile_handle = CreateFile(errfile_path, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, &sa, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        si.hStdError = errfile_handle;
    }
    if (!CreateProcess(NULL, (char*)command, NULL, NULL, TRUE, flags, NULL, NULL, &si, &process->pi))
        goto fail_really_bad;
    if (outfile_handle != NULL)
        CloseHandle(outfile_handle);
    if (errfile_handle != NULL)
        CloseHandle(errfile_handle);
    return process;

fail_really_bad:
    printf("Something went very wrong error: %ld %s:%d", GetLastError(), __FILE__, __LINE__);
    printf("Process pid: %ld\n", process->pi.dwProcessId);
    exit(1);
    return NULL;
}

ProcessPair process_pair_create(const char* command1, const char* command2)
{
    puts("Process pair not implemented yet");
    return (ProcessPair) {NULL,NULL};
}

void process_wait(Process* process)
{
    WaitForSingleObject(process->pi.hProcess, INFINITE);
}

size_t process_memory(Process* process)
{
    PROCESS_MEMORY_COUNTERS ppsemCounters;
    GetProcessMemoryInfo(process->pi.hProcess, &ppsemCounters, sizeof(ppsemCounters));
    return ((size_t)ppsemCounters.QuotaPeakPagedPoolUsage);
}

bool process_success(Process* process)
{
    DWORD lpExitCode;
    GetExitCodeProcess(process->pi.hProcess, &lpExitCode);
    return lpExitCode == PROCESS_SUCCESS;
}

bool process_failed(Process* process)
{
    DWORD lpExitCode;
    GetExitCodeProcess(process->pi.hProcess, &lpExitCode);
    return lpExitCode == PROCESS_FAILED;
}

bool process_error(Process* process)
{
    DWORD lpExitCode;
    GetExitCodeProcess(process->pi.hProcess, &lpExitCode);
    return lpExitCode == PROCESS_ERROR;
}

bool process_running(Process* process)
{
    DWORD lpExitCode;
    GetExitCodeProcess(process->pi.hProcess, &lpExitCode);
    return lpExitCode == STILL_ACTIVE;
}


void process_destroy(Process* process)
{
    if (process->pi.hProcess != NULL)
        TerminateProcess(process->pi.hProcess, 1);
    if (process->infile != NULL)
        CloseHandle(process->infile);
    if (process->infile_rd != NULL)
        CloseHandle(process->infile_rd);
    if (process->pi.hProcess != NULL)
        CloseHandle(process->pi.hProcess);
    if (process->pi.hThread != NULL)
        CloseHandle(process->pi.hThread);
    free(process);
}

#else

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

Process* process_create(const char* command, const char* infile_path, const char* outfile_path, const char* errfile_path)
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
        if (infile_path != NULL) {
            fd_in = open(infile_path, O_RDONLY, S_IRUSR);
            if (fd_in < 0)
                goto fail;
            dup2(fd_in, STDIN_FILENO);
            close(fd_in);
        }
        if (outfile_path != NULL) {
            fd_out = open(outfile_path, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
            if (fd_out < 0)
                goto fail;
            dup2(fd_out, STDOUT_FILENO);
            close(fd_out);
        }
        if (errfile_path != NULL) {
            fd_err = open(outfile_path, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
            if (fd_err < 0)
                goto fail;
            dup2(fd_err, STDERR_FILENO);
            close(fd_err);
        }
        if (execvp("/bin/bash", process->argv))
            goto fail;
    }
    process->pid = pid;
    pthread_create(&process->wait_thread, NULL, process_handler_daemon, process);

    return process;

fail:
    puts("Something went horribly wrong");
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

#endif
