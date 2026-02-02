// ===============================
// AIoT Backend Server (Render)
// ===============================

const express = require("express");
const path = require("path");

const app = express();
const PORT = process.env.PORT || 3000;

// ---------- Middleware ----------
app.use(express.json());
app.use(express.static(path.join(__dirname, "public")));

// ---------- In-memory states ----------

// Commands sent FROM website TO ESP32
let commandState = {
  move: "stop",       // forward, backward, left, right, stop
  pump: "off",        // on, off
  pan: "center",      // left, right, center
  tilt: "center",     // up, down, center
  mode: "manual",      // manual / auto (future)
};

// Status sent FROM ESP32 TO website
let statusState = {
  fireDetected: false,
  flameLeft: 1,
  flameCenter: 1,
  flameRight: 1,
  pumpStatus: "off",
  aiStatus: "idle"
};

// ---------- API: COMMAND ----------

// Website -> Backend (set command)
app.post("/api/command", (req, res) => {
  commandState = { ...commandState, ...req.body};
  console.log("COMMAND UPDATE:", commandState);
  res.json({ success: true });
});

// ESP32 -> Backend (get command)
app.get("/api/command", (req, res) => {
  res.json(commandState);
});

// ---------- API: STATUS ----------

// ESP32 -> Backend (send status)
app.post("/api/status", (req, res) => {
  statusState = { ...statusState, ...req.body };
  console.log("STATUS UPDATE:", statusState);
  res.json({ success: true });
});

// Website -> Backend (get status)
app.get("/api/status", (req, res) => {
  res.json(statusState);
});

app.get("/health", (req, res) => {
  res.json({ status: "ok" });
});

// ---------- Root ----------
app.get("/", (req, res) => {
  res.sendFile(path.join(__dirname, "public", "index.html"));
});

// ---------- Start server ----------
app.listen(PORT, () => {
  console.log(`✅ AIoT Backend running on port ${PORT}`);
});






