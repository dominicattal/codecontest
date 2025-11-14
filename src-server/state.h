#ifndef STATE_H
#define STATE_H

#include <stdlib.h>
#include <json.h>

typedef struct {
    const char* username;
    const char* password;
    int id;
} Team;

typedef struct {
    JsonObject* object;
    const char* name;
    const char* extension;
    const char* compile;
    const char* execute;
    int id;
} Language;

typedef struct {
    JsonObject* object;
    const char* name;
    const char* dir;
    const char* validate;
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
