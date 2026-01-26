window.kernel = {
  wasm: null,
};

// ===============================
// Terminal configuration
// ===============================
const canvas = document.getElementById("screen");
const input = document.getElementById("kbd");

const ctx = canvas.getContext("2d");

if (window.innerWidth <= 800) {
  canvas.width = 500;
  canvas.height = 250;
}

const WIDTH = canvas.width;
const HEIGHT = canvas.height;

const imageData = ctx.createImageData(WIDTH, HEIGHT);
const fb = imageData.data;

// ===============================
// keyboard
// ===============================

const IRQ_KEYBOARD = 1;

canvas.addEventListener("touchstart", () => {
  input.focus({ preventScroll: true });
});

canvas.addEventListener("mousedown", () => {
  input.focus();
});

window.addEventListener("keydown", (e) => {
  if (!window.kernel.wasm) return;

  let code = 0;

  if (e.key.length === 1) {
    code = e.key.charCodeAt(0);
  } else if (e.key === "Enter") {
    code = 10; // '\n'
  } else if (e.key === "Backspace") {
    code = 8;
  } else {
    return;
  }

  console.log(window.kernel.wasm.exports);
  window.kernel.wasm.exports.kernel_irq(IRQ_KEYBOARD, BigInt(code));
});

input.addEventListener("input", (e) => {
  const value = input.value;

  for (let i = 0; i < value.length; i++) {
    const ch = value.charCodeAt(i);
    window.kernel.wasm.exports.kernel_irq(IRQ_KEYBOARD, BigInt(ch));
  }

  input.value = "";
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
