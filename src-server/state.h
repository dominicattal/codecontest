#ifndef STATE_H
#define STATE_H

#include <stdlib.h>

typedef struct {
    const char* username;
    const char* password;
    int id;
} Team;

typedef struct {
    const char* name;
    const char* extension;
    const char* compile;
    const char* execute;
    int id;
} Language;

typedef struct {
    const char* name;
    const char* dir;
    const char* validator;
    size_t mem_limit;
    double time_limit;
    int num_testcases;
    int id;
} Problem;

typedef struct {
    int num_teams;
    int num_languages;
    int num_problems;
    Team* teams;
    Language* languages;
    Problem* problems;
} GlobalContext;

extern GlobalContext ctx;

#endif
