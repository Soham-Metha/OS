# webOS

## Changelogs

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
- [26.01.29](https://github.com/Soham-Metha/OS/tree/f187d1285b05bebaca7fb747ecdf3650fa2e8516) Mouse IRQ for native, cursor draw, files re-organization to better seperate kernel and userspace( ... with a few exceptions )
- [26.01.28](https://github.com/Soham-Metha/OS/tree/6af5e854da971e58c0e4e3cd1adf38eddb4a6b59) add yield and exit syscalls, create "tasks", io lib, and simple shell app, try web workers for context switching in web.
- [26.01.27](https://github.com/Soham-Metha/OS/tree/268dda240b9d32f83cf06f1d142313b4526dc5b8) add keyboard and timer interrupt support to native arch, initial implementation of a non-preemptive scheduler, simple shell app that echoes input.
- [26.01.26](https://github.com/Soham-Metha/OS/tree/c7c386c6697781bc3a38c6529d54c256df8c5185): add native arch support
- [26.01.25](https://github.com/Soham-Metha/OS/tree/b15f1f034a9d9c451c437d2e280ef9091d63d98b): add event handling, continue fixing boundary violations
- [26.01.24](https://github.com/Soham-Metha/OS/tree/a07f27ea88a680184bbc3a081f5337c17dda49cf): create simple malloc, window manager, compositor, surfaces, start kernel render loop, start fixing boundary violations
- [26.01.23](https://github.com/Soham-Metha/OS/tree/7f9406a464f8bf972c9ed653e2e9d5d4a992fab8): Switch from text to graphics mode, implementing font renderer, terminals, add support for mobile phone keyboards
- [26.01.22](https://github.com/Soham-Metha/OS/tree/6d98e2f280fb3d7899e58401265e213068c2c3c1): Added keyboard interrupts (for browser), tty input buffer, removed all dependencies (stdio, etc etc), switch website from textarea to canvas
- [26.01.19](https://github.com/Soham-Metha/OS/tree/4fea8d49ac7a90ee7eea3851c46921642d54cde5): Followed Dr. Birch's osdev tutorial 1.
- [26.01.18](https://github.com/Soham-Metha/OS/tree/fb9704466cca8777cd1382515cf607e6dc9bf741): Initial Commit
