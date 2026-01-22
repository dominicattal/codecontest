<?php
    $db = new SQLite3("../problems/runs.db");
    $db->enableExceptions(true);
    $db->exec('PRAGMA journal_mode = wal;');

    $stmt = $db->prepare("SELECT * FROM teams");
    $res = $stmt->execute();
    $arr = $res->fetchArray(SQLITE3_ASSOC);
    $teams = array();
    while ($arr) {
        $teams[$arr["id"]] = $arr["username"];
        $arr = $res->fetchArray(SQLITE3_ASSOC);
    }
    $res->finalize();

    $stmt = $db->prepare("SELECT * FROM problems");
    $res = $stmt->execute();
    $arr = $res->fetchArray(SQLITE3_ASSOC);
    $problems = array();
    while ($arr) {
        $problems[$arr["id"]] = array("name" => $arr["name"], "letter" => $arr["letter"], "html" => $arr["html_path"]);
        $arr = $res->fetchArray(SQLITE3_ASSOC);
    }
    $res->finalize();

    $stmt = $db->prepare("SELECT * FROM languages");
    $res = $stmt->execute();
    $arr = $res->fetchArray(SQLITE3_ASSOC);
    $langs = array();
    while ($arr) {
        $langs[$arr["id"]] = $arr["name"];
        $arr = $res->fetchArray(SQLITE3_ASSOC);
    }
    $res->finalize();

    $db->close();
?>
