const express = require("express");
const app = express();

// Serve static files from "public" folder
app.use(express.static("public"));

// Root route
app.get("/", (req, res) => {
  res.sendFile(__dirname + "/public/index.html");
});

// Render provides PORT automatically
const PORT = process.env.PORT || 3000;
app.listen(PORT, () => {
  console.log("Server running on port", PORT);
});