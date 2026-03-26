<?php
require_once "header.php";
require_once "create_arrays.php";
require_once "run_enum.php";
?>

<style>
#standings-table {
    margin-left: auto;
    margin-right: auto;
    margin-top: 15px;
    text-align: center;
    border: 2px solid rgb(140 140 140);
    font-size: 0.8rem;
    letter-spacing: 1px;
}
.success {
    background-color: #00ff00;
}
.failed {
    background-color: red;
}
.frozen {
    background-color: #a0a0a0;
}
</style>

<?php
$first_success_query = "SELECT timestamp
                        FROM runs
                        WHERE problem_id=:problem_id 
                          AND team_id=:team_id 
                          AND status=$RUN_SUCCESS
                        ORDER BY timestamp ASC
                        LIMIT 1;";
$last_run_query = "SELECT timestamp
                   FROM runs
                   WHERE problem_id=:problem_id 
                     AND team_id=:team_id 
                   ORDER BY timestamp DESC
                   LIMIT 1;";
$success_query = "SELECT COUNT(*) AS success 
                  FROM runs 
                  WHERE problem_id=:problem_id 
                    AND team_id=:team_id 
                    AND status=$RUN_SUCCESS";
$wrong_query = "SELECT COUNT(*) AS wrong 
                FROM runs 
                WHERE problem_id=:problem_id 
                  AND team_id=:team_id 
                  AND status BETWEEN $RUN_COMPILATION_ERROR AND $RUN_WRONG_ANSWER";
$db = new SQLite3($config["database"]);
$db->enableExceptions(true);
$db->exec('PRAGMA journal_mode = wal;');
$standings = array();
foreach ($teams as $team_id => $username) {
  $standings[$team_id] = array();
  $standings[$team_id]["time"] = 0;
  $standings[$team_id]["name"] = $teams[$team_id];
  foreach ($problems as $problem_id => $problem) {
    $standings[$team_id][$problem_id] = 10000;
    $query = "SELECT *
                FROM runs 
                WHERE problem_id=:problem_id 
                  AND team_id=:team_id 
                  AND status=$RUN_SUCCESS";

    $stmt = $db->prepare($query);
    $stmt->bindParam(':problem_id', $problem_id);
    $stmt->bindParam(':team_id', $team_id);
    $res = $stmt->execute();
    $run = $res->fetchArray(SQLITE3_ASSOC);
    while ($run) {
      $prev = $standings[$team_id][$problem_id];
      $cur = $run["time"];
      $standings[$team_id][$problem_id] = min($prev, $cur);
      $run = $res->fetchArray(SQLITE3_ASSOC);
    }
    $standings[$team_id]["time"] += $standings[$team_id][$problem_id];
  }
}
function compare_function($a, $b)
{
  return $a["time"] > $b["time"];
}

usort($standings, "compare_function");
echo "<table id='standings-table'>";
echo "<caption>";
echo "All times are in milliseconds";
echo "</caption>";
echo "<thead><tr>";
echo "<th scope='col'>Rank</th>";
echo "<th scope='col'>Team</th>";
foreach ($problems as $id => $problem)
  echo "<th scope='col'>$problem[letter]</th>";
echo "<th scope='col'>Total</th>";
echo "</tr></thead>";
echo "<tbody>";
$rank = 1;
foreach ($standings as $team => $arr) {
  $username = $arr["name"];
  echo "<tr>";
  echo "<td>$rank</td>";
  $rank += 1;
  echo "<td>$username</td>";
  foreach ($problems as $problem_id => $problem) {
    $time = $arr[$problem_id];
    if ($time == 10000)
      echo "<td scope='col'>N/A</td>";
    else  
      echo "<td scope='col'>$time</td>";
  }
  $time = $arr["time"];
  echo "<td scope='col'>$time</td>";
  echo "</tr>";
}
echo "</tbody>";
echo "</table>";
$db->close();
include "footer.php";
?>
