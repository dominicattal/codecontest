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

char* read_file(const char* path, int* size)
{
    FILE* file;
    char* buffer;
    int end;
    file = fopen(path, "r");
    if (file == NULL)
        goto fail;
    if (fseek(file, 0, SEEK_END) != 0)
        goto fail_close_file;
    end = ftell(file);
    *size = end;
    if (end == -1L)
        goto fail_close_file;
    buffer = malloc((end+1)*sizeof(char));
    if (buffer == NULL)
        goto fail_close_file;
    if (fseek(file, 0, SEEK_SET) != 0)
        goto fail_free_buffer;
    fread(buffer, sizeof(char), end, file);
    if (ferror(file) != 0)
        goto fail_free_buffer;
    fclose(file);
    puts(path);
    puts(buffer);
    buffer[end] = '\0';
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
    ProcessID* pid;
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
    pid = malloc(sizeof(ProcessID));
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pid->pi, sizeof(pid->pi));
    pid->infile = NULL;
    si.cb = sizeof(si);
    si.dwFlags |= STARTF_USESTDHANDLES;
    if (infile_path != NULL) {
        // this is aids
        CreatePipe(&pid->infile_rd, &infile_wr, &saAttr, 0);
        SetHandleInformation(&infile_wr, HANDLE_FLAG_INHERIT, 0);
        text = read_file(infile_path, &size);
        WriteFile(infile_wr, text, size, &written, NULL);
        free(text);
        CloseHandle(infile_wr);
        si.hStdInput = pid->infile_rd;
    }
    if (outfile_path != NULL) {
        h = CreateFile(outfile_path, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 
            &sa, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        si.hStdOutput = h;
        si.hStdError = h;
    }
    if (!CreateProcess(NULL, (char*)command, NULL, NULL, TRUE, flags, NULL, NULL, &si, &pid->pi))
        goto fail_really_bad;
    CloseHandle(h);
    return pid;

fail_really_bad:
    puts("Something went very wrong");
    printf("%lu\n", GetLastError());
    exit(1);
    return NULL;
}

void process_wait(ProcessID* pid)
{
    WaitForSingleObject(pid->pi.hProcess, INFINITE);
}

bool process_running(ProcessID* pid)
{
    DWORD lpExitCode;
    GetExitCodeProcess(pid->pi.hProcess, &lpExitCode);
    return lpExitCode == STILL_ACTIVE;
}

size_t process_memory(ProcessID* pid)
{
    PROCESS_MEMORY_COUNTERS ppsemCounters;
    GetProcessMemoryInfo(pid->pi.hProcess, &ppsemCounters, sizeof(ppsemCounters));
    return (size_t)ppsemCounters.PeakWorkingSetSize;
}

bool process_success(ProcessID* pid)
{
    DWORD lpExitCode;
    GetExitCodeProcess(pid->pi.hProcess, &lpExitCode);
    return lpExitCode == 0;
}

void process_destroy(ProcessID* pid)
{
    TerminateProcess(pid->pi.hProcess, 1);
    CloseHandle(pid->infile);
    CloseHandle(pid->infile_rd);
    CloseHandle(pid->pi.hProcess);
    CloseHandle(pid->pi.hThread);
    free(pid);
}

#endif
