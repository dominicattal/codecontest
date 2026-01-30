<?php
    include "header.php";
?>
<body>
<table id='runs-table'>
  <thead>
    <tr>
      <th scope='col'>ID</th>
      <th scope='col'>When</th>
      <th scope='col'>Team</th>
      <th scope='col'>Problem</th>
      <th scope='col'>Language</th>
      <th scope='col'>Verdict</th>
      <th scope='col'>Time</th>
      <th scope='col'>Memory</th>
    </tr>
  </thead>
  <tbody>
<?php
    include "create_arrays.php";
    require "run_enum.php";

    $db = new SQLite3("../problems/runs.db");
    $db->enableExceptions(true);
    $db->busyTimeout(5000);
    $db->exec('PRAGMA journal_mode = wal;');
    $stmt = $db->prepare("SELECT * FROM runs ORDER BY id DESC");
    $res = $stmt->execute();
    $run = $res->fetchArray(SQLITE3_ASSOC);
    while ($run) {
        echo "<tr id='$run[id]'>";
        echo "<td id='id-$run[id]'>$run[id]</td>";
        echo "<td>TDB</td>";
        echo "<td id='team-$run[id]'>{$teams[$run['team_id']]}</td>";
        $problem_letter = $problems[$run['problem_id']]["letter"];
        $problem_name = $problems[$run['problem_id']]["name"];
        echo "<td id='problem-$run[id]'>$problem_letter - $problem_name</td>";
        echo "<td id='lang-$run[id]'>{$langs[$run['language_id']]}</td>";
        $status_str = run_status_str($run);
        echo "<td id='verdict-$run[id]'>$status_str</td>";
        echo "<td id='time-$run[id]'>$run[time] ms</td>";
        echo "<td id='mem-$run[id]'>$run[memory] KB</td>";
        echo "</tr>";
        $run = $res->fetchArray(SQLITE3_ASSOC);
    }
    $res->finalize();
    $db->close();
?>
    </tbody>
</table>

<script>

var host = "ws://127.0.0.1:27106";
var socket = new WebSocket(host);
const table_body = document.getElementsByTagName('tbody')[0];

function status_to_text(stat, testcase) {
    const RUN_IDLE = 0;
    const RUN_ENQUEUED = 1;
    const RUN_COMPILING = 2;
    const RUN_RUNNING = 3;
    const RUN_SUCCESS = 4;
    const RUN_COMPILATION_ERROR = 5;
    const RUN_RUNTIME_ERROR = 6;
    const RUN_TIME_LIMIT_EXCEEDED = 7;
    const RUN_MEM_LIMIT_EXCEEDED = 8;
    const RUN_WRONG_ANSWER = 9;
    const RUN_SERVER_ERROR = 10;
    const RUN_DEAD = 11;
    switch (parseInt(stat)) {
    case RUN_IDLE: return "Idle";
    case RUN_ENQUEUED: return "In queue";
    case RUN_COMPILING: return "Compiling";
    case RUN_RUNNING: return `Running on testcase ${testcase}`;
    case RUN_SUCCESS: return "Accepted";
    case RUN_COMPILATION_ERROR: return "Compilation failed";
    case RUN_RUNTIME_ERROR: return `Runtime error on testcase ${testcase}`;
    case RUN_TIME_LIMIT_EXCEEDED: return `Time limit exceeded on testcase ${testcase}`;
    case RUN_MEM_LIMIT_EXCEEDED: return `Memory limit exceeded testcase ${testcase}`;
    case RUN_WRONG_ANSWER: return `Wrong answer on testcase ${testcase}`;
    case RUN_SERVER_ERROR: return "Server error";
    case RUN_DEAD: return "How tf did this happen";
    }
    return "?";
}

function create_table_row(id, stat, testcase, letter, problem, lang, team, time, memory) {
    tr = document.createElement("tr");
    tr.setAttribute("id", id);

    td = document.createElement("td");
    td_text = document.createTextNode(id);
    td.appendChild(td_text);
    td.setAttribute("id", `id-${id}`);
    tr.appendChild(td);

    td = document.createElement("td");
    td_text = document.createTextNode("N/A");
    td.appendChild(td_text);
    //td.setAttribute("id", `team-${id}`);
    tr.appendChild(td);

    td = document.createElement("td");
    td_text = document.createTextNode(team);
    td.appendChild(td_text);
    td.setAttribute("id", `team-${id}`);
    tr.appendChild(td);

    td = document.createElement("td");
    td_text = document.createTextNode(`${letter} - ${problem}`);
    td.appendChild(td_text);
    td.setAttribute("id", `problem-${id}`);
    tr.appendChild(td);

    td = document.createElement("td");
    td_text = document.createTextNode(lang);
    td.appendChild(td_text);
    td.setAttribute("id", `lang-${id}`);
    tr.appendChild(td);

    td = document.createElement("td");
    td_text = document.createTextNode(status_to_text(stat, testcase));
    td.appendChild(td_text);
    td.setAttribute("id", `verdict-${id}`);
    tr.appendChild(td);

    td = document.createElement("td");
    td_text = document.createTextNode(`${time} ms`);
    td.appendChild(td_text);
    td.setAttribute("id", `time-${id}`);
    tr.appendChild(td);

    td = document.createElement("td");
    td_text = document.createTextNode(`${memory} KB`);
    td.appendChild(td_text);
    td.setAttribute("id", `mem-${id}`);
    tr.appendChild(td);

    return tr;
}

function update_table_row(tr, id, stat, testcase, time, memory) {
    td = tr.querySelector(`#verdict-${id}`);
    if (td) td.textContent = status_to_text(stat, testcase);
    td = tr.querySelector(`#time-${id}`);
    if (td) td.textContent = `${time} ms`;
    td = tr.querySelector(`#mem-${id}`);
    if (td) td.textContent = `${memory} KB`;
}

window.addEventListener('beforeunload', function() {
    socket.close();
});

socket.onopen = (e) => {
    console.log(e);
}

socket.onmessage = (e) => {
    arr = e["data"].split(",");
    [id, stat, testcase, letter, problem, lang, team, time, memory] = arr;
    tr = document.getElementById(`${id}`);
    if (tr) {
        update_table_row(tr, id, stat, testcase, time, memory);
    } else {
        new_tr = create_table_row(id, stat, testcase, letter, problem, lang, team, time, memory);
        table_body.insertBefore(new_tr, table_body.children[0]);
        //table_body.children[0].insertBefore(new_tr, 0);
    }

}
socket.onclose = (e) => {
    console.log(e);
}
socket.onerror = (e) => {
    console.log(e);
}
</script>

</body>

