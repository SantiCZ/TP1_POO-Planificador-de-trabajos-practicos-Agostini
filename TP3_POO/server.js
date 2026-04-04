/**
 * Collaborative Canvas – VPS Backend
 * Endpoint: POST /drawing  → save strokes
 *           GET  /drawing  → retrieve strokes
 *
 * Install: npm install express cors
 * Run:     node server.js
 */

const express = require("express");
const cors    = require("cors");
const fs      = require("fs");
const path    = require("path");

const app  = express();
const PORT = process.env.PORT || 5005;
const DATA_FILE = path.join(__dirname, "drawing_state.json");

app.use(cors());
app.use(express.json({ limit: "50mb" }));

// ── Helpers ───────────────────────────────────────────────────────────────────

function loadState() {
  try {
    if (fs.existsSync(DATA_FILE)) {
      const raw = fs.readFileSync(DATA_FILE, "utf8");
      return JSON.parse(raw);
    }
  } catch (e) {
    console.error("Failed to load state:", e.message);
  }
  return { strokes: [] };
}

function saveState(state) {
  fs.writeFileSync(DATA_FILE, JSON.stringify(state), "utf8");
}

// ── GET /drawing ──────────────────────────────────────────────────────────────
// Returns current drawing state (all strokes)

app.get("/drawing", (req, res) => {
  const state = loadState();
  res.json(state);
});

// ── POST /drawing ─────────────────────────────────────────────────────────────
// Merges incoming strokes with stored strokes (incremental merge by ID)

app.post("/drawing", (req, res) => {
  const incoming = req.body;

  if (!incoming || !Array.isArray(incoming.strokes)) {
    return res.status(400).json({ error: "Invalid payload: expected { strokes: [] }" });
  }

  const current = loadState();

  // Build index of existing stroke IDs
  const knownIds = new Set(current.strokes.map(s => s.id));

  let added = 0;
  for (const stroke of incoming.strokes) {
    if (!knownIds.has(stroke.id)) {
      current.strokes.push(stroke);
      knownIds.add(stroke.id);
      added++;
    }
  }

  // Sort by timestamp to preserve draw order
  current.strokes.sort((a, b) => (a.timestamp || 0) - (b.timestamp || 0));

  saveState(current);

  console.log(`[${new Date().toISOString()}] Merged ${added} new strokes. Total: ${current.strokes.length}`);

  res.json({ ok: true, total: current.strokes.length, added });
});

// ── DELETE /drawing ───────────────────────────────────────────────────────────
// Clear all strokes (admin use)

app.delete("/drawing", (req, res) => {
  saveState({ strokes: [] });
  res.json({ ok: true, message: "Canvas cleared." });
});

// ── Health ────────────────────────────────────────────────────────────────────

app.get("/health", (req, res) => {
  const state = loadState();
  res.json({ status: "ok", strokeCount: state.strokes.length });
});

// ── Start ─────────────────────────────────────────────────────────────────────

app.listen(PORT, () => {
  console.log(`Collaborative Canvas server running on port ${PORT}`);
  console.log(`Endpoints:`);
  console.log(`  GET    /drawing  → fetch all strokes`);
  console.log(`  POST   /drawing  → merge & save strokes`);
  console.log(`  DELETE /drawing  → clear canvas`);
  console.log(`  GET    /health   → server status`);
});