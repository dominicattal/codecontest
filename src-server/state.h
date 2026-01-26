#ifndef STATE_H
#define STATE_H

#include <stdlib.h>
#include <stdbool.h>
#include <json.h>
#include <pthread.h>
#include <sqlite3.h>

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
    const char* html;
    const char* validate;
    char letter;
    size_t mem_limit;
    int time_limit;
    int num_testcases;
    int id;
} Problem;

typedef struct {
    sqlite3* db;
    Team* teams;
    Language* languages;
    Problem* problems;
    pthread_t* run_threads;
    int num_teams;
    int num_languages;
    int num_problems;
    int num_run_threads;
    int num_runs;
    bool kill;
} GlobalContext;

extern GlobalContext ctx;

bool db_exec(char* query_fmt, ...);

#endif
