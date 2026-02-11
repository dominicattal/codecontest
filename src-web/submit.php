<?php
session_start();
if (!isset($_SESSION["loggedin"])) {
  header("Location: login.php");
  die();
}
require_once "header.php";
require_once "create_arrays.php";
require_once "config.php";
?>

<style>
#problems-submit {
  margin-top: 10px;
  display: flex;
  margin-left: auto;
  margin-right: auto;
  justify-content: center;
}

#problems-submit-table {
  margin-left: auto;
  margin-right: auto;
  border-collapse: collapse;
  border: 2px solid rgb(140 140 140);
  letter-spacing: 1px;
}

#problems-submit-table tfoot {
  text-align: center;
}
</style>

<div>
  <?php
  include "submit_form.php";
  ?>
  <div>
    <?php
      include "run_enum.php";
      echo "<table id='runs-table'>";
      echo "<thead>";
      echo "<tr>";
      echo "<th scope='col'>ID</th>";
      echo "<th scope='col'>When</th>";
      echo "<th scope='col'>Team</th>";
      echo "<th scope='col'>Problem</th>";
      echo "<th scope='col'>Language</th>";
      echo "<th scope='col'>Verdict</th>";
      echo "<th scope='col'>Time</th>";
      echo "<th scope='col'>Memory</th>";
      echo "</tr>";
      echo "</thead>";
      echo "<tbody>";

      $db = new SQLite3($config["database"]);
      $db->enableExceptions(true);
      $db->busyTimeout(5000);
      $db->exec('PRAGMA journal_mode = wal;');
      $stmt = $db->prepare("SELECT *
                            FROM runs
                            WHERE team_id=:team_id
                            ORDER BY id DESC
                            LIMIT 10");
      $stmt->bindParam(':team_id', $team_to_id[$_SESSION["username"]]);
      $res = $stmt->execute();
      $run = $res->fetchArray(SQLITE3_ASSOC);
      while ($run) {
          echo "<tr id='$run[id]'>";
          echo "<td id='id-$run[id]'>$run[id]</td>";
          echo "<td>TBD</td>";
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
      echo "</tbody>";
      echo "</table>";
    ?>
  </div>
</div>
<?php include "footer.php"; ?>
