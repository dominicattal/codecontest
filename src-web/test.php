<?php
    include "header.php";
    $db = new SQLite3("../problems/runs.db", SQLITE3_OPEN_READONLY);
    $db->enableExceptions(true);
    $stmt = $db->prepare("SELECT * FROM runs");
    $res = $stmt->execute();
    print_r($res->fetchArray(SQLITE3_ASSOC));
    $res->finalize();
    $db->close();
?>
