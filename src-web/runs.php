<?php
    include "header.php";
    include "create_arrays.php";

    $db = new SQLite3("../problems/runs.db", SQLITE3_OPEN_READONLY);
    $db->enableExceptions(true);
    $stmt = $db->prepare("SELECT * FROM runs ORDER BY id DESC");
    $res = $stmt->execute();
    $row = $res->fetchArray(SQLITE3_ASSOC);
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
    while ($row) {
        echo "<tr>";
        echo "<td>$row[id]</td>";
        echo "<td>TDB</td>";
        echo "<td>{$teams[$row['team_id']]}</td>";
        $problem_letter = $problems[$row['problem_id']]["letter"];
        $problem_name = $problems[$row['problem_id']]["name"];
        echo "<td>$problem_letter - $problem_name</td>";
        echo "<td>{$langs[$row['language_id']]}</td>";
        echo "<td>TBD</td>";
        echo "<td>TBD</td>";
        echo "<td>TBD</td>";
        echo "</tr>";
        $row = $res->fetchArray(SQLITE3_ASSOC);
    }
    echo "</tbody>";
    echo "</table>";
    $res->finalize();

    $db->close();
?>
