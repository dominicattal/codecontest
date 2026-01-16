<body>
<h1>Stuff:</h1>
<?php
    echo is_writeable(sys_get_temp_dir());
    echo "<h3>" . sys_get_temp_dir() . "</h3>";
    echo "<h3>POST: " . var_dump($_POST) . "</h3>";
    echo "<h3>FILES: " . count($_FILES) . "</h3>";
    echo "<h3>FILES VAR DUMP:</h3>";
    var_dump($_FILES);
    echo "<br />";
    var_dump($_FILES["code"]["tmp_name"]);

    if ($_FILES["code"]["error"] != UPLOAD_ERR_OK) {
        echo "could not upload";
        die();
    }

    $path = "./uploads";
    
    if (!is_dir($path)) {
        mkdir($path, 0777, true);
    }

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
?>
</body>
</html>
