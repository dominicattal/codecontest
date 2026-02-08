<?php
require_once "header.php";
require_once "create_arrays.php";
require_once "run_enum.php";
require_once "config.php";
?>
<style>
#problems {
  display: flex;
  justify-content: center;
  margin: 10px;
}

#problems-left {
  width: 60%;
  --background-color: red;
}

#problems-left img {
  display: block;
  margin: auto;
}

#problems-right {
  width: 30%;
  --background-color: blue;
}

#problems-table {
  margin-left: auto;
  margin-right: auto;
  justify-content: center;
  border-collapse: collapse;
  border: 2px solid rgb(140 140 140);
  letter-spacing: 1px;
  background-color: white;
}

#problems-table thead {
  background-color: rgb(228 240 245);
}

#problems-table th, td {
  border: 1px solid rgb(160 160 160);
  padding: 8px 10px;
}

#problems-submit {
  margin-top: 10px;
  display: flex;
  margin-left: auto;
  margin-right: auto;
  justify-content: center;
}

#problem-header {
  text-align: center;
}

#problem-header h1, h2, h3, h4, h5, h6 {
  margin: 0px;
}

#problem-header h2 {
  font-size: 20px;
}

@media only screen and (max-width: 1300px) {
  #problems-submit-table {
    display: none;
  }
}

#problems-submit-table {
  margin-left: auto;
  margin-right: auto;
  border-collapse: collapse;
  border: 2px solid rgb(140 140 140);
  letter-spacing: 1px;
  background-color: white;
  max-width:
}

#problems-submit-table tfoot {
  text-align: center;
}

.problem-success {
  background-color: #00af00;
}

.problem-failed {
  background-color: #ff9999;
}
</style>
<div id='problems'>
  <div id='problems-left'>
  <?php
  foreach ($problems as $id => $problem) {
    echo "<div id='problem-$problem[letter]' hidden>";
    //include "../$problem[html]";
    echo "<embed src='tmp/$problem[pdf]' type='application/pdf' width=100% height=1200px></embed>";
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
          $db = new SQLite3($config["database"]);
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

var host = "<?php echo "ws://$config[ip]:$config[web_port]"; ?>";

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
console.log(teams_solved);

function status_failed(stat) {
  return stat >= RUN_COMPILATION_ERROR && stat <= RUN_WRONG_ANSWER;
}

function update_problem_table(stat, letter, team) {
  tr = document.getElementById(`problem-table-${letter}`);
  if (stat == RUN_SUCCESS) {
    key = `${team}-${letter}`;
    if (!teams_solved.get(key)) {
      teams_solved.set(key, true);
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
    arr = e["data"].split("\r");
    [id, stat, testcase, letter, problem, lang, team, time, memory] = arr;
    update_problem_table(stat, letter, team);
}
socket.onclose = (e) => {
    console.log(e);
}
socket.onerror = (e) => {
    console.log(e);
}
</script>
