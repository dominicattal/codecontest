<?php
require_once "header.php";
require_once "config.php";
session_start();
?>
<style>
#login-table {
  margin-left: auto;
  margin-right: auto;
  margin-top: 15px;
  text-align: center;
  font-size: 0.8rem;
  letter-spacing: 1px;
}
#login-table th, td {
  border: 1px solid rgb(160 160 160);
}
</style>
<div id="login-form">
  <form action="login_handler.php" method="post" enctype="multipart/form-data">
    <table id='login-table'>
      <tbody>
        <tr>
          <td><label for="username">Username</label></td>
          <td><input type="text" name="username" id="username" required /></td>
        </tr>
        <tr>
          <td><label for="password">Password</label></td>
          <td><input type="text" name="password" id="password" required /></td>
        </tr>
        <tr>
          <td colspan="2"><input type="submit" value="Submit" id="submit"></input></td>
        </tr>
      </tbody>
    </table>
  </form>
</div>
<?php 
if (isset($_SESSION["login_error"])) {
  echo "<p>{$_SESSION["login_error"]}</p>";
  unset($_SESSION["login_error"]);
}
?>
