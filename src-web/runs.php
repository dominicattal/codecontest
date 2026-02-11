<?php
    require_once "header.php";
    require_once "config.php";
    require_once "create_arrays.php";
    require_once "run_enum.php";

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
    $db = new SQLite3($config["database"]);
    $db->enableExceptions(true);
    $db->busyTimeout(5000);
    $db->exec('PRAGMA journal_mode = wal;');
    $stmt = $db->prepare("SELECT * FROM runs ORDER BY id DESC");
    $res = $stmt->execute();
    $run = $res->fetchArray(SQLITE3_ASSOC);
    while ($run) {
        if ($run["timestamp"] <= $contest["freeze"]) {
          echo "<tr id='$run[id]'>";
          echo "<td id='id-$run[id]'>$run[id]</td>";
          $when = $run["timestamp"] - $contest["start"];
          $hour = intdiv($when, 3600);
          $minute = intdiv($when%3600, 60);
          $second = $when%60;
          $str = sprintf("%d:%02d:%02d", $hour, $minute, $second);
          echo "<td>$str</td>";
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
        }
        $run = $res->fetchArray(SQLITE3_ASSOC);
    }
    $res->finalize();
    $db->close();
?>
    </tbody>
</table>
<?php include "footer.php"; ?>
