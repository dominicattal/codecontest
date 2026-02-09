<?php
session_start();
$loggedin = isset($_SESSION["loggedin"]) && $_SESSION["loggedin"];
if (!$loggedin) {
  header("Location: login.php");
  die();
}
if ($_FILES["code"]["error"] != UPLOAD_ERR_OK) {
  trigger_error("Failed to upload file", E_USER_WARNING);
  $_SESSION["message"] = "Internal error";
  goto done;
}

require_once "config.php";

$ip = $config["ip"];
$port = $config["cli_port"];
$username = $_SESSION["username"];
$password = $_SESSION["password"];
$lang = $_POST["language"];
$problem = $_POST["problem"];
$file = $_FILES["code"]["tmp_name"];
$org_name = basename($_FILES["code"]["name"]);

$command = "../bin/client -i $ip -t $port -u $username -p $password -l $lang -b $problem -f $file -n $org_name -a";
$output = null;
exec($command, $output, $result_code);
if ($result_code == 1) {
  trigger_error("Internal error", E_USER_WARNING);
  trigger_error($output[0], E_USER_WARNING);
  $_SESSION["message"] = "Internal error";
}

done:
header("Location: submit.php");
die();
?>
