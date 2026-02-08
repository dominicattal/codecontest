#ifndef STATE_H
#define STATE_H

#include <stdlib.h>
#include <stdbool.h>
#include <json.h>
#include <pthread.h>
#include <sqlite3.h>
#include <networking.h>
#include <stdarg.h>

typedef enum {
    FATAL,
    ERROR,
    WARNING,
    INFO,
    DEBUG,
    DBQUERY,
    NUM_LOG_LEVELS
} LogLevel;

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
    char* name;
    int id;
} Testcase;

typedef struct {
    JsonObject* object;
    Testcase* testcases;
    const char* name;
    const char* dir;
    const char* testcases_dir;
    const char* html;
    const char* pdf;
    const char* validate;
    char letter;
    size_t mem_limit;
    int time_limit;
    int num_testcases;
    int id;
    bool pipe;
} Problem;

typedef struct {
    sqlite3* db;
    Team* teams;
    Language* languages;
    Problem* problems;
    pthread_t* run_threads;
    NetContext* cli_net_ctx;
    NetContext* web_net_ctx;
    int num_teams;
    int num_languages;
    int num_problems;
    int num_run_threads;
    int num_runs;
    bool kill;
} GlobalContext;

extern GlobalContext ctx;

void _log(LogLevel level, const char* fmt, const char* file, int line, ...);
#define log(level, fmt, ...) _log(level, fmt, __FILE__, __LINE__, ##__VA_ARGS__)

bool _db_exec(char* query_fmt, const char* file, int line, ...);
#define db_exec(query_fmt, ...) _db_exec(query_fmt, __FILE__, __LINE__, ##__VA_ARGS__)

bool create_file(const char* path, const char* code, int code_length);
bool create_dir(const char* path);
bool find_dir(const char* path);

#endif
