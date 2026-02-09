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
$problem_stats = array();
foreach ($problems as $problem_id => $problem) {
  $problem_stats[$problem_id] = array();
  $problem_stats[$problem_id]["success"] = 0;
  $problem_stats[$problem_id]["total"] = 0;
}
foreach ($teams as $team_id => $username) {
  $standings[$team_id] = array();
  $standings[$team_id]["success"] = 0;
  $standings[$team_id]["total"] = 0;
  $standings[$team_id]["solved"] = 0;
  $standings[$team_id]["time"] = 0;
  $standings[$team_id]["name"] = $teams[$team_id];
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
    //$total_all += $total;
    $standings[$team_id][$problem_id]["success"] = $success;
    $standings[$team_id][$problem_id]["total"] = $total;
    $standings[$team_id][$problem_id]["timestamp"] = 0;
    $standings[$team_id][$problem_id]["frozen"] = false;
    $standings[$team_id]["success"] += $success;
    $standings[$team_id]["total"] += $total;
    if ($total == 0) {
      continue;
    } else if ($success == 0) {
      $stmt = $db->prepare($last_run_query);
      $stmt->bindParam(':problem_id', $problem_id);
      $stmt->bindParam(':team_id', $team_id);
      $res = $stmt->execute();
      $timestamp_last_run = $res->fetchArray(SQLITE3_ASSOC)["timestamp"];
      $res->finalize();
      if ($timestamp_last_run > $contest["freeze"])
        $standings[$team_id][$problem_id]["frozen"] = true;
    } else {
      $stmt = $db->prepare($first_success_query);
      $stmt->bindParam(':problem_id', $problem_id);
      $stmt->bindParam(':team_id', $team_id);
      $res = $stmt->execute();
      $timestamp_first_success = $res->fetchArray(SQLITE3_ASSOC)["timestamp"];
      $res->finalize();
      if ($timestamp_first_success > $contest["freeze"]) {
        $standings[$team_id][$problem_id]["frozen"] = true;
      } else {
        $standings[$team_id][$problem_id]["timestamp"] = $timestamp_first_success - $contest["start"];
        $standings[$team_id]["solved"] += 1;
        $standings[$team_id]["time"] += $standings[$team_id][$problem_id]["timestamp"];
        $standings[$team_id]["time"] += $wrong * 60*20;
      }
    }
  }
}
function compare_function($a, $b)
{
  if ($a["solved"] > $b["solved"] || ($a["solved"] == $b["solved"] && $a["time"] < $b["time"]))
    return -1;
  if ($a["solved"] == $b["solved"] && $a["time"] == $b["time"])
    return 0;
  return 1;
}
usort($standings, "compare_function");
echo "<table id='standings-table'>";
echo "<thead><tr>";
echo "<th scope='col'>Rank</th>";
echo "<th scope='col'>Team</th>";
echo "<th scope='col'>Solved</th>";
echo "<th scope='col'>Time</th>";
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
  $solved = $arr["solved"];
  echo "<td>$solved</td>";
  $team_time = intdiv($arr['time'],60);
  echo "<td>$team_time</td>";
  foreach ($problems as $problem_id => $problem) {
    $success = $arr[$problem_id]["success"];
    $total = $arr[$problem_id]["total"];
    $timestamp = intdiv($arr[$problem_id]["timestamp"],60);
    if ($total == 0)
      echo "<td scope='col'>0/--</td>";
    else if ($arr[$problem_id]["frozen"])
      echo "<td scope='col' class='frozen'>$total/--</td>";
    else if ($success == 0)
      echo "<td scope='col' class='failed'>$total/--</td>";
    else  
      echo "<td scope='col' class='success'>$total/$timestamp</td>";
  }
  $solved = $arr["solved"];
  $total = $arr["total"];
  echo "<td scope='col'>$total/$solved</td>";
  echo "</tr>";
}
echo "</tbody></table>";
$db->close();

require_once "footer.php";
?>
