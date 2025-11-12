#include "process.h"
#include <stdio.h>

#ifdef __WIN32
#include <windows.h>

typedef struct ProcessID {
    PROCESS_INFORMATION pi;
} ProcessID;

ProcessID* process_create(char* command, char* outfile_path)
{
    STARTUPINFO si;
    ProcessID* pid;
    SECURITY_ATTRIBUTES sa;
    HANDLE h;
    DWORD flags;
    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;
    h = CreateFile(outfile_path, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 
            &sa, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    flags = CREATE_NO_WINDOW;
    pid = malloc(sizeof(ProcessID));
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pid->pi, sizeof(pid->pi));
    si.cb = sizeof(si);
    si.dwFlags |= STARTF_USESTDHANDLES;
    si.hStdInput = NULL;
    si.hStdError = h;
    si.hStdOutput = h;
    if (!CreateProcess(NULL, command, NULL, NULL, TRUE, flags, NULL, NULL, &si, &pid->pi)) {
        puts("Something went very wrong");
        printf("%lu\n", GetLastError());
        exit(1);
    }
    CloseHandle(h);
    return pid;
}

void process_wait(ProcessID* pid)
{
    WaitForSingleObject(pid->pi.hProcess, INFINITE);
}

bool process_success(ProcessID* pid)
{
    DWORD lpExitCode;
    GetExitCodeProcess(pid->pi.hProcess, &lpExitCode);
    printf("%lu\n", lpExitCode);
    return lpExitCode == 0;
}

void process_destroy(ProcessID* pid)
{
    CloseHandle(pid->pi.hProcess);
    CloseHandle(pid->pi.hThread);
    free(pid);
}

#endif
