#ifndef PROCESS_H
#define PROCESS_H

#include <stdbool.h>

// cross-platform create processes
// god i hate cross platform development
// why can't we all just use linux

typedef struct ProcessID ProcessID;

ProcessID*  process_create(char* command, char* outfile_path);
void        process_wait(ProcessID* pid);
bool        process_success(ProcessID* pid);
void        process_destroy(ProcessID* pid);

#endif


