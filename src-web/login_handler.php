<?php
    session_start();
    var_dump($_POST);
    $db = new SQLite3("../problems/runs.db", SQLITE3_OPEN_READONLY);
    $db->enableExceptions(true);
    $username = htmlspecialchars($_POST["username"]);
    $password = htmlspecialchars($_POST["password"]);
    $stmt = $db->prepare("SELECT * FROM teams WHERE username=:username");
    $stmt->bindParam('username', $username);
    $res = $stmt->execute();
    $arr = $res->fetchArray(SQLITE3_ASSOC);
    if (!$arr) {
        $_SESSION["login_error"] = "User not found";
        header('Location: login.php');
        goto cleanup;
    }
    if ($arr["password"] !== $password) {
        $_SESSION["login_error"] = "Invalid password";
        header('Location: login.php');
        goto cleanup;
    }
    unset($_SESSION["login_error"]);
    $_SESSION["username"] = $username;
    $_SESSION["loggedin"] = true;
    header('Location: problems.php');
cleanup:
    $res->finalize();
    $db->close();
    die();
?>
