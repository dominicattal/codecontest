## Configuration file for server

"ip" -> must be either null or properly formatted ip address ("127.0.0.1") \
"port" -> must be a valid port number formatted as a string ("27105") \
"teams" -> must be an array containing each team's information \
    "username" -> name of the team \
    "password" -> password for the team \
"languages" -> must be an array containing each valid language and its build instructions \
    "language" -> name of the language \
    "extension" -> filename extension for the language. defaults to .txt if this is missing \
    "compile" -> command for compiling a file with this language \
    "execute" -> command for executing a file with this language \
"problems" -> must be an array containing each problem's information \
    "name" -> name of the problem. clients will see exactly what is put here \
    "dir" -> directory of the problem's information \
    "validate" -> command for validating a program's output \
    "testcases" -> number of testcases for this problem \
    "time_limit" -> time limit in seconds \
    "mem_limit" -> memory limit in MB

A command is a string that is to be run in the system's shell. \
If any of the following tokens appear in the command, they will be expanded: \
- [PROBLEM_DIR]         -> directory of the problem
- [CASE_DIR_NAME]       -> name of the directory's case dir, which contains the testcases. default is cases
- [CASE_DIR]            -> directory of the problem's case dir, which contains each testcase. equivalent to [PROBLEM_DIR]/[CASE_DIR_NAME]
- [RUN_DIR_NAME]        -> name of the directory that stores all of the runs. default is runs
- [RUN_DIR]             -> directory of the problem's run dir, which contains each run. equivalent to [PROBLEM_DIR]/[RUN_DIR_NAME]
- [COMPILE_DIR_NAME]    -> name of the directory that stores the compilation outputs. default is runs
- [COMPILE_DIR]         -> directory of the problem's compile dir. equivalent to [PROBLEM_DIR]/[COMPILE_DIR_NAME]
- [BIN_DIR_NAME]        -> name of the bin directory to store compiled files. default is bin
- [BIN_DIR]             -> directory of the problem's bin dir. equivalent to [PROBLEM_DIR]/[BIN_DIR_NAME]
- [TEAM_NAME]           -> name of the team submitting
- [RUN_ID]              -> global id of the run
- [RUN_PID]             -> id of the run local to problem
- [MINUTE]              -> minute of the contest the run was submitted on
- [LANGUAGE_EXT]        -> extension of the language used.
- [CODE_FILENAME]       -> filename as it appears on client's machine, extension not included
- [BASENAME]            -> basename of the run. equivalent to [TEAM_NAME]-[RUN_ID]
- [CODE_DIR]            -> directory of the code file generated. default is runs
- [CODE_PATH]           -> path to the code file generated. The default is [CODE_DIR]/[BASENAME].[LANGUAGE_EXT]
- [COMPILE_PATH]        -> path to the compile output file generated. This is equivalent to [COMPILE_DIR]/[BASENAME].compile
- [TESTCASE]            -> current testcase number. equal to -1 if not used in the "validate" or the "execute" fields
- [CASE_PATH]           -> file that contains current testcase. equivalent to [CASE_DIR]/[TESTCASE].in
- [OUTPUT_DIR]          -> path to the dir that contains the output files. Equivalent to [RUN_DIR]/[BASENAME]
- [OUTPUT_PATH]         -> path to the textfile generated after executing. This is equivalent to [OUTPUT_DIR]/[TESTCASE].output.

When a run is submitted from a client, the server places it in the run queue. When a run is processed, \
it creates a file with the code called [CODE_PATH]. The code is then compiled based on the language's execute command, \
and the output is stored in [COMPILE_PATH]. If compilation succeeds, then the code will try to run each testcase. \
For each testcase, the execute command will execute. Each testcase's output (stdout) will be stored to [OUTPUT_PATH]. \
If the run exceeds the time limit, exceeds the memory limit, or has a runtime error, and no more testcases will be run. \
Otherwise, the problem's validate function will run for the current testcase; the validator *should* read the file from [OUTPUT_PATH] \
and check if it is correct. The validator's implementation is left up to the server admin, but it should conform with these rules:
- If the output is correct, it should exit with a code of 0.
- If the output is wrong, it should exit with a code of 1.
- Otherwise, such as when something wrong happened, it should exit with a code of 2, indicating a server error.
- The output path and testcase number should be passed in as arguments to the validator.
- The answer files should be in [CASE_DIR], and each answer file should be labeled as [TESTCASE].ans 
If the validator determines the output is wrong or if there is an error, no more testcases will be run. Otherwise, the run will \
continue to the next testcase. If no errors occur during this process, the run will succeed.

If at any point this process failed unexpectedly, the process ends and the run's status is set to server error.

Once a run stops running testcases, it will let the client know the verdict. All information related to this run (output path, compile path, etc.)
will persist.

The tokens above may be overrided in certain situations. The order they are overrided in is undefined,
so do not rely on the tokens being expanded in a particular order.
- [CODE_DIR] -> in a language
- [CODE_PATH] -> in a language
