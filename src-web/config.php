<?php
$string = file_get_contents("../config-web.json");
$config = get_object_vars(json_decode($string));
?>
