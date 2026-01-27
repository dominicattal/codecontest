<?php
    include "header.php";
?>
    <form action="submit_handler.php" method="post" enctype="multipart/form-data">
        <label for="language">Language</label>
        <select id="language" name="language">
            <option value="">Choose Language</option>
            <option value="CPP">C++</option>
            <option value="Python">Python</option>
        </select>
        <br />
        <label for="code">File</label>
        <input type="hidden" name="MAX_FILE_SIZE" value="30000" />
        <input type="file" id="code" name="code"></input>
        <br />
        <input type="submit" value="Submit" id="submit"></input>
    </form>
    <br />
    <p id="dynamic-ele"></p>
    <button id="send-socket">Send</button>
    <button id="close-socket">Close</button>

    <script>
    </script>
