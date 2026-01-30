<?php
    include "header.php";
?>

<?php
    include "create_arrays.php";
    include "run_enum.php";

    $success_query = "SELECT COUNT(*) AS success 
                      FROM runs 
                      WHERE problem_id=:problem_id 
                         AND team_id=:team_id 
                         AND status=$RUN_SUCCESS";
    $wrong_query = "SELECT COUNT(*) AS wrong 
                    FROM runs 
                    WHERE problem_id=:problem_id 
                        AND team_id=:team_id 
                        AND status BETWEEN $RUN_RUNTIME_ERROR AND $RUN_WRONG_ANSWER";

    $db = new SQLite3("../problems/runs.db");
    $db->enableExceptions(true);
    $db->exec('PRAGMA journal_mode = wal;');
    echo "<table id='standings-table'>";
    echo "<thead><tr>";
    echo "<th scope='col'>Team</th>";
    foreach ($problems as $id => $problem)
        echo "<th scope='col'>$problem[letter]</th>";
    echo "</tr></thead>";
    echo "<tbody>";
    foreach ($teams as $team_id => $username) {
        echo "<tr>";
        echo "<td>$username</td>";
        foreach ($problems as $problem_id => $problem) {
            $stmt = $db->prepare($success_query);
            $stmt->bindParam(':problem_id', $problem_id);
            $stmt->bindParam(':team_id', $team_id);
            $res = $stmt->execute();
            $success = $res->fetchArray(SQLITE3_ASSOC)["success"];
            $res->finalize();
            $stmt = $db->prepare($wrong_query);
            $stmt->bindParam(':problem_id', $problem_id);
            $stmt->bindParam(':team_id', $team_id);
            $res = $stmt->execute();
            $wrong = $res->fetchArray(SQLITE3_ASSOC)["wrong"];
            $res->finalize();
            $total = $success + $wrong;
            if ($total != 0)
                echo "<td scope='col'>$success/$total</td>";
            else
                echo "<td scope='col'>--/--</td>";
        }
        echo "</tr>";
    }
    echo "</tbody></table>";
    $db->close();
?>
