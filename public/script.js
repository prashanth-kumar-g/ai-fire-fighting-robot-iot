function sendCmd(cmd) {
  fetch("/control?cmd=" + cmd);
}
