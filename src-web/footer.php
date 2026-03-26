<script>
let header_countdown = document.getElementById('header-countdown');
let active = <?php echo (isset($contest["active"]) ? $contest["active"] : 0); ?>;
if (header_countdown && active) {
  let end_time = <?php echo (isset($contest["end"]) ? $contest["end"] : 0); ?>;
  setInterval(function() {
    cur_time = new Date().getTime() / 1000;
    let time_left = end_time - cur_time;
    if (time_left > 0) {
      let hour = Math.floor(time_left / 3600);
      let minute = (Math.floor((time_left % 3600) / 60)).toString().padStart(2, '0');
      let second = (Math.floor(time_left % 60)).toString().padStart(2, '0');
      header_countdown.textContent = `${hour}:${minute}:${second}`;
    } else {
      header_countdown.textContent = '0:00:00';
    }
  }, 500);
}
</script>
