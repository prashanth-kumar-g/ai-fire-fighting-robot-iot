// ===============================
// Frontend Controller Script
// ===============================

const API_BASE = ""; 
// empty means "same origin" (Render URL automatically)

// ---------- Send Command to Backend ----------
function sendCommand(data) {
  fetch("/api/command", {
    method: "POST",
    headers: {
      "Content-Type": "application/json"
    },
    body: JSON.stringify(data)
  })
  .then(() => {
    // optional log (can remove later)
    console.log("Sent:", data);
  })
  .catch(err => {
    console.error("Send failed:", err);
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
/* -------- PAN -------- */
function panLeft() {
  sendCommand({ pan: "left" });
}

function panRight() {
  sendCommand({ pan: "right" });
}

/* -------- TILT -------- */
function tiltUp() {
  sendCommand({ tilt: "up" });
}

function tiltDown() {
  sendCommand({ tilt: "down" });
}

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




