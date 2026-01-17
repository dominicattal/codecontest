#ifndef PROCESS_H
#define PROCESS_H

#include <stdbool.h>
#include <stdlib.h>

typedef struct Process Process;

Process*    process_create(const char* command, const char* infile_path, const char* outfile_path);
void        process_wait(Process* process);
bool        process_running(Process* process);
// return virtual memory size of process in KiB
size_t      process_memory(Process* process);
bool        process_success(Process* process);
bool        process_error(Process* process);
void        process_destroy(Process* process);

#endif


