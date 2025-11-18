#ifndef PROCESS_H
#define PROCESS_H

#include <stdbool.h>
#include <stdlib.h>

typedef struct ProcessID ProcessID;

void        process_init(void);
ProcessID*  process_create(const char* command, const char* infile_path, const char* outfile_path);
void        process_wait(ProcessID* pid);
bool        process_running(ProcessID* pid);
size_t      process_memory(ProcessID* pid);
bool        process_success(ProcessID* pid);
void        process_destroy(ProcessID* pid);
void        process_cleanup(void);

#endif


