// ===============================
// Terminal configuration
// ===============================
const canvas = document.getElementById("screen");
const ctx = canvas.getContext("2d");

const FONT_SIZE = 16;
const COLS = Math.floor(canvas.width / FONT_SIZE);
const ROWS = Math.floor(canvas.height / FONT_SIZE);

ctx.font = `${FONT_SIZE}px monospace`;
ctx.textBaseline = "top";

// Colors
const BG = "black";
const FG = "lime";

// Screen state
let cursorX = 0;
let cursorY = 0;

// Clear screen
ctx.fillStyle = BG;
ctx.fillRect(0, 0, canvas.width, canvas.height);
ctx.fillStyle = FG;

// ===============================
// Input buffer (keyboard)
// ===============================
const inputBuffer = [];

window.addEventListener("keydown", (e) => {
  if (e.key.length === 1) {
    inputBuffer.push(e.key.charCodeAt(0));
  } else if (e.key === "Enter") {
    inputBuffer.push(10); // '\n'
  } else if (e.key === "Backspace") {
    inputBuffer.push(8);
  }
  e.preventDefault();
});

// ===============================
// HAL exports (WASM imports)
// ===============================

// Write one character to screen
function __hal_write_char(c) {
  if (c === 10) {
    // '\n'
    cursorX = 0;
    cursorY++;
  } else if (c === 8) {
    // backspace
    if (cursorX > 0) {
      cursorX--;
      ctx.fillStyle = BG;
      ctx.fillRect(
        cursorX * FONT_SIZE,
        cursorY * FONT_SIZE,
        FONT_SIZE,
        FONT_SIZE
      );
      ctx.fillStyle = FG;
    }
  } else {
    ctx.fillText(
      String.fromCharCode(c),
      cursorX * FONT_SIZE,
      cursorY * FONT_SIZE
    );
    cursorX++;
  }

  if (cursorX >= COLS) {
    cursorX = 0;
    cursorY++;
  }

  if (cursorY >= ROWS) {
    scroll_screen();
    cursorY = ROWS - 1;
  }
}

// Non-blocking read
function __hal_read_char() {
  if (inputBuffer.length === 0) {
    return -1;
  }
  return inputBuffer.shift();
}

// ===============================
// Scroll screen
// ===============================
function scroll_screen() {
  const img = ctx.getImageData(
    0,
    FONT_SIZE,
    canvas.width,
    canvas.height - FONT_SIZE
  );
  ctx.putImageData(img, 0, 0);

  ctx.fillStyle = BG;
  ctx.fillRect(0, canvas.height - FONT_SIZE, canvas.width, FONT_SIZE);
  ctx.fillStyle = FG;
}
