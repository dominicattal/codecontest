<?php
    if ($_FILES["code"]["error"] != UPLOAD_ERR_OK) {
        trigger_error("Failed to upload file", E_USER_WARNING);
        goto done;
    }

    $path = "./uploads";
    
    if (!is_dir($path)) {
        mkdir($path, 0777, true);
    }

    trigger_error("WARNING 1", E_USER_WARNING);

    $tmp_name = $_FILES["code"]["tmp_name"];
    $org_name = basename($_FILES["code"]["name"]);
    $new_file_name = "$path/$org_name";

    echo "<br />";
    echo $tmp_name;
    echo "<br />";
    echo $new_file_name;
    echo "<br />";
    $res = move_uploaded_file($tmp_name, $new_file_name) ? "true" : "false";
    echo "<p>" . $res . "</p>";

done:
    header("Location: runs.php");
    die();
?>
