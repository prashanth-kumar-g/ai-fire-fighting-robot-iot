// ===============================
// Frontend Controller Script
// ===============================

const API_BASE = ""; 
// empty means "same origin" (Render URL automatically)

// ---------- Send Command to Backend ----------
function sendCommand(command) {
  fetch(`${API_BASE}/api/command`, {
    method: "POST",
    headers: {
      "Content-Type": "application/json"
    },
    body: JSON.stringify(command)
  })
  .then(res => res.json())
  .then(data => {
    console.log("Command sent:", command);
  })
  .catch(err => {
    console.error("Command error:", err);
  });
}

// ---------- Button Actions (CAR) ----------
function moveForward() {
  sendCommand({ move: "forward" });
}

function moveBackward() {
  sendCommand({ move: "backward" });
}

function turnLeft() {
  sendCommand({ move: "left" });
}

function turnRight() {
  sendCommand({ move: "right" });
}

function stopCar() {
  sendCommand({ move: "stop" });
}

// ---------- Pump Control ----------
function pumpOn() {
  sendCommand({ pump: "on" });
}

function pumpOff() {
  sendCommand({ pump: "off" });
}

// ---------- Pan Tilt Control ----------
const panLeft = document.getElementById("panLeft");
panLeft.addEventListener("mousedown", () => {
  sendCommand({ pan: "left" });
});
panLeft.addEventListener("mouseup", () => {
  sendCommand({ pan: "none" });
});

panLeft.addEventListener("touchstart", () => {
  sendCommand({ pan: "left" });
});
panLeft.addEventListener("touchend", () => {
  sendCommand({ pan: "none" });
});

const panRight = document.getElementById("panRight");
panRight.addEventListener("mousedown", () => {
  sendCommand({ pan: "right" });
});
panRight.addEventListener("mouseup", () => {
  sendCommand({ pan: "none" });
});

panRight.addEventListener("touchstart", () => {
  sendCommand({ pan: "right" });
});
panRight.addEventListener("touchend", () => {
  sendCommand({ pan: "none" });
});

const tiltUp = document.getElementById("tiltUp");
tiltUp.addEventListener("mousedown", () => {
  sendCommand({ tilt: "up" });
});
tiltUp.addEventListener("mouseup", () => {
  sendCommand({ tilt: "none" });
});

tiltUp.addEventListener("touchstart", () => {
  sendCommand({ tilt: "up" });
});
tiltUp.addEventListener("touchend", () => {
  sendCommand({ tilt: "none" });
});

const tiltDown = document.getElementById("tiltDown");

tiltDown.addEventListener("mousedown", () => {
  sendCommand({ tilt: "down" });
});
tiltDown.addEventListener("mouseup", () => {
  sendCommand({ tilt: "none" });
});

tiltDown.addEventListener("touchstart", () => {
  sendCommand({ tilt: "down" });
});
tiltDown.addEventListener("touchend", () => {
  sendCommand({ tilt: "none" });
});

// ---------- Poll Status from Backend ----------
function updateStatus() {
  fetch(`${API_BASE}/api/status`)
    .then(res => res.json())
    .then(status => {
      console.log("Status:", status);

      // Update UI text (IDs must exist in HTML)
      document.getElementById("fireStatus").innerText =
        status.fireDetected ? "YES" : "NO";

      document.getElementById("aiStatus").innerText =
        status.aiStatus.toUpperCase();

      document.getElementById("alertStatus").innerText =
        status.pumpStatus === "on" ? "ON" : "OFF";
    })
    .catch(err => {
      console.error("Status error:", err);
    });
}

// Poll status every 1 second
setInterval(updateStatus, 1000);

