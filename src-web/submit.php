<?php
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
          <?php
          if (isset($_SESSION["message"])) {
            echo "<tr>";
            echo "<td colspan='2'>$_SESSION[message]</td>";
            echo "</tr>";
            unset($_SESSION["message"]);
          }
          ?>
        </tfoot>
      </table>
    </form>
  </div>
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
<script>
var host = "<?php echo "ws://$config[ip]:$config[web_port]"; ?>";
var socket = new WebSocket(host);
var runs_table = document.getElementById('runs-table');
if (runs_table) {
  var table_body = runs_table.getElementsByTagName('tbody')[0];
}

socket.onopen = (e) => {
    console.log(e);
}

socket.onmessage = (e) => {
    arr = e["data"].split("\r");
    [id, stat, testcase, letter, problem, lang, team, time, memory] = arr;
    if (table_body && team == TEAM) 
      update_table_body(id, stat, testcase, letter, problem, lang, team, time, memory);
}
socket.onclose = (e) => {
    console.log(e);
}
socket.onerror = (e) => {
    console.log(e);
}

const TEAM = "<?php echo $_SESSION['username']; ?>";

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
  $db = new SQLite3($config["database"]);
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
</script>
<?php
require_once "footer.php";
?>
