<?php
$RUN_IDLE = 0;
$RUN_ENQUEUED = 1;
$RUN_COMPILING = 2;
$RUN_RUNNING = 3;
$RUN_SUCCESS = 4;
$RUN_COMPILATION_ERROR = 5;
$RUN_RUNTIME_ERROR = 6;
$RUN_TIME_LIMIT_EXCEEDED = 7;
$RUN_MEM_LIMIT_EXCEEDED = 8;
$RUN_WRONG_ANSWER = 9;
$RUN_SERVER_ERROR = 10;
$RUN_DEAD = 11;
function run_status_str($run)
{
    global 
        $RUN_IDLE,
        $RUN_ENQUEUED,
        $RUN_COMPILING,
        $RUN_RUNNING,
        $RUN_SUCCESS,
        $RUN_COMPILATION_ERROR,
        $RUN_RUNTIME_ERROR,
        $RUN_TIME_LIMIT_EXCEEDED,
        $RUN_MEM_LIMIT_EXCEEDED,
        $RUN_WRONG_ANSWER,
        $RUN_SERVER_ERROR,
        $RUN_DEAD;

    $testcase = $run["testcase"];
    switch ($run["status"]) {
    case $RUN_IDLE: return "Idle";
    case $RUN_ENQUEUED: return "In queue";
    case $RUN_COMPILING: return "Compiling";
    case $RUN_RUNNING: return "Running on testcase $testcase";
    case $RUN_SUCCESS: return "Accepted";
    case $RUN_COMPILATION_ERROR: return "Compilation failed";
    case $RUN_RUNTIME_ERROR: return "Runtime error on testcase $testcase";
    case $RUN_TIME_LIMIT_EXCEEDED: return "Time limit exceeded on testcase $testcase";
    case $RUN_MEM_LIMIT_EXCEEDED: return "Memory limit exceeded testcase $testcase";
    case $RUN_WRONG_ANSWER: return "Wrong answer on testcase $testcase";
    case $RUN_SERVER_ERROR: return "Server error";
    case $RUN_DEAD: return "How tf did this happen";
    }
}
?>
