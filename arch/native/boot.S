.equ MULTIBOOT_HEADER_MAGIC          , 0x1BADB002
.equ MULTIBOOT_HEADER_FLAGS          , 0x00000007   # VIDEO_MODE (4) + MEMORY_INFO (2) + PAGE_ALIGN (1)
.equ MULTIBOOT_HEADER_CHECKSUM       , -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS) & 0xFFFFFFFF

.section .text 
    .code32

    /* MULTIBOOT HEADER of 8 bytes + checksum */
    .align  4
    .long   MULTIBOOT_HEADER_MAGIC
    .long   MULTIBOOT_HEADER_FLAGS
    .long   MULTIBOOT_HEADER_CHECKSUM

    .long 0,0,0,0,0

    /* video mode request */
    .long 0                   /* mode type: 0 = linear framebuffer */
    .long 1024                /* width  */
    .long 768                 /* height */
    .long 32                  /* bpp    */

    .globl  start
    .globl  gdtFlush
    .extern kernelMain

start:
    CLI
    MOVL    $stackSpace + 8192,     %ESP
    
    PUSH    %EBX        # mbi
    PUSH    %EAX        # magic
    CALL    kernelMain
    ADD     $8, %ESP

    HLT

haltKernel:
    CLI
    HLT
    JMP     haltKernel

gdtFlush:
    MOVL    4(%ESP),                %EAX
    LGDT    (%EAX)
    MOVW    $0x10,                  %AX
    MOVW    %AX,                    %DS
    MOVW    %AX,                    %ES
    MOVW    %AX,                    %FS
    MOVW    %AX,                    %GS
    MOVW    %AX,                    %SS
    LJMP    $0x08,                  $flush      /* LJMP to update the CS */

flush:
    RET

.section .bss
    .lcomm  stackSpace,     8192
