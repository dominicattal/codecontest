#include "process.h"
#include <stdio.h>

#define STATUS_SUCCESS 0
#define STATUS_FAILED  1
#define STATUS_ERROR   2
#define STATUS_RUNNING 3

#ifdef __WIN32
#include <windows.h>
#include <psapi.h>

typedef struct Process {
    PROCESS_INFORMATION pi;
    HANDLE infile;
    HANDLE infile_rd;
} Process;

char* read_file(const char* path, int* size)
{
    FILE* file;
    char* buffer;
    int end, length;
    file = fopen(path, "r");
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

Process* process_create(const char* command, const char* infile_path, const char* outfile_path)
{
    STARTUPINFO si;
    Process* process;
    char* text;
    int size;
    DWORD written;
    SECURITY_ATTRIBUTES sa, saAttr;
    HANDLE h;
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
    si.cb = sizeof(si);
    si.dwFlags |= STARTF_USESTDHANDLES;
    if (infile_path != NULL) {
        // this is aids
        CreatePipe(&process->infile_rd, &infile_wr, &saAttr, 0);
        SetHandleInformation(&infile_wr, HANDLE_FLAG_INHERIT, 0);
        text = read_file(infile_path, &size);
        WriteFile(infile_wr, text, size, &written, NULL);
        free(text);
        CloseHandle(infile_wr);
        si.hStdInput = process->infile_rd;
    }
    if (outfile_path != NULL) {
        h = CreateFile(outfile_path, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 
            &sa, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        si.hStdOutput = h;
        si.hStdError = h;
    }
    puts(command);
    if (!CreateProcess(NULL, (char*)command, NULL, NULL, TRUE, flags, NULL, NULL, &si, &process->pi))
        goto fail_really_bad;
    CloseHandle(h);
    return process;

fail_really_bad:
    puts("Something went very wrong");
    printf("%lu\n", GetLastError());
    exit(1);
    return NULL;
}

void process_wait(Process* process)
{
    WaitForSingleObject(process->pi.hProcess, INFINITE);
}

bool process_running(Process* process)
{
    DWORD lpExitCode;
    GetExitCodeProcess(process->pi.hProcess, &lpExitCode);
    return lpExitCode == STILL_ACTIVE;
}

size_t process_memory(Process* process)
{
    PROCESS_MEMORY_COUNTERS ppsemCounters;
    GetProcessMemoryInfo(process->pi.hProcess, &ppsemCounters, sizeof(ppsemCounters));
    return ((size_t)ppsemCounters.PeakWorkingSetSize)<<10;
}

bool process_success(Process* process)
{
    DWORD lpExitCode;
    GetExitCodeProcess(process->pi.hProcess, &lpExitCode);
    return lpExitCode == STATUS_SUCCESS;
}

bool process_error(Process* process)
{
    DWORD lpExitCode;
    GetExitCodeProcess(process->pi.hProcess, &lpExitCode);
    return lpExitCode == STATUS_ERROR;
}

void process_destroy(Process* process)
{
    TerminateProcess(process->pi.hProcess, 1);
    CloseHandle(process->infile);
    CloseHandle(process->infile_rd);
    CloseHandle(process->pi.hProcess);
    CloseHandle(process->pi.hThread);
    free(process);
}

#else

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <spawn.h>
#include <ctype.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

typedef struct Process {
    char** argv;
    pthread_t thread_id;
    pid_t pid;
    int status;
} Process;

static void* process_handler_daemon(void* vargp)
{
    Process* process;
    int status;
    process = vargp;
    waitpid(process->pid, &status, 0);
    process->status = WEXITSTATUS(status);
    return NULL;
}

Process* process_create(const char* command, const char* infile_path, const char* outfile_path)
{
    Process* process;
    pid_t pid;
    int fd_in, fd_out;

    process = malloc(sizeof(Process));
    process->argv = malloc(4 * sizeof(char*));
    process->argv[0] = "/bin/bash";
    process->argv[1] = "-c";
    process->argv[2] = (char*)command;
    process->argv[3] = NULL;
    process->status = STATUS_RUNNING;
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
    pthread_create(&process->thread_id, NULL, process_handler_daemon, process);

    return process;

fail:
    puts("Something went horribly wrong");
    return NULL;
}

void process_wait(Process* process)
{
    pthread_join(process->thread_id, 0);
}

bool process_running(Process* process)
{
    return process->status == STATUS_RUNNING;
}

size_t process_memory(Process* process)
{
    pid_t mem_pid;
    int status;
    int pipe_fd[2];
    char pid[16];
    char* output;
    char vsz[16];
    size_t res = 0;
    pipe(pipe_fd);
    sprintf(pid, "%d", process->pid);
    mem_pid = fork();
    if (mem_pid == -1)
        return 0;
    else if (mem_pid == 0) {
        dup2(pipe_fd[1], 1);
        dup2(pipe_fd[1], 2);
        execl("/bin/ps", "/bin/ps", "-p", pid, "-o", "vsz", NULL);
    }
    waitpid(mem_pid, &status, 0);
    output = calloc(256, sizeof(char));
    read(pipe_fd[0], output, 256);
    sscanf(output, "%s %lu", vsz, &res);
    free(output);
    close(pipe_fd[0]);
    close(pipe_fd[1]);
    return res;
}

bool process_success(Process* process)
{
    return process->status == STATUS_SUCCESS;
}

bool process_error(Process* process)
{
    return process->status == STATUS_ERROR;
}

void process_destroy(Process* process)
{
    kill(process->pid, SIGTERM);
    pthread_kill(process->thread_id, 69);
    free(process->argv);
    free(process);
}

#endif
