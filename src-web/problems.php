<?php
  include "header.php";
  include "create_arrays.php";
  include "run_enum.php";
?>
<div id='problems'>
  <div id='problems-left'>
  <?php
  foreach ($problems as $id => $problem) {
    echo "<div id='problem-$problem[letter]' hidden>";
    include "../$problem[html]";
    echo "</div>";
  }
  ?>
  </div>
  <div id='problems-right'>
    <div id='problems-list'>
      <table id='problems-table'>
        <thead>
          <tr>
            <th scope='col'>ID</th>
            <th scope='col'>Name</th>
            <th scope='col'>Num Solved</th>
          </tr>
        </thead>
        <tbody>
        <?php
          $db = new SQLite3("../runs.db");
          $db->enableExceptions(true);
          $db->busyTimeout(5000);
          $db->exec('PRAGMA journal_mode = wal;');
          $team_id = $team_to_id[$_SESSION["username"]];
          foreach ($problems as $problem_id => $problem) {
            $success = 0;
            $failed = 0;
            if (isset($team_id)) {
              $stmt = $db->prepare("SELECT COUNT(*) success
                                    FROM runs
                                    WHERE problem_id=:problem_id
                                      AND status=$RUN_SUCCESS
                                      AND team_id=:team_id");
              $stmt->bindParam(':problem_id', $problem_id);
              $stmt->bindParam(':team_id', $team_id);
              $res = $stmt->execute();
              $success = $res->fetchArray(SQLITE3_ASSOC)["success"];
              $res->finalize();
              $stmt = $db->prepare("SELECT COUNT(*) failed
                                    FROM runs
                                    WHERE problem_id=:problem_id
                                      AND team_id=:team_id
                                      AND status BETWEEN $RUN_RUNTIME_ERROR AND $RUN_WRONG_ANSWER");
              $stmt->bindParam(':problem_id', $problem_id);
              $stmt->bindParam(':team_id', $team_id);
              $res = $stmt->execute();
              $failed = $res->fetchArray(SQLITE3_ASSOC)["failed"];
              $res->finalize();
            }
            $stmt = $db->prepare("SELECT COUNT(DISTINCT team_id) solved
                                  FROM runs
                                  WHERE problem_id=:problem_id
                                    AND status=$RUN_SUCCESS");
            $stmt->bindParam(':problem_id', $problem_id);
            $res = $stmt->execute();
            $solved = $res->fetchArray(SQLITE3_ASSOC)["solved"];
            $res->finalize();
            $class = 'problem-not-attempted';
            if ($success > 0)
              $class = 'problem-success';
            else if ($failed > 0)
              $class = 'problem-failed';
            echo "<tr id='problem-table-$problem[letter]' class='$class'>";
            echo "<td scope='col'>$problem[letter]</td>";
            echo "<td scope='col' class='td-click'><a onclick=showProblem('$problem[letter]')><button>$problem[name]</button></a></td>";
            echo "<td scope='col'>$solved</td>";
            echo "</tr>";
          }
          $db->close();
        ?>
        </tbody>
      </table>
    </div>
    <div id='problems-submit'>
      <form action="submit_handler.php" method="post" enctype="multipart/form-data">
        <table id='problems-submit-table'>
          <tbody>
            <tr>
              <td><label for="problem">Problem</label></td>
              <td>
                <select id="problem" name="problem">
                  <option value="">Choose Problem</option>
                  <?php
                    foreach ($problems as $id => $problem)
                      echo "<option value='$problem[letter]'>$problem[letter] - $problem[name]</option>";
                  ?>
                </select>
              </td>
            </tr>
            <tr>
              <td><label for="language">Language</label></td>
              <td>
                <select id="language" name="language">
                  <option value="">Choose Language</option>
                  <?php
                    foreach ($langs as $id => $lang)
                      echo "<option value='$lang'>$lang</option>";
                  ?>
                </select>
              </td>
            </tr>
            <tr>
              <td><label for="code">File</label></td>
              <td>
                <input type="hidden" name="MAX_FILE_SIZE" value="30000" />
                <input type="file" id="code" name="code"></input>
              </td>
            </tr>
          </tbody>
          <tfoot>
            <tr>
              <td colspan="2"><input type="submit" value="Submit" id="submit"></input></td>
            </tr>
          </tfoot>
        </table>
      </form>
    </div>
    <?php
      if (!isset($_SESSION["username"]))
        goto done;
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

      $db = new SQLite3("../runs.db");
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

      done:
      echo "</tbody>";
      echo "</table>";
    ?>
  </div>
</div>
<script>
  var cur_shown = null;
  function showProblem(letter) {
    if (cur_shown) cur_shown.setAttribute("hidden", "");
    ele = document.getElementById(`problem-${letter}`);
    ele.removeAttribute("hidden");
    cur_shown = ele;
  }
  showProblem('A');

  const TEAM = "<?php echo $_SESSION['username']; ?>";

  var host = "ws://127.0.0.1:27106";
  var socket = new WebSocket(host);
  var runs_table = document.getElementById('runs-table');
  if (runs_table) {
    var table_body = runs_table.getElementsByTagName('tbody')[0];
  }

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

  var teams_solved = new Map();
  <?php
    $db = new SQLite3("../runs.db");
    $db->enableExceptions(true);
    $db->busyTimeout(5000);
    $db->exec('PRAGMA journal_mode = wal;');
    foreach ($teams as $team_id => $team) { 
      foreach ($problems as $problem_id => $problem) {
        $stmt = $db->prepare("SELECT COUNT(DISTINCT team_id) count FROM runs WHERE team_id=:team_id AND problem_id=:problem_id");
        $stmt->bindParam(':team_id', $team_id);
        $stmt->bindParam(':problem_id', $problem_id);
        $res = $stmt->execute();
        $solved = ($res->fetchArray(SQLITE3_ASSOC)["count"] != 0) ? "true" : "false";
        echo "teams_solved.set('$team-$problem[letter]', $solved);\n";
        $res->finalize();
      }
    }
    $db->close(); 
  ?>
  console.log(teams_solved);

  function status_to_text(stat, testcase) {
      
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

  function status_failed(stat) {
    return stat >= RUN_COMPILATION_ERROR && stat <= RUN_WRONG_ANSWER;
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

  function update_table_body(id, stat, testcase, letter, problem, lang, team, time, memory) {
    tr = document.getElementById(`${id}`);
      if (tr) {
          update_table_row(tr, id, stat, testcase, time, memory);
      } else {
          new_tr = create_table_row(id, stat, testcase, letter, problem, lang, team, time, memory);
          table_body.insertBefore(new_tr, table_body.children[0]);
          //table_body.children[0].insertBefore(new_tr, 0);
          console.log(table_body.children.length);
          if (table_body.children.length > 10) {
            var child = table_body.children[table_body.children.length-1];
            table_body.removeChild(child);
            child.remove();
          }
      }
  }

  function update_problem_table(stat, letter, team) {
    tr = document.getElementById(`problem-table-${letter}`);
    if (stat == RUN_SUCCESS) {
      key = `${team}-${letter}`;
      if (!teams_solved[key]) {
        teams_solved[key] = true;
        td = tr.children[2];
        td.textContent = parseInt(td.textContent)+1;
      }
    }
    if (tr.className == "problem-success")
      return;
    tr.removeAttribute("class");
    if (stat == RUN_SUCCESS) {
      tr.setAttribute("class", "problem-success");
    } else if (status_failed(stat)) {
      tr.setAttribute("class", "problem-failed");
    }
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
      if (table_body && team == TEAM) 
        update_table_body(id, stat, testcase, letter, problem, lang, team, time, memory);
      update_problem_table(stat, letter, team);
  }
  socket.onclose = (e) => {
      console.log(e);
  }
  socket.onerror = (e) => {
      console.log(e);
  }
</script>
