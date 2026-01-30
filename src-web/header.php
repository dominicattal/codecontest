<!DOCTYPE html>
<html>
<head>
  <title>NJIT Programming Team</title>
  <link blocked="render" rel="stylesheet" href="styles.css" />
</head>
<body>
  <div id="header">
    <div>
      <a>Programming Team</a>
    </div>
    <div>
      <a href="problems.php">Problems</a>
      <a href="runs.php">Runs</a>
      <a href="standings.php">Standings</a>
      <?php
        session_start();
        if (isset($_SESSION["loggedin"]) && $_SESSION["loggedin"]) {
            echo '<a href="logout.php">Logout</a>';
        } else {
            echo '<a href="login.php">Login</a>';
        }
      ?>
    </div>
    <div id="countdown">
      <a>0:00:00</a>
    </div> 
  </div>
