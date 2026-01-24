// ===============================
// MUST REMAIN UNUSED
// created to suppress the warnings, no other use.
// TODO: switch to EM_ASM/EM_JS if any workaround for standalone wasm is found
// ===============================

mergeInto(LibraryManager.library, {
  __hal_put_pixel: function (x, y, rgba) {
    alert(
      "ERROR: __hal_put_pixel is not available for emcc generated js files"
    );
  },

  __hal_clear: function (rgba) {
    alert("ERROR: __hal_clear is not available for emcc generated js files");
  },

  __hal_present: function () {
    alert("ERROR: __hal_present is not available for emcc generated js files");
  },

  __hal_get_width: function () {
    alert(
      "ERROR: __hal_get_width is not available for emcc generated js files"
    );
  },

  __hal_get_height: function () {
    alert(
      "ERROR: __hal_get_height is not available for emcc generated js files"
    );
  },
});
