<div id='problems-submit'>
  <form action="submit_handler.php" method="post" enctype="multipart/form-data">
    <table id='problems-submit-table'>
      <tbody>
        <tr>
          <td><label for="problem">Problem</label></td>
          <td>
            <select id="problem" name="problem">
              <option value="">Choose Problem</option>
              <?php
                foreach ($problems as $id => $problem)
                  echo "<option value='$problem[letter]'>$problem[letter] - $problem[name]</option>";
              ?>
            </select>
          </td>
        </tr>
        <tr>
          <td><label for="language">Language</label></td>
          <td>
            <select id="language" name="language">
              <option value="">Choose Language</option>
              <?php
                foreach ($langs as $id => $lang)
                  echo "<option value='$lang'>$lang</option>";
              ?>
            </select>
          </td>
        </tr>
        <tr>
          <td><label for="code">File</label></td>
          <td>
            <input type="hidden" name="MAX_FILE_SIZE" value="30000" />
            <input type="file" id="code" name="code"></input>
          </td>
        </tr>
      </tbody>
      <tfoot>
        <tr>
          <td colspan="2"><input type="submit" value="Submit" id="submit"></input></td>
        </tr>
        <?php
        if (isset($_SESSION["message"])) {
          echo "<tr>";
          echo "<td colspan='2'>$_SESSION[message]</td>";
          echo "</tr>";
          unset($_SESSION["message"]);
        }
        ?>
      </tfoot>
    </table>
  </form>
</div>
