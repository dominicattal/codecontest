<?php
    include "header.php";
    include "create_arrays.php";
    echo "<table id='standings-table'>";
    echo "<thead><tr>";
    echo "<th scope='col'>Team</th>";
    foreach ($problems as $id => $problem)
        echo "<th scope='col'>$problem[letter]</th>";
    echo "</tr></thead>";
    echo "<tbody>";
    foreach ($teams as $id => $username) {
        echo "<tr>";
        echo "<td>$username</td>";
        foreach ($problems as $id => $problem)
            echo "<td scope='col'>--/--</td>";
        echo "</tr>";
    }
    echo "</tbody></table>";
?>
