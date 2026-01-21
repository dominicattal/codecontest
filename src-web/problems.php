<?php
    include "header.php";
    include "create_arrays.php";

    echo "<div id='problems'>";
    echo "<div id='problems-left'>";
    foreach ($problems as $id => $problem) {
        $hidden = ($id == 0) ? "" : "hidden";
        echo "<div id='problem-$problem[letter]' $hidden>";
        include "../$problem[html]";
        echo "</div>";
    }
    echo "</div>";
    echo "<div id='problems-right'>";
    echo "<table id='problems-table'>";
    echo "<thead><tr>";
    echo "<th scope='col'>ID</th>";
    echo "<th scope='col'>Name</th>";
    echo "<th scope='col'>Status</th>";
    echo "<th scope='col'>Num Solved</th>";
    echo "</tr></thead>";
    echo "<tbody>";
    foreach ($problems as $id => $problem) {
        echo "<tr>";
        echo "<td scope='col'>$problem[letter]</td>";
        echo "<td scope='col' class='td-click'><a onclick=showProblem('$problem[letter]')>$problem[name]</a></td>";
        echo "<td scope='col'>----</td>";
        echo "<td scope='col'>----</td>";
        echo "</tr>";
    }
    echo "</tbody></table>";
    echo "</div>";
    echo "</div>";
?>
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
<script src="fouc.js"></script>
