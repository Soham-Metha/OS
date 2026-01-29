window.kernel = {
  wasm: null,
};

const IRQ_TIMER = 0;
const IRQ_KEYBOARD = 1;
const IRQ_MOUSE = 12;

// ===============================
// Terminal configuration
// ===============================

const canvas = document.getElementById("screen");
const ctx = canvas.getContext("2d");

canvas.width = window.innerWidth;
canvas.height = window.innerHeight;

const WIDTH = canvas.width;
const HEIGHT = canvas.height;

const imageData = ctx.createImageData(WIDTH, HEIGHT);
const fb = imageData.data;

// ===============================
// keyboard
// ===============================

const SCAN_CODE_SET1 = {
  Digit1: 0x02, Digit2: 0x03, Digit3: 0x04, Digit4: 0x05, Digit5: 0x06, Digit6: 0x07, Digit7: 0x08, Digit8: 0x09, Digit9: 0x0a, Digit0: 0x0b, Minus: 0x0c, Equal: 0x0d, Backspace: 0x0e,
  Tab: 0x0f, KeyQ: 0x10, KeyW: 0x11, KeyE: 0x12, KeyR: 0x13, KeyT: 0x14, KeyY: 0x15, KeyU: 0x16, KeyI: 0x17, KeyO: 0x18, KeyP: 0x19, BracketLeft: 0x1a, BracketRight: 0x1b, Enter: 0x1c,
  ControlLeft: 0x1d, KeyA: 0x1e, KeyS: 0x1f, KeyD: 0x20, KeyF: 0x21, KeyG: 0x22, KeyH: 0x23, KeyJ: 0x24, KeyK: 0x25, KeyL: 0x26, Semicolon: 0x27, Quote: 0x28, Backquote: 0x29,
  ShiftLeft: 0x2a, Backslash: 0x2b, KeyZ: 0x2c, KeyX: 0x2d, KeyC: 0x2e, KeyV: 0x2f, KeyB: 0x30, KeyN: 0x31, KeyM: 0x32, Comma: 0x33, Period: 0x34, Slash: 0x35, ShiftRight: 0x36, NumpadMultiply: 0x37,
  AltLeft: 0x38, Space: 0x39, CapsLock: 0x3a,
  NumLock: 0x45,
  Numpad7: 0x47, Numpad8: 0x48, Numpad9: 0x49, NumpadSubtract: 0x4a,
  Numpad4: 0x4b, Numpad5: 0x4c, Numpad6: 0x4d, NumpadAdd: 0x4e,
  Numpad1: 0x4f, Numpad2: 0x50, Numpad3: 0x51,
  Numpad0: 0x52, NumpadDecimal: 0x53,
};

window.addEventListener("keydown", (e) => {
  if (!window.kernel.wasm) return;

  const scancode = SCAN_CODE_SET1[e.code];
  if (scancode === undefined) return;

  console.log(e.code, scancode);
  window.kernel.wasm.exports.kernel_irq_wrapper(IRQ_KEYBOARD, scancode, 0, 0);
});

window.addEventListener("keyup", (e) => {
  if (!window.kernel.wasm) return;

  const scancode = SCAN_CODE_SET1[e.code];
  if (scancode === undefined) return;

  const breakcode = scancode | 0x80;

  window.kernel.wasm.exports.kernel_irq_wrapper(IRQ_KEYBOARD, breakcode, 0, 0);
});

canvas.addEventListener("mousemove", (e) => {
  window.kernel.wasm.exports.kernel_irq_wrapper(
    IRQ_MOUSE,
    e.movementX,
    e.movementY,
    e.buttons
  );
});

canvas.addEventListener("mousedown", (e) => {
  window.kernel.wasm.exports.kernel_irq_wrapper(
    IRQ_MOUSE,
    0,
    0,
    e.buttons
  );
});

canvas.addEventListener("mouseup", (e) => {
  window.kernel.wasm.exports.kernel_irq_wrapper(
    IRQ_MOUSE,
    0,
    0,
    e.buttons
  );
});

// ===============================
// HAL exports (WASM imports)
// ===============================

function __hal_put_pixel(x, y, rgba) {
  if (x < 0 || y < 0 || x >= WIDTH || y >= HEIGHT) return;

  const idx = (y * WIDTH + x) * 4;
  fb[idx + 0] = (rgba >> 24) & 0xff; // R
  fb[idx + 1] = (rgba >> 16) & 0xff; // G
  fb[idx + 2] = (rgba >> 8) & 0xff; // B
  fb[idx + 3] = (rgba >> 0) & 0xff; // A
}

function __hal_clear(rgba) {
  for (let idx = 0; idx < fb.length; idx += 4) {
    fb[idx + 0] = (rgba >> 24) & 0xff; // R
    fb[idx + 1] = (rgba >> 16) & 0xff; // G
    fb[idx + 2] = (rgba >> 8) & 0xff; // B
    fb[idx + 3] = (rgba >> 0) & 0xff; // A
  }
}

function __hal_present() {
  ctx.putImageData(imageData, 0, 0);
}

function __hal_get_width() {
  return WIDTH;
}

function __hal_get_height() {
  return HEIGHT;
}

// ===============================
// fetch wasm instance
// ===============================

async function boot() {
  const response = await fetch("./build/shell.wasm");
  const bytes = await response.arrayBuffer();

  const imports = {
    env: {
      __hal_put_pixel,
      __hal_clear,
      __hal_present,
      __hal_get_width,
      __hal_get_height,
    },
  };

  const { instance } = await WebAssembly.instantiate(bytes, imports);
  window.kernel.wasm = instance;
}
