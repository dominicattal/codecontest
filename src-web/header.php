<?php
session_start();
require_once "config.php";
require_once "create_arrays.php";
?>
<!DOCTYPE html>
<html>
<head>
    <title><?php echo $config["title"]; ?></title>
  <style>
    #header {
        display: flex;
        justify-content: space-between;
        background: #f0f0f0;
    }

    #header div {
        padding: 20px;
        color: blue;
    }

    #header a {
        padding: 20px;
    }
    #header-countdown, #header-name {
        width: 15%;
    }
    #header-countdown {
      text-align: right;
    }
    body {
        font-family: consolas;
    }

    #runs-table {
        margin-left: auto;
        margin-right: auto;
        margin-top: 15px;
        border-collapse: collapse;
        text-align: center;
        border: 2px solid rgb(140 140 140);
        font-size: 0.8rem;
        letter-spacing: 1px;
    }

    #runs-table thead {
        background-color: rgb(228 240 245);
    }

    #runs-table th, td {
        border: 1px solid rgb(160 160 160);
        padding: 8px 10px;
    }

    #runs-table tbody > tr:nth-of-type(even) {
        background-color: rgb(237 238 242);
    }
    #runs-table tbody > tr:nth-of-type(odd) {
        background-color: rgb(255 255 255);
    }
  </style>
</head>
<body>
  <div id="header">
    <div id="header-name">
        <?php echo $config["name"]; ?>
    </div>
    <div>
      <a href="problems.php">Problems</a>
      <a href="submit.php">Submit</a>
      <a href="runs.php">Runs</a>
      <a href="standings.php">Standings</a>
      <?php
        if (isset($_SESSION["loggedin"]) && $_SESSION["loggedin"]) {
            echo '<a href="logout.php">Logout</a>';
        } else {
            echo '<a href="login.php">Login</a>';
        }
      ?>
    </div>
    <div id='header-countdown'>
    <?php
      if ($contest["active"]) {
        $time_left = $contest["end"] - time() - 1; // -1 to compensate a little
        $hour = intdiv($time_left, 3600);
        $minute = intdiv($time_left%3600, 60);
        $second = $time_left%60;
        $str = sprintf("%d:%02d:%02d", $hour, $minute, $second);
        if ($time_left > 0)
          echo "$str";
        else
          echo "0:00:00";
      }
      ?>
    </div>
  </div>
