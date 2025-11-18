#include "process.h"
#include <stdio.h>

#ifdef __WIN32
#include <windows.h>
#include <psapi.h>

typedef struct ProcessID {
    PROCESS_INFORMATION pi;
    HANDLE infile;
    HANDLE infile_rd;
} ProcessID;

void process_init(void)
{
}

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

ProcessID* process_create(const char* command, const char* infile_path, const char* outfile_path)
{
    STARTUPINFO si;
    ProcessID* process;
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
    process = malloc(sizeof(ProcessID));
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

void process_wait(ProcessID* process)
{
    WaitForSingleObject(process->pi.hProcess, INFINITE);
}

bool process_running(ProcessID* process)
{
    DWORD lpExitCode;
    GetExitCodeProcess(process->pi.hProcess, &lpExitCode);
    return lpExitCode == STILL_ACTIVE;
}

size_t process_memory(ProcessID* process)
{
    PROCESS_MEMORY_COUNTERS ppsemCounters;
    GetProcessMemoryInfo(process->pi.hProcess, &ppsemCounters, sizeof(ppsemCounters));
    return (size_t)ppsemCounters.PeakWorkingSetSize;
}

bool process_success(ProcessID* process)
{
    DWORD lpExitCode;
    GetExitCodeProcess(process->pi.hProcess, &lpExitCode);
    return lpExitCode == 0;
}

void process_destroy(ProcessID* process)
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

#define STATUS_SUCCESS 0
#define STATUS_FAILED  1
#define STATUS_ERROR   2
#define STATUS_RUNNING 3

typedef struct ProcessID {
    char** argv;
    pthread_t thread_id;
    pid_t pid;
    int status;
} ProcessID;

static void* handler_daemon(void* vargp)
{
    ProcessID* process;
    int status;
    process = vargp;
    waitpid(process->pid, &status, 0);
    process->status = WEXITSTATUS(status);
    return NULL;
}

void process_init(void)
{
    ProcessID* process;
    char* command = "python3 test.py";
    process = process_create(command, "test.in", "test.output");
    puts("start");
    while (process_running(process))
        ;
    puts("end");
    process_destroy(process);
}

ProcessID* process_create(const char* command, const char* infile_path, const char* outfile_path)
{
    ProcessID* process;
    pid_t pid;
    int fd_in, fd_out;

    process = malloc(sizeof(ProcessID));
    process->argv = malloc(4 * sizeof(char*));
    process->argv[0] = "/bin/bash";
    process->argv[1] = "-c";
    process->argv[2] = (char*)command;
    process->argv[3] = NULL;
    pid = fork();
    if (pid == -1) {
        goto fail;
    } else if (pid == 0) {
        if (infile_path != NULL) {
            fd_in = open(infile_path, O_RDONLY, S_IRUSR);
            dup2(fd_in, 0);
            close(fd_in);
        }
        if (outfile_path != NULL) {
            fd_out = open(outfile_path, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
            dup2(fd_out, 1);
            dup2(fd_out, 2);
            close(fd_out);
        }
        execvp("/bin/bash", process->argv);
    }
    process->pid = pid;
    pthread_create(&process->thread_id, NULL, handler_daemon, process);
    process->status = STATUS_RUNNING;

    return process;

fail:
    puts("Something went horribly wrong");
    return NULL;
}

void process_wait(ProcessID* process)
{
    pthread_join(process->thread_id, 0);
}

bool process_running(ProcessID* process)
{
    return process->status == STATUS_RUNNING;
}

size_t process_memory(ProcessID* process)
{
    pid_t mem_pid;
    int status;
    int pipe_fd[2];
    char pid[16];
    char output[256];
    char vsz[16];
    size_t res;
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
    read(pipe_fd[0], output, 256);
    sscanf(output, "%s %lu", vsz, &res);
    close(pipe_fd[0]);
    close(pipe_fd[1]);
    return res;
}

bool process_success(ProcessID* process)
{
    return process->status == STATUS_SUCCESS;
}

void process_destroy(ProcessID* process)
{
    kill(process->pid, 2);
    pthread_join(process->thread_id, NULL);
    free(process);
}

void process_cleanup(void)
{
}

#endif
