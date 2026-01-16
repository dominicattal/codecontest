<html>
<head>
    <title>NJIT Programming Team</title>
</head>
<body>
    <h1>Hello World</h1>
    <form action="submit.php" method="post" enctype="multipart/form-data">
        <label for="language">Language</label>
        <select id="language" name="language">
            <option value="">Choose Language</option>
            <option value="CPP">C++</option>
            <option value="Python">Python</option>
        </select>
        <label for="code">File</label>
        <input type="hidden" name="MAX_FILE_SIZE" value="30000" />
        <input type="file" id="code" name="code"></input>
        <label for="test_text">test</label>
        <input type="text" id="test_text" name="test_text"></input>
        <input type="submit" value="Submit" id="submit"></input>
    </form>
    <br />
    <p id="dynamic-ele"></p>

    <script>
        var host = "ws://0.0.0.0:12345/socket.php";
        var socket = new WebSocket(host);
        socket.onmessage = function(e) {
               document.getElementById("dynamic-ele").text = e.data;
        }
    </script>
</body>
</html>
