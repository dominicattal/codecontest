<?php
    session_start();
    $db = new SQLite3("../runs.db");
    $db->enableExceptions(true);
    $db->exec('PRAGMA journal_mode = wal;');
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
    $_SESSION["password"] = $password;
    $_SESSION["loggedin"] = true;
    header('Location: problems.php');
cleanup:
    $res->finalize();
    $db->close();
    die();
?>
