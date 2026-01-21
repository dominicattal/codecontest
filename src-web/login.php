<?php
    include "header.php";
    session_start();
?>
<form action="login_handler.php" method="post" enctype="multipart/form-data">
    <label for="username">Username</label>
    <input type="text" name="username" id="username" required />
    <label for="password">Password</label>
    <input type="text" name="password" id="password" required />
    <input type="submit" />
</form>
<?php 
    if (isset($_SESSION["login_error"])) {
        echo "<p>{$_SESSION["login_error"]}</p>";
        unset($_SESSION["login_error"]);
    }
?>
