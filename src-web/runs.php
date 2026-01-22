<?php
    include "header.php";
    include "create_arrays.php";
    require "run_enum.php";

    $db = new SQLite3("../problems/runs.db");
    $db->enableExceptions(true);
    $db->busyTimeout(5000);
    $db->exec('PRAGMA journal_mode = wal;');
    $stmt = $db->prepare("SELECT * FROM runs ORDER BY id DESC");
    $res = $stmt->execute();
    $run = $res->fetchArray(SQLITE3_ASSOC);
    echo "<table id='runs-table'>";
    echo "<thead><tr>";
    echo "<th scope='col'>ID</th>";
    echo "<th scope='col'>When</th>";
    echo "<th scope='col'>Team</th>";
    echo "<th scope='col'>Problem</th>";
    echo "<th scope='col'>Language</th>";
    echo "<th scope='col'>Verdict</th>";
    echo "<th scope='col'>Time</th>";
    echo "<th scope='col'>Memory</th>";
    echo "</tr></thead>";
    echo "<tbody>";
    while ($run) {
        echo "<tr>";
        echo "<td>$run[id]</td>";
        echo "<td>TDB</td>";
        echo "<td>{$teams[$run['team_id']]}</td>";
        $problem_letter = $problems[$run['problem_id']]["letter"];
        $problem_name = $problems[$run['problem_id']]["name"];
        echo "<td>$problem_letter - $problem_name</td>";
        echo "<td>{$langs[$run['language_id']]}</td>";
        $status_str = run_status_str($run);
        echo "<td>$status_str</td>";
        echo "<td>$run[time]</td>";
        echo "<td>$run[memory]</td>";
        echo "</tr>";
        $run = $res->fetchArray(SQLITE3_ASSOC);
    }
    echo "</tbody>";
    echo "</table>";
    $res->finalize();

    $db->close();
?>
