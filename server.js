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
  mode: "manual",     // manual / automatic
  move: "stop",       // forward, backward, left, right, stop
  pump: "off",        // on, off
  pan: "none",        // left, right, none
  tilt: "none"        // up, down, none
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
  const { mode, move, pan, tilt, pump } = req.body;

  if (mode !== undefined) commandState.mode = mode;
  if (move !== undefined) commandState.move = move;
  if (pan !== undefined)  commandState.pan  = pan;
  if (tilt !== undefined) commandState.tilt = tilt;
  if (pump !== undefined) commandState.pump = pump;

  res.json({ status: "OK", commandState });
});

// ESP32 -> Backend (get command)
app.get("/api/command", (req, res) => {

  const cmd = {
    mode: commandState.mode,
    move: commandState.move,
    pan: commandState.pan,
    tilt: commandState.tilt,
    pump: commandState.pump
  };

  // One-shot resets
  commandState.pan  = "none";
  commandState.tilt = "none";
  // DO NOT reset mode(toggle-based)
  // DO NOT reset pump(toggle-based)

  res.json(cmd);
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

