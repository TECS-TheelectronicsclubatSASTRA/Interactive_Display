#include <WiFi.h>
#include <WebServer.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

// ================= HARDWARE CONFIG =================
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES   4   // Strictly 4-Module Array (32 total characters)
#define CLK_PIN       18
#define DATA_PIN      23
#define CS_PIN        5

MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);
WebServer server(80);

// Buffer for 32 total columns (4 modules x 8 columns)
byte displayState[MAX_DEVICES][8];

// ================= ROLLING ANIMATION VARIABLES =================
bool isScrolling = false;
String scrollText = "";
int scrollIndex = 0;
unsigned long lastScrollTime = 0;
int scrollDelay = 150; // Milliseconds per character shift (Lower = Faster)

// ================= 7-SEGMENT ALPHANUMERIC FONT ENGINE =================
byte getCharPattern(char c) {
  c = toupper(c);
  switch(c) {
    case '0': return 0x7E; case '1': return 0x30; case '2': return 0x6D; case '3': return 0x79;
    case '4': return 0x33; case '5': return 0x5B; case '6': return 0x5F; case '7': return 0x70;
    case '8': return 0x7F; case '9': return 0x7B; case 'A': return 0x77; case 'B': return 0x1F;
    case 'C': return 0x4E; case 'D': return 0x3D; case 'E': return 0x4F; case 'F': return 0x47;
    case 'G': return 0x5E; case 'H': return 0x37; case 'I': return 0x30; case 'J': return 0x3C;
    case 'K': return 0x57; case 'L': return 0x0E; case 'M': return 0x55; case 'N': return 0x15;
    case 'O': return 0x7E; case 'P': return 0x67; case 'Q': return 0x73; case 'R': return 0x05;
    case 'S': return 0x5B; case 'T': return 0x0F; case 'U': return 0x3E; case 'V': return 0x1C;
    case 'W': return 0x2A; case 'X': return 0x37; case 'Y': return 0x3B; case 'Z': return 0x6D;
    case ' ': return 0x00; case '-': return 0x01; case '.': return 0x80; case '_': return 0x08;
    default:  return 0x00;
  }
}

// Helper to render any 32-character slice onto the physical displays
void renderTextSlice(String slice) {
  memset(displayState, 0, sizeof(displayState));
  int len = min((int)slice.length(), MAX_DEVICES * 8);
  for (int i = 0; i < len; i++) {
    char c = slice[i];
    byte pattern = getCharPattern(c);
    int m = i / 8;
    int d = 7 - (i % 8); // Natural left-to-right mapping
    for (int s = 0; s < 8; s++) {
      if (pattern & (1 << s)) {
        displayState[m][s] |= (1 << d);
      }
    }
  }
  
  for (int m = 0; m < MAX_DEVICES; m++) {
    for (int i = 0; i < 8; i++) {
      mx.setColumn(m, i, displayState[m][i]);
    }
  }
}

// Non-blocking ticker step called inside loop()
void stepScroll() {
  int totalLen = scrollText.length();
  if (totalLen == 0) return;
  
  String slice = "";
  for (int i = 0; i < 32; i++) {
    int idx = (scrollIndex + i) % totalLen;
    slice += scrollText[idx];
  }
  
  renderTextSlice(slice);
  scrollIndex = (scrollIndex + 1) % totalLen;
}

// ================= WEB PAGE (Professional Studio UI) =================
const char* html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name='viewport' content='width=device-width, initial-scale=1, maximum-scale=1, user-scalable=no'>
  <title>LED Matrix Studio PRO</title>
  <style>
    :root { --bg: #0b0f17; --panel: #141c2b; --accent: #00ffcc; --border: #26354f; --text: #e1e8f0; }
    * { box-sizing: border-box; }
    body { 
      background: var(--bg); color: var(--text); font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif; 
      margin: 0; display: flex; flex-direction: column; align-items: center; 
      min-height: 100vh; padding: 15px; touch-action: none;
    }
    .header { display: flex; justify-content: space-between; align-items: center; width: 100%; max-width: 320px; margin-bottom: 15px; }
    h3 { color: var(--accent); margin: 0; font-size: 0.95rem; letter-spacing: 1.5px; text-transform: uppercase; font-weight: 700; }
    .status { font-size: 0.7rem; font-weight: 700; background: rgba(0,255,204,0.1); color: var(--accent); padding: 4px 10px; border-radius: 20px; border: 1px solid var(--accent); letter-spacing: 1px; }
    
    /* Navigation Tabs */
    .tab-bar { display: flex; width: 100%; max-width: 320px; gap: 8px; margin-bottom: 15px; }
    .tab { 
      flex: 1; padding: 12px 8px; background: #182232; color: #7a91b0; border: 1px solid var(--border); 
      border-radius: 8px; font-weight: 600; font-size: 0.75rem; text-transform: uppercase; letter-spacing: 1px;
      cursor: pointer; transition: all 0.2s; text-align: center;
    }
    .tab.active { background: rgba(0,255,204,0.12); color: var(--accent); border-color: var(--accent); box-shadow: 0 0 12px rgba(0,255,204,0.15); }
    
    .panel { display: flex; flex-direction: column; width: 100%; max-width: 320px; align-items: center; }
    
    /* Studio Canvas */
    #canvas-container {
      width: 320px; height: 400px; background: var(--panel); border: 2px solid var(--border); 
      border-radius: 12px; box-shadow: 0 10px 30px rgba(0,0,0,0.7); overflow: hidden;
    }
    canvas { width: 100%; height: 100%; display: block; cursor: crosshair; }
    
    /* Marquee Studio */
    #wordInput {
      width: 100%; padding: 16px; background: var(--panel); border: 2px solid var(--border);
      border-radius: 8px; color: var(--accent); font-size: 1rem; font-weight: 700;
      text-transform: uppercase; outline: none; margin-bottom: 12px; text-align: center;
      letter-spacing: 2px;
    }
    #wordInput:focus { border-color: var(--accent); box-shadow: 0 0 15px rgba(0,255,204,0.2); }
    
    .slider-box { width: 100%; background: var(--panel); padding: 12px; border-radius: 8px; border: 1px solid var(--border); margin-bottom: 15px; }
    .slider-label { display: flex; justify-content: space-between; font-size: 0.7rem; color: #7a91b0; font-weight: 700; margin-bottom: 6px; text-transform: uppercase; letter-spacing: 1px; }
    input[type=range] { width: 100%; accent-color: var(--accent); cursor: pointer; }

    .mod-preview-container { width: 100%; display: flex; flex-direction: column; gap: 6px; margin-bottom: 15px; }
    .mod-row { 
      display: flex; justify-content: space-between; align-items: center; 
      background: var(--panel); padding: 8px 12px; border-radius: 6px; border: 1px solid var(--border);
    }
    .mod-label { font-size: 0.65rem; color: #7a91b0; font-weight: 700; letter-spacing: 1px; }
    .mod-text { font-family: monospace; font-size: 1.1rem; color: var(--accent); letter-spacing: 4px; font-weight: 700; }

    /* Action Buttons */
    .btn { 
      width: 100%; max-width: 320px; margin-top: 8px; padding: 14px; background: #26354f; color: #fff; 
      border: none; border-radius: 8px; font-weight: 700; font-size: 0.8rem; text-transform: uppercase; 
      letter-spacing: 1.5px; cursor: pointer; transition: all 0.2s;
    }
    .btn.primary { background: var(--accent); color: #000; box-shadow: 0 4px 15px rgba(0,255,204,0.3); }
    .btn.danger { background: #ff3366; color: #fff; box-shadow: 0 4px 15px rgba(255,51,102,0.3); }
    .btn:active { transform: scale(0.98); }
  </style>
</head>
<body>
  <div class="header">
    <h3>Matrix Studio PRO</h3>
    <div class="status" id="status">ONLINE</div>
  </div>

  <div class="tab-bar">
    <div class="tab active" id="tab-draw" onclick="switchMode('draw')">🖌️ Freehand Studio</div>
    <div class="tab" id="tab-word" onclick="switchMode('word')">🎬 Marquee Ticker</div>
  </div>

  <!-- TAB 1: FREEHAND STUDIO -->
  <div class="panel" id="draw-panel">
    <div id="canvas-container">
      <canvas id="board" width="320" height="400"></canvas>
    </div>
    <button class="btn danger" onclick="clearCanvas()" style="margin-top: 15px;">Wipe Canvas Clean</button>
  </div>

  <!-- TAB 2: MARQUEE TICKER STUDIO -->
  <div class="panel" id="word-panel" style="display: none;">
    <input type="text" id="wordInput" maxlength="100" placeholder="ENTER MESSAGE (MAX 100)..." oninput="updateWordPreview()" onkeydown="if(event.key==='Enter') sendRollingData()">
    
    <div class="slider-box">
      <div class="slider-label"><span>Rolling Speed</span><span id="speedVal">Normal</span></div>
      <input type="range" id="speedSlider" min="50" max="350" value="200" step="10" oninput="updateSpeedLabel()">
    </div>

    <!-- Live Studio Array Preview -->
    <div class="mod-preview-container">
      <div class="mod-row"><span class="mod-label">ARRAY 1 [COLS 01-08]</span><span class="mod-text" id="mod-prev-0">        </span></div>
      <div class="mod-row"><span class="mod-label">ARRAY 2 [COLS 09-16]</span><span class="mod-text" id="mod-prev-1">        </span></div>
      <div class="mod-row"><span class="mod-label">ARRAY 3 [COLS 17-24]</span><span class="mod-text" id="mod-prev-2">        </span></div>
      <div class="mod-row"><span class="mod-label">ARRAY 4 [COLS 25-32]</span><span class="mod-text" id="mod-prev-3">        </span></div>
    </div>

    <button class="btn primary" onclick="sendRollingData()">🚀 Start Rolling Ticker</button>
    <button class="btn" onclick="sendStaticData()">⏸️ Display Static Snapshot</button>
    <button class="btn danger" onclick="clearWords()">🗑️ Clear Display</button>
  </div>

  <script>
    const canvas = document.getElementById('board');
    const ctx = canvas.getContext('2d');
    const statusEl = document.getElementById('status');
    
    let isDrawing = false;
    let lastX = -1, lastY = -1;
    let lastSendTime = 0;
    const sentCache = new Set();

    function switchMode(mode) {
      document.getElementById('draw-panel').style.display = (mode === 'draw') ? 'flex' : 'none';
      document.getElementById('word-panel').style.display = (mode === 'word') ? 'flex' : 'none';
      document.getElementById('tab-draw').className = (mode === 'draw') ? 'tab active' : 'tab';
      document.getElementById('tab-word').className = (mode === 'word') ? 'tab active' : 'tab';
    }

    /* --- DRAW CANVAS LOGIC --- */
    function drawGrid() {
      ctx.fillStyle = '#141c2b'; ctx.fillRect(0, 0, canvas.width, canvas.height);
      ctx.strokeStyle = '#1f2b40'; ctx.lineWidth = 1;
      for(let i = 1; i < 8; i++) {
        let x = (canvas.width / 8) * i;
        ctx.beginPath(); ctx.moveTo(x, 0); ctx.lineTo(x, canvas.height); ctx.stroke();
      }
      for(let i = 1; i < 4; i++) {
        let y = (canvas.height / 4) * i;
        ctx.strokeStyle = '#2d3f5e'; ctx.lineWidth = 2;
        ctx.beginPath(); ctx.moveTo(0, y); ctx.lineTo(canvas.width, y); ctx.stroke();
      }
    }
    drawGrid();

    function drawGlowPoint(x, y) {
      ctx.fillStyle = '#00ffcc'; ctx.shadowColor = '#00ffcc'; ctx.shadowBlur = 6;
      ctx.beginPath(); ctx.arc(x, y, 3, 0, Math.PI * 2); ctx.fill(); ctx.shadowBlur = 0;
    }

    function sendSegment(m, d, s) {
      const key = `${m}-${d}-${s}`;
      if (sentCache.has(key)) return; 
      const now = Date.now();
      if (now - lastSendTime < 15) { setTimeout(() => sendSegment(m, d, s), 15); return; }
      sentCache.add(key); lastSendTime = now;
      statusEl.innerText = 'TX...'; statusEl.style.color = '#ffcc00';

      fetch(`/draw?m=${m}&d=${d}&s=${s}`)
        .then(() => { statusEl.innerText = 'ONLINE'; statusEl.style.color = '#00ffcc'; })
        .catch(() => { statusEl.innerText = 'ERROR'; statusEl.style.color = '#ff3366'; });
    }

    function processPoint(clientX, clientY) {
      const rect = canvas.getBoundingClientRect();
      const normX = Math.max(0, Math.min(1, (clientX - rect.left) / rect.width));
      const normY = Math.max(0, Math.min(1, (clientY - rect.top) / rect.height));
      drawGlowPoint(normX * canvas.width, normY * canvas.height);

      const m = Math.floor(normY * 4);
      const d = 7 - Math.floor(normX * 8); 
      const lx = (normX * 8) % 1, ly = (normY * 4) % 1;

      let sBit = -1;
      if (lx > 0.65 && ly > 0.75) sBit = 7;      // Dot (DP)
      else if (ly <= 0.18) sBit = 6;             // Top (A)
      else if (ly > 0.82) sBit = 3;              // Bottom (D)
      else if (ly > 0.40 && ly <= 0.60) sBit = 0;// Center (G)
      else if (ly <= 0.40) sBit = (lx < 0.40) ? 1 : 5; // TopLeft(F) / TopRight(B)
      else sBit = (lx < 0.40) ? 2 : 4;                 // BotLeft(E) / BotRight(C)

      if (sBit !== -1) sendSegment(m, d, sBit);
    }

    function handleMove(e) {
      if (!isDrawing) return;
      const clientX = e.touches ? e.touches[0].clientX : e.clientX;
      const clientY = e.touches ? e.touches[0].clientY : e.clientY;
      if (lastX !== -1 && lastY !== -1) {
        const dist = Math.hypot(clientX - lastX, clientY - lastY);
        const steps = Math.ceil(dist / 4);
        for (let i = 1; i <= steps; i++) {
          processPoint(lastX + (clientX - lastX) * (i / steps), lastY + (clientY - lastY) * (i / steps));
        }
      } else { processPoint(clientX, clientY); }
      lastX = clientX; lastY = clientY;
    }

    canvas.addEventListener('pointerdown', (e) => { isDrawing = true; lastX = -1; lastY = -1; handleMove(e); });
    canvas.addEventListener('pointermove', handleMove);
    window.addEventListener('pointerup', () => { isDrawing = false; });
    window.addEventListener('pointercancel', () => { isDrawing = false; });
    function clearCanvas() { sentCache.clear(); drawGrid(); fetch('/clear'); }

    /* --- MARQUEE TICKER LOGIC --- */
    function updateSpeedLabel() {
      const val = parseInt(document.getElementById('speedSlider').value);
      const label = document.getElementById('speedVal');
      if (val > 270) label.innerText = "Slow Crawl";
      else if (val > 150) label.innerText = "Normal Speed";
      else label.innerText = "Turbo Speed";
    }

    function updateWordPreview() {
      const text = document.getElementById('wordInput').value.toUpperCase();
      for(let i = 0; i < 4; i++) {
        const slice = text.slice(i * 8, (i + 1) * 8).padEnd(8, ' ');
        document.getElementById(`mod-prev-${i}`).innerText = slice;
      }
    }

    function sendRollingData() {
      const text = document.getElementById('wordInput').value;
      if(!text) return;
      // Invert slider so dragging right makes delay smaller (faster)
      const speed = 400 - parseInt(document.getElementById('speedSlider').value);
      statusEl.innerText = 'ANIMATING'; statusEl.style.color = '#00ffcc';
      fetch(`/scroll?msg=${encodeURIComponent(text)}&speed=${speed}`);
    }

    function sendStaticData() {
      const text = document.getElementById('wordInput').value;
      statusEl.innerText = 'TX...'; statusEl.style.color = '#ffcc00';
      fetch(`/text?msg=${encodeURIComponent(text)}`)
        .then(() => { statusEl.innerText = 'ONLINE'; statusEl.style.color = '#00ffcc'; });
    }

    function clearWords() {
      document.getElementById('wordInput').value = '';
      updateWordPreview();
      fetch('/clear');
    }
  </script>
</body>
</html>
)rawliteral";

// ================= RUNTIME EXECUTION BACKEND =================

void handleDraw() {
  isScrolling = false; // Stop marquee if drawing manually
  int m = server.arg("m").toInt(); 
  int d = server.arg("d").toInt(); 
  int s = server.arg("s").toInt(); 
  if (m < 0 || m >= MAX_DEVICES || d < 0 || d >= 8 || s < 0 || s > 7) return;

  displayState[m][s] |= (1 << d); 
  mx.setColumn(m, s, displayState[m][s]);
  server.send(200, "text/plain", "OK");
}

void handleText() {
  isScrolling = false; // Stop animation for static snapshot
  String msg = server.arg("msg");
  renderTextSlice(msg);
  server.send(200, "text/plain", "OK");
}

void handleScroll() {
  String msg = server.arg("msg");
  int speed = server.arg("speed").toInt();
  if (speed <= 10) speed = 150;
  
  scrollDelay = speed;
  // Pad with 32 spaces so the text smoothly rolls in from the right edge
  scrollText = "                                " + msg + " ";
  scrollIndex = 0;
  isScrolling = true;
  
  server.send(200, "text/plain", "OK");
}

void setup() {
  Serial.begin(115200);
  
  mx.begin();
  mx.control(MD_MAX72XX::SHUTDOWN, MD_MAX72XX::OFF);
  mx.control(MD_MAX72XX::INTENSITY, 1); // Keep low for Wi-Fi power stability
  mx.clear();
  
  WiFi.softAP("Matrix-Studio-PRO", "12345678");
  
  server.on("/", []() { server.send(200, "text/html", html); });
  server.on("/draw", handleDraw);
  server.on("/text", handleText);
  server.on("/scroll", handleScroll);
  server.on("/clear", []() { 
    isScrolling = false;
    mx.clear(); 
    memset(displayState, 0, sizeof(displayState)); 
    server.send(200); 
  });
  
  server.begin();
  Serial.println("Matrix Studio PRO Active at 192.168.4.1");
}

void loop() { 
  server.handleClient(); 
  
  // Non-blocking timer: executes scrolling step without freezing the Wi-Fi server
  if (isScrolling && (millis() - lastScrollTime >= scrollDelay)) {
    lastScrollTime = millis();
    stepScroll();
  }
}