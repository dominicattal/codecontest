// boilerplate
var host = "ws://127.0.0.1:27106";
var socket = new WebSocket(host);
window.addEventListener('beforeunload', function() {
    socket.close();
});
socket.onopen = (e) => {
    console.log(e);
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
