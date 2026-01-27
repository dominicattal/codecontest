var host = "ws://127.0.0.1:27106";
var socket = new WebSocket(host);
window.addEventListener('beforeunload', function() {
    console.log("closing web socket");
    socket.close();
});
socket.onopen = (e) => {
    console.log(e);
    socket.send("Hello from client");
}
socket.onmessage = (e) => {
    console.log(e);
}
socket.onclose = (e) => {
    console.log(e);
}
socket.onerror = (e) => {
    console.log(e);
}


//var ele = document.getElementById("close-socket");
//ele.onclick = (e) => {
//    socket.close();
//}
//
//var ele = document.getElementById("send-socket");
//ele.onclick = (e) => {
//    socket.send("Hello from client");
//
