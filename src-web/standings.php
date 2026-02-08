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
  //$stmt = $db->prepare($num_solved_query);
  //$stmt->bindParam(':team_id', $team_id);
  //$res = $stmt->execute();
  //$solved = $res->fetchArray(SQLITE3_ASSOC)["solved"];
  //echo "<td>$solved</td>";
  //$res->finalize();
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
    $total_success += $success;
    //$total_all += $total;
    $standings[$team_id][$problem_id]["success"] = $success;
    $standings[$team_id][$problem_id]["total"] = $total;
    $standings[$team_id]["success"] += $success;
    $standings[$team_id]["total"] += $total;
    if ($success > 0)
      $standings[$team_id]["solved"] += 1;
  }
}
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
foreach ($teams as $team_id => $username) {
  echo "<tr>";
  echo "<td>0</td>";
  echo "<td>$username</td>";
  $solved = $standings[$team_id]["solved"];
  echo "<td>$solved</td>";
  echo "<td>0</td>";
  foreach ($problems as $problem_id => $problem) {
    $success = $standings[$team_id][$problem_id]["success"];
    $total = $standings[$team_id][$problem_id]["total"];
    if ($total == 0) {
      echo "<td scope='col'>--/--</td>";
      continue;
    }
    if ($success == 0)
      $class = 'failed';
    else  
      $class = 'success';
    echo "<td scope='col' class='$class'>$success/$total</td>";
  }
  $success = $standings[$team_id]["success"];
  $total = $standings[$team_id]["total"];
  echo "<td scope='col'>$success/$total</td>";
  echo "</tr>";
}
echo "</tbody></table>";
$db->close();

require_once "footer.php";
?>
