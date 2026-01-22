<?php
    session_start();
    $loggedin = isset($_SESSION["loggedin"]) && $_SESSION["loggedin"];
    if (!$loggedin) {
        header("Location: login.php");
        die();
    }
    if ($_FILES["code"]["error"] != UPLOAD_ERR_OK) {
        trigger_error("Failed to upload file", E_USER_WARNING);
        goto done;
    }

    $ip = "127.0.0.1";
    $port = "27105";
    $username = $_SESSION["username"];
    $password = $_SESSION["password"];
    $lang = $_POST["language"];
    $problem = $_POST["problem"];
    $file = $_FILES["code"]["tmp_name"];
    $org_name = basename($_FILES["code"]["name"]);

    $res = shell_exec("../bin/dev/client -i $ip -t $port -u $username -p $password -l $lang -b $problem -f $file -n $org_name -a");

done:
    header("Location: runs.php");
    die();
?>
