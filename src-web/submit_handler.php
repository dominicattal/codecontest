<?php
    if ($_FILES["code"]["error"] != UPLOAD_ERR_OK) {
        trigger_error("Failed to upload file", E_USER_WARNING);
        goto done;
    }

    $ip = "127.0.0.1";
    $port = "27105";
    $username = "team1";
    $password = "shape-untrue";
    $lang = $_POST["language"];
    $problem = $_POST["problem"];
    $file = $_FILES["code"]["tmp_name"];
    $org_name = basename($_FILES["code"]["name"]);

    $pid = pcntl_fork();
    if ($pid == -1) {
        die('server error - couldn\'t fork');
    } else if ($pid) {
        pcntl_wait($pid);
    } else {
        exec("../bin/dev/client -i $ip -t $port -u $username -p $password -l $lang -b $problem -f $file -n $org_name");
    }

done:
    header("Location: runs.php");
    die();
?>
