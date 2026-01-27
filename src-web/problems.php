<?php
    include "header.php";
    include "create_arrays.php";
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
                        <th scope='col'>Status</th>
                        <th scope='col'>Num Solved</th>
                    </tr>
                </thead>
                <tbody>
                <?php
                    foreach ($problems as $id => $problem) {
                        echo "<tr>";
                        echo "<td scope='col'>$problem[letter]</td>";
                        echo "<td scope='col' class='td-click'><a onclick=showProblem('$problem[letter]')>$problem[name]</a></td>";
                        echo "<td scope='col'>----</td>";
                        echo "<td scope='col'>----</td>";
                        echo "</tr>";
                    }
                ?>
                </tbody>
            </table>
        </div>
        <div id='problems-submit'>
            <form action="submit_handler.php" method="post" enctype="multipart/form-data">
                <label for="problem">Problem</label>
                <select id="problem" name="problem">
                    <option value="">Choose Problem</option>
                    <?php
                        foreach ($problems as $id => $problem)
                            echo "<option value='$problem[letter]'>$problem[letter] - $problem[name]</option>";
                    ?>
                </select>
                <label for="language">Language</label>
                <select id="language" name="language">
                    <option value="">Choose Language</option>
                    <?php
                        foreach ($langs as $id => $lang)
                            echo "<option value='$lang'>$lang</option>";
                    ?>
                </select>
                <br />
                <label for="code">File</label>
                <input type="hidden" name="MAX_FILE_SIZE" value="30000" />
                <input type="file" id="code" name="code"></input>
                <br />
                <input type="submit" value="Submit" id="submit"></input>
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
</script>
<script src="websocket.js"></script>
