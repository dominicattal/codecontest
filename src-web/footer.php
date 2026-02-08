<script>
let header_countdown = document.getElementById('header-countdown');
if (header_countdown) {
  let end_time = <?php echo $contest["end"]; ?>;
  setInterval(function() {
    cur_time = new Date().getTime() / 1000;
    let time_left = end_time - cur_time;
    let hour = Math.floor(time_left / 3600);
    let minute = (Math.floor((time_left % 3600) / 60)).toString().padStart(2, '0');
    let second = (Math.floor(time_left % 60)).toString().padStart(2, '0');
    header_countdown.textContent = `${hour}:${minute}:${second}`;
  }, 500);
}
</script>
