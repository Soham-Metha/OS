# webOS

```
                    ┌─────────────────────────┐
                    │       apps/             │
                    │     Shell, etc.         │
                    └────────────┬────────────┘
                                 │
                    ┌────────────▼────────────┐
                    │        osapi/           │
                    │ OS-specific glue / APIs │
                    └────────────┬────────────┘
                                 │
              ┌──────────────────┴──────────────────┐
              │                                     │
   ┌──────────▼──────────┐               ┌──────────▼──────────┐
   │      common/        │               │      kernel/        │
   │  Standalone libs    │               │   OS implementation │
   │                     │               │                     │
   │ gfx                 │               │ (glues common libs) │
   │ VIREX/SASM          │<─────────────>│ interrupts          │
   │ strings             │               │ filesystem          │
   │ memory utilities    │               │ heap                │
   │ events              │               │ scheduler           │
   └─────────────────────┘               └──────────┬──────────┘
                                                    │
                                requires primitives │
                                 ┌──────────────────┘
                                 │
                    ┌────────────▼────────────┐
                    │          hal/           │
                    │ Native / Browser        │
                    └────────────┬────────────┘
                                 │
                    ┌────────────▼────────────┐
                    │       Platform          │
                    │ x86 / Browser / WASM    │
                    └─────────────────────────┘
```

```
.
├── build/                              # Generated build artifacts
│   ├── OS/                             # Native bootable OS image contents
│   │   └── boot/
│   │       └── grub/
│   │           └── grub.cfg            # GRUB bootloader configuration
│   ├── os.iso                          # Bootable native OS ISO
│   └── shell.wasm                      # Browser/WASM build of the OS shell
│
├── src/                                # OS source tree
│   │
│   ├── apps/                           # User-space applications
│   │   ├── shell.c                     # Main shell application
│   │   └── shell.h
│   │
│   ├── common/                         # Standalone, reusable libraries
│   │   │                               # Designed to be usable independently
│   │   │                               # of the OS; the OS supplies required
│   │   │                               # platform primitives (malloc, memcpy, etc.)
│   │   │
│   │   ├── gfx/                        # Standalone graphics library
│   │   │   ├── graphics.h              # 2D/3D graphics primitives and Canvas
│   │   │   ├── gfx_tests.h             # Graphics library tests/examples
│   │   │   ├── examples/               # Graphics library demonstration assets
│   │   │   │   ├── generated/          # Generated C representations of assets
│   │   │   │   ├── images/              # Source image assets for examples
│   │   │   │   └── models/              # 3D models and textures
│   │   │   └── scripts/                 # Host-side graphics asset utilities
│   │   │       ├── obj_file_handler.c   # OBJ processing/conversion
│   │   │       └── ppm_handler.c        # PPM image processing/conversion
│   │   │
│   │   ├── virex/                       # Standalone VIREX execution library
│   │   │   ├── virex.h                  # VIREX virtual execution environment
│   │   │   └── sasm.h                   # SASM instruction set / VM implementation
│   │   │
│   │   ├── errors.h                     # Errors and simple exception handling
│   │   ├── event.h
│   │   ├── memmanager.h                 # Region-based memory management/Arena
│   │   ├── strings.h                    # string-view
│   │   └── types.h                      # Primitive types and "result" definition
│   │
│   ├── drivers/                         # Hardware/device drivers
│   │   ├── keyboard.h                   # Keyboard scancodes
│   │   └── tty.h                        # Ring-buffer
│   │
│   ├── hal/                             # Hardware/platform abstraction
│   │   ├── hal.h                        # Common HAL interface
│   │   │
│   │   ├── browser/                     # Browser/WASM specific implementation
│   │   └── native/                      # Native x86 specific implementation
│   │
│   ├── kernel/                          # OS-specific kernel implementation
│   │   ├── fs/                          # Kernel filesystem subsystem
│   │   │   ├── disk.h                   # Disk/block-device interface
│   │   │   └── fs.h                     # Filesystem implementation/API
│   │   │
│   │   ├── heap.h                       # Kernel heap / allocation implementation
│   │   ├── interrupt.c
│   │   ├── interrupt.h
│   │   ├── kernel.c
│   │   ├── kernel.h
│   │   └── scheduler.h
│   │
│   ├── osapi/                           # OS-specific interfaces
│   │   │
│   │   ├── gfx/                         # OS graphics stack built on common/gfx
│   │   │   ├── compositor.h             # Compositor; operates on "Surface"s
│   │   │   ├── event.c                  # Event handling impl, dependant on wm.h for mouse rendering
│   │   │   ├── font.h                   # Font rendering built on "Surface"
│   │   │   ├── terminal.h               # Char-level abstraction over "Surface"
│   │   │   └── wm.h                     # manage terminal windows
│   │   │
│   │   ├── io.h                         # putch/printf/getch/get_line impl
│   │   ├── osapi.c
│   │   └── osapi.h                      # syscalls
│   │
│   └── platform/                        # Platform-specific build/runtime glue
│       ├── browser/                     # Browser platform integration
│       │   ├── hal.js                   # JavaScript ↔ WASM platform bridge
│       │   └── style.css                # Browser UI styling
│       │
│       └── native/                     # Native platform integration
│           └── native.ld                # x86 kernel linker script
│
├── tools/
│   └── emsdk_install.sh                # Emscripten SDK installation helper
│
├── index.html                          # Browser entry point for WASM OS
├── Makefile
├── README.md
└── LICENSE                             # GPLv3 license
```

# Refactoring TODOs

## Common Libraries

### GFX

* [x] Make `common/gfx` completely OS-independent
* [x] Keep `graphics.h` / `gfx_tests.h` free of OS includes
* [ ] Define a clean host/backend interface for rendering output
* [ ] Decide whether the backend should expose a pixel buffer, canvas sink, callback, etc.
* [ ] Keep the backend interface independent of HAL
* [x] Decouple `Canvas` from HAL-specific `put_pixel`
* [x] Ensure GFX can theoretically be compiled/used outside webOS
* [ ] Keep GFX examples/tests independent of webOS
* [ ] Decide whether `math` should remain part of GFX or become a separate library

### VIREX / SASM

* [x] Keep `common/virex` completely OS-independent
* [x] Remove remaining OS-specific dependencies
* [x] Keep required host functions (`printf`, etc.) externally provided
* [x] Verify VIREX/SASM can be embedded independently of the OS
* [ ] Define/document the host/runtime interface required by VIREX/SASM
* [ ] Get SASM/VIREX working on native

### Event Component System

* [ ] Design a proper ECS
* [ ] Define generic event/component representation
* [ ] Define event queues and dispatching
* [ ] Define event ownership/routing
* [ ] Separate event generation from event consumption
* [x] Make interrupts/keyboard/input feed events rather than directly calling WM logic
* [ ] Move the event implementation fully into `common`
* [ ] Remove `event.c -> wm.h` dependency
* [ ] Remove WM-specific concepts from the event library
* [ ] Make the event system usable by both VIREX and WM
* [ ] Allow multiple consumers to observe relevant events independently
* [ ] Make the event component system independently testable

### Existing Common Libraries

* [x] Review `strings`
* [ ] Review `memmanager`
* [x] Review `errors`
* [x] Review `types`
* [ ] Identify remaining OS dependencies
* [ ] Document host-provided functions required by common libraries
* [ ] Keep expanding `common` only when a component has a genuinely reusable boundary
* [ ] Look for additional standalone libraries that can be extracted from OS code

---

## GFX / WM Architecture

* [ ] Take reference from Manim/Panim for the GFX system refactor
* [x] Keep GFX primitives as an independent library
* [ ] Keep WM / compositor as OS-side code
* [ ] Clarify ownership/lifetime of `Surface`
* [x] Fix the `Surface` ↔ `Canvas` ↔ HAL boundary
* [x] Decide how a GFX `Canvas` ultimately reaches the display backend
* [x] Prevent GFX from depending directly on HAL
* [x] Make compositor depend on GFX, not the reverse
* [ ] Make `Surface` an OS-side abstraction over a GFX `Canvas`
* [ ] Keep `font.h` / `terminal.h` / WM-specific functionality above the GFX library
* [ ] Remove direct display/HAL access from WM/compositor
* [ ] Remove the shell's dependency on `gfx_tests.h`

---

## Event / Input Architecture

* [ ] Remove direct interrupt → WM coupling
* [ ] Remove direct interrupt → refresh/render logic
* [ ] Make keyboard/input handlers produce generic events
* [x] Make the event system the intermediary between input and consumers
* [ ] Allow WM to consume relevant events
* [ ] Allow VIREX/applications to consume relevant events
* [ ] Define a clean event propagation/dispatch model
* [ ] Separate interrupt handling from event dispatch
* [ ] Route timer/input events through the common event mechanism

Target:

```text
Hardware
    ↓
Interrupt
    ↓
Driver
    ↓
Event System
    ├──→ WM
    └──→ VIREX / Application
```

---

## Scheduler / Execution

* [ ] Keep scheduler as kernel functionality
* [ ] Do **not** model VIREX as a scheduler job yet
* [ ] Refactor the current C-function-based scheduler
* [ ] Fix process/function yield semantics
* [ ] Fix exit/lifetime handling
* [ ] Define execution-context states
  * [ ] Ready
  * [ ] Running
  * [ ] Blocked
  * [ ] Exited
* [ ] Establish a proper execution/process abstraction
* [ ] Define what the scheduler actually schedules
* [ ] Only then decide how VIREX execution maps onto scheduler entities
* [ ] Investigate the eventual `VM instance = job` model

---

## OSAPI

* [ ] Review everything currently under `osapi/`
* [ ] Separate genuine OS interfaces from reusable library functionality
* [ ] Keep OS-specific interfaces in `osapi`
* [x] Avoid making common libraries depend on `osapi`
* [ ] Define the eventual VIREX-facing OS interface
* [ ] Define the eventual graphics interface exposed to VIREX applications
* [ ] Remove anything from `osapi` that can be made into a standalone common library

---

## Kernel / HAL / Drivers

* [ ] Keep interrupts in the kernel
* [ ] Keep scheduler in the kernel
* [ ] Keep filesystem in the kernel/OS
* [ ] Keep drivers outside `common`
* [ ] Keep HAL outside `common`
* [ ] Make HAL provide platform-specific implementations rather than being called directly by reusable libraries
* [ ] Clean up `interrupt.c` dependencies
* [ ] Separate interrupt dispatch from event generation
* [ ] Separate timer interrupts from input interrupts
* [ ] Make interrupt handling feed the new event mechanism
* [ ] Define clean driver → HAL boundaries

---

## Dependency Cleanup

* [ ] Audit **every** `#include`
* [ ] Identify dependencies going *up* the architecture
* [x] Remove `common → OS` dependencies
* [x] Remove `GFX → HAL` dependencies
* [ ] Remove `Event → WM` dependency
* [x] Remove `VIREX/SASM → OSAPI` dependencies
* [ ] Remove WM → interrupt implementation dependencies
* [ ] Ensure common libraries only depend on their own code + explicitly provided host functions
* [ ] Make each common library independently testable
* [ ] Update Makefile paths/dependencies after the refactor
* [ ] Verify both native and WASM builds after each major refactor

---

## Filesystem

* [ ] Fix filesystem persistence in browser
* [ ] Separate filesystem implementation from storage backend
* [ ] Make browser persistence a backend rather than a special case

---

## Final Architectural Target

* [ ] **Common = reusable, OS-independent libraries**
* [ ] **Kernel = OS core and resource management**
* [ ] **HAL / Drivers = platform and hardware integration**
* [ ] **OSAPI = OS-specific interfaces and glue**
* [ ] **Apps = consumers of the OS**
* [ ] **GFX = standalone graphics library**
* [ ] **VIREX / SASM = standalone execution library**
* [ ] **Event System = standalone event/component library**
* [ ] **Glue code = connects components without becoming a monolithic subsystem**

### Refactoring Order

* [ ] Event / ECS design
* [ ] Event library extraction
* [ ] WM / Event decoupling
* [ ] GFX / Surface / backend cleanup
* [ ] WM / Compositor cleanup
* [ ] Scheduler refactor
* [ ] VIREX / Scheduler integration
* [ ] OSAPI cleanup
* [ ] Extract additional common libraries
* [ ] Native / WASM validation

## Arch

```
digraph G {
    rankdir=TB;
    splines=ortho;
    nodesep=0.45;
    ranksep=0.65;

    graph [
        bgcolor="white",
        pad=0.25
    ];

    node [
        shape=box,
        style="rounded,filled",
        fontname="Helvetica",
        fontsize=11,
        margin="0.15,0.08",
        color="#555555",
        penwidth=1.2
    ];

    edge [
        fontname="Helvetica",
        fontsize=9,
        color="#555555",
        penwidth=1.2,
        arrowsize=0.7
    ];


    // =========================================================
    // Layer labels
    // =========================================================

    L1 [label="USER APPLICATIONS",
        shape=plaintext,
        fontname="Helvetica-Bold",
        fontsize=11,
        fontcolor="#444444"];

    L2 [label="OS INTERFACES",
        shape=plaintext,
        fontname="Helvetica-Bold",
        fontsize=11,
        fontcolor="#444444"];

    LC [label="COMMON LIBRARIES",
        shape=plaintext,
        fontname="Helvetica-Bold",
        fontsize=11,
        fontcolor="#24736F"];

    L3 [label="OS SERVICES",
        shape=plaintext,
        fontname="Helvetica-Bold",
        fontsize=11,
        fontcolor="#444444"];

    L4 [label="KERNEL",
        shape=plaintext,
        fontname="Helvetica-Bold",
        fontsize=11,
        fontcolor="#444444"];

    L5 [label="HARDWARE ABSTRACTION",
        shape=plaintext,
        fontname="Helvetica-Bold",
        fontsize=11,
        fontcolor="#444444"];

    L6 [label="HARDWARE / PLATFORM",
        shape=plaintext,
        fontname="Helvetica-Bold",
        fontsize=11,
        fontcolor="#444444"];


    // =========================================================
    // Layer 1 — User Applications
    // =========================================================

    Shell [
        label="Shell",
        fillcolor="#E8F1FF",
        color="#4A78B8"
    ];

    VProgram [
        label="VIREX Program",
        fillcolor="#E8F1FF",
        color="#4A78B8"
    ];

    { rank=same; L1; Shell; VProgram; }


    // =========================================================
    // Layer 2 — OS Interfaces
    // =========================================================

    VIREX [
        label="VIREX\nExecution Interface",
        fillcolor="#EDE7F6",
        color="#7656A6"
    ];

    WM [
        label="Window Manager",
        fillcolor="#EDE7F6",
        color="#7656A6"
    ];

    { rank=same; L2; VIREX; WM; }


    // =========================================================
    // Common Libraries
    // =========================================================

    GFX [
        label="GFX",
        fillcolor="#E0F2F1",
        color="#3F817D"
    ];

    VIREXLib [
        label="VIREX / SASM",
        fillcolor="#E0F2F1",
        color="#3F817D"
    ];

    EventLib [
        label="Event",
        fillcolor="#E0F2F1",
        color="#3F817D"
    ];

    MemLib [
        label="Memory",
        fillcolor="#E0F2F1",
        color="#3F817D"
    ];

    // UtilityLib [
    //     label="Strings / Types / Errors",
    //     fillcolor="#E0F2F1",
    //     color="#3F817D"
    // ];

    { rank=same; LC; GFX; VIREXLib; EventLib; MemLib; }


    // =========================================================
    // Layer 3 — OS Services
    // =========================================================

    Scheduler [
        label="Scheduler",
        fillcolor="#E8F5E9",
        color="#4E8B57"
    ];

    Compositor [
        label="Compositor",
        fillcolor="#E8F5E9",
        color="#4E8B57"
    ];

    Surface [
        label="Surface",
        fillcolor="#E8F5E9",
        color="#4E8B57"
    ];

    { rank=same; L3; Scheduler; Compositor; Surface; }


    // =========================================================
    // Layer 4 — Kernel
    // =========================================================

    Interrupts [
        label="Interrupts",
        fillcolor="#FFF3E0",
        color="#C47A22"
    ];

    Memory [
        label="Memory Manager",
        fillcolor="#FFF3E0",
        color="#C47A22"
    ];

    FS [
        label="File System",
        fillcolor="#FFF3E0",
        color="#C47A22"
    ];

    { rank=same; L4; Interrupts; Memory; FS; }


    // =========================================================
    // Layer 5 — Hardware Abstraction
    // =========================================================

    Drivers [
        label="Drivers",
        fillcolor="#FCE4EC",
        color="#B84A68"
    ];

    HAL [
        label="HAL",
        fillcolor="#FCE4EC",
        color="#B84A68"
    ];

    { rank=same; L5; Drivers; HAL; }


    // =========================================================
    // Layer 6 — Hardware / Platform
    // =========================================================

    CPU [
        label="CPU",
        fillcolor="#F5F5F5",
        color="#777777"
    ];

    Display [
        label="Display",
        fillcolor="#F5F5F5",
        color="#777777"
    ];

    Input [
        label="Input",
        fillcolor="#F5F5F5",
        color="#777777"
    ];

    Storage [
        label="Storage",
        fillcolor="#F5F5F5",
        color="#777777"
    ];

    { rank=same; L6; CPU; Display; Input; Storage; }


    // =========================================================
    // Applications → OS interfaces
    // =========================================================

    Shell -> VProgram;
    Shell -> WM;
    Shell -> FS;

    VProgram -> VIREX;


    // =========================================================
    // OS interfaces → Common libraries
    // =========================================================

    VIREX -> VIREXLib;

    WM -> Compositor;
    Compositor -> Surface;

    Surface -> GFX;
    Surface -> HAL;


    // =========================================================
    // Common library relationships
    // =========================================================

    // VIREX/SASM uses generic common utilities.
    VIREXLib -> MemLib;
    // VIREXLib -> UtilityLib;

    // Event is consumed by OS components.
    Scheduler -> EventLib;
    Interrupts -> EventLib;
    WM -> EventLib;


    // =========================================================
    // OS services → Kernel
    // =========================================================

    Scheduler -> Memory;

    // =========================================================
    // Kernel relationships
    // =========================================================

    Interrupts -> Scheduler;
    Interrupts -> Drivers;

    FS -> Drivers;


    // =========================================================
    // HAL / Drivers
    // =========================================================

    Drivers -> HAL;

    HAL -> CPU;
    HAL -> Display;
    HAL -> Input;
    HAL -> Storage;


    // =========================================================
    // Common library host dependencies
    // =========================================================

    // Common libraries are provided with host functionality
    // by the OS rather than depending on the OS directly.
    MemLib -> Memory [style=dashed, dir=back];


    // =========================================================
    // Layer ordering
    // =========================================================

    L1 -> L2  [style=invis];
    L2 -> LC  [style=invis];
    LC -> L3  [style=invis];
    L3 -> L4  [style=invis];
    L4 -> L5  [style=invis];
    L5 -> L6  [style=invis];
}
```

## Changelogs

- [26.03.30](https://github.com/Soham-Metha/OS/tree/5f3a8f650ea961abb726735a6cd8fee48713e6e4) add texturing, bug fixing, code cleanup, add more examples, hardcode the binary for VM example
- [26.03.29](https://github.com/Soham-Metha/OS/tree/ec71a63abda0e944bcc6a074d6454275f28360bb) move all examples from extras to examples dir, add script to process PPM images.
- [26.03.28](https://github.com/Soham-Metha/OS/tree/308a67df73b2af49669ca8f5465669207655bc32) add clipping to GFX3D, render a teapot, get GFX working on native(increase native stack size), virex no longer hangs on native.
- [26.03.27](https://github.com/Soham-Metha/OS/tree/00c0e9b1bb97100dc3bc061bdcde3a24634f2814) switch native to 1920x1080, add gun 3d model, and Point3f operations, reorganize GFX3D code, add camera movements
- [26.03.19](https://github.com/Soham-Metha/OS/tree/d9dc5b1d6750d50ee2133c2b0769ae9445c8f786) add sqrt/tan and matrix operations,chained matrix multiplication, initial 3D pipeline using triangles and 3D mesh, by following javidx9's game engine tutorial, add fish 3D model
- [26.03.18](https://github.com/Soham-Metha/OS/tree/9e403dd0c9816916b09f5819e2cd04983fa5deb3) continue working of 3d by following tsoding tutorial, add sin/cos via LUT
- [26.03.04](https://github.com/Soham-Metha/OS/tree/d1604072028254b698fc2a58202b9a5aa3f623f7) get virex working (on web), merge PR into main, create simple shell handler, start work on graphics 2D:rect, circ, line, triangle, alpha blending, super/under sampling, testing patterns mostly by following tsoding's tutorial(olivec).
- [26.02.22](https://github.com/Soham-Metha/OS/tree/378083002e08444a3d81e2fe8cac43749e968a4f) continue work on error-handling, remove all virex dependencies and move virex to kernel modules
- [26.02.21](https://github.com/Soham-Metha/OS/tree/fd034dc3de3f65cc18a92049bc449b24d93c6ed3) Start error handling rework
- [26.02.18](https://github.com/Soham-Metha/OS/tree/91cde8d1467f0802d5cf74bde7fe4ae7826892f9) Continue working on virex, cleaned up code and added simple test, switched result to be more generic.
- [26.02.17](https://github.com/Soham-Metha/OS/tree/407ee3fa3abe8cf9c4b1b126531223b7fcb7fb4e) remove all file handling and stdio dependency from virex.
- [26.02.11](https://github.com/Soham-Metha/OS/tree/3586cccb477006fbe8963e0453c7ee7519bc46fa) switch wasm to shared memory, and remove (most of) [virex](https://github.com/Soham-Metha/OS/tree/e3858f3cddb68727bdfcd90ae7784f8589714efd) dependencies, also added the string_view implementation.
- [26.02.07](https://github.com/Soham-Metha/OS/tree/50f9c9b9d0b3a221f37928c39d0425fb06375e54) virex code cleanup, organize into single file headers
- [26.02.06](https://github.com/Soham-Metha/OS/tree/1cc41c82a4aa1369938df49f9f67ce3a7374b863) add "arena_free" and start work on porting virex(compiler) to OS
- [26.02.03](https://github.com/Soham-Metha/OS/tree/1d556b0958e431641fbb599a8f25587d048d6ec6) Code cleanup/refactor and add "inode_create"
- [26.02.02](https://github.com/Soham-Metha/OS/tree/91d9976abbff4350504db95ec3695ab195fdfd8b) Create Inital in-memory file system by following Dr. Birch
- [26.02.01](https://github.com/Soham-Metha/OS/tree/3b46c6b7cd4d15d1ec1772754bb162218de23f7b) Create a result impelmentation and arena(region based memory manager)
- [26.01.31](https://github.com/Soham-Metha/OS/tree/e07fa7b6faa71c20c692c3a5c2d07718719f8a4e) Add var args and printf
- [26.01.30](https://github.com/Soham-Metha/OS/tree/fb050e70ff4b9f81b0a492744957dacdef0ff0c9) Mouse and cursor support for web, break web on mobile
- [26.01.29](https://github.com/Soham-Metha/OS/tree/f187d1285b05bebaca7fb747ecdf3650fa2e8516) Mouse IRQ for native, cursor draw, files re-organization to better seperate kernel and userspace( ... with a few violations)
- [26.01.28](https://github.com/Soham-Metha/OS/tree/6af5e854da971e58c0e4e3cd1adf38eddb4a6b59) add yield and exit syscalls, create "tasks", io lib, and simple shell app, try web workers for context switching in web(discontinued).
- [26.01.27](https://github.com/Soham-Metha/OS/tree/268dda240b9d32f83cf06f1d142313b4526dc5b8) add keyboard and timer interrupt support to native arch, initial implementation of a non-preemptive scheduler, simple shell app that echoes input.
- [26.01.26](https://github.com/Soham-Metha/OS/tree/c7c386c6697781bc3a38c6529d54c256df8c5185): add native arch support
- [26.01.25](https://github.com/Soham-Metha/OS/tree/b15f1f034a9d9c451c437d2e280ef9091d63d98b): add event handling, continue fixing boundary violations
- [26.01.24](https://github.com/Soham-Metha/OS/tree/a07f27ea88a680184bbc3a081f5337c17dda49cf): create simple malloc, window manager, compositor, surfaces, start kernel render loop, start fixing boundary violations. Initial WM/Compositor/Surfaces implemented by taking help of AI.
- [26.01.23](https://github.com/Soham-Metha/OS/tree/7f9406a464f8bf972c9ed653e2e9d5d4a992fab8): Switch from text to graphics mode, implementing font renderer, terminals, add support for mobile phone keyboards
- [26.01.22](https://github.com/Soham-Metha/OS/tree/6d98e2f280fb3d7899e58401265e213068c2c3c1): Added keyboard interrupts (for browser), ring-buffer, removed all dependencies (stdio, etc etc), switch website from textarea to canvas
- [26.01.19](https://github.com/Soham-Metha/OS/tree/4fea8d49ac7a90ee7eea3851c46921642d54cde5): very simple web-based shell & project dir, with linux-based dependencies. Followed Dr. Birch's osdev tutorial 1.
- [26.01.18](https://github.com/Soham-Metha/OS/tree/fb9704466cca8777cd1382515cf607e6dc9bf741): Initial Commit
