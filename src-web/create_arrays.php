<?php
require_once "config.php";
$db = new SQLite3($config["database"]);
$db->enableExceptions(true);
$db->exec('PRAGMA journal_mode = wal;');

$stmt = $db->prepare("SELECT * FROM teams");
$res = $stmt->execute();
$arr = $res->fetchArray(SQLITE3_ASSOC);
$teams = array();
$team_to_id = array();
while ($arr) {
    $teams[$arr["id"]] = $arr["username"];
    $team_to_id[$arr["username"]] = $arr["id"];
    $arr = $res->fetchArray(SQLITE3_ASSOC);
}
$res->finalize();

$stmt = $db->prepare("SELECT * FROM problems");
$res = $stmt->execute();
$arr = $res->fetchArray(SQLITE3_ASSOC);
$problems = array();
while ($arr) {
    $problems[$arr["id"]] = array("name" => $arr["name"], "letter" => $arr["letter"], "html" => $arr["html_path"], "pdf" => $arr["pdf_path"]);
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
