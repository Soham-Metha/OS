.equ MULTIBOOT_HEADER_MAGIC          , 0x1BADB002
.equ MULTIBOOT_HEADER_FLAGS          , 0x00000007   # VIDEO_MODE (4) + MEMORY_INFO (2) + PAGE_ALIGN (1)
.equ MULTIBOOT_HEADER_CHECKSUM       , -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS) & 0xFFFFFFFF

.macro isr_err_stub num
    .globl isr_stub_\num
isr_stub_\num:
    pushl $\num
    call exception_handler
    add $4, %esp
    iret
.endm

.macro isr_no_err_stub num, cfunc
    .globl isr_stub_\num
isr_stub_\num:
    pushl $0
    pushl $\num
    pusha
    call \cfunc
    popa
    add $8, %esp
    iret
.endm

.extern exception_handler
.extern irq0_handler_c
.extern irq1_handler_c

isr_no_err_stub 0, exception_handler
isr_no_err_stub 1, exception_handler
isr_no_err_stub 2, exception_handler
isr_no_err_stub 3, exception_handler
isr_no_err_stub 4, exception_handler
isr_no_err_stub 5, exception_handler
isr_no_err_stub 6, exception_handler
isr_no_err_stub 7, exception_handler
isr_err_stub    8
isr_no_err_stub 9, exception_handler
isr_err_stub    10
isr_err_stub    11
isr_err_stub    12
isr_err_stub    13
isr_err_stub    14
isr_no_err_stub 15, exception_handler
isr_no_err_stub 16, exception_handler
isr_err_stub    17
isr_no_err_stub 18, exception_handler
isr_no_err_stub 19, exception_handler
isr_no_err_stub 20, exception_handler
isr_no_err_stub 21, exception_handler
isr_no_err_stub 22, exception_handler
isr_no_err_stub 23, exception_handler
isr_no_err_stub 24, exception_handler
isr_no_err_stub 25, exception_handler
isr_no_err_stub 26, exception_handler
isr_no_err_stub 27, exception_handler
isr_no_err_stub 28, exception_handler
isr_no_err_stub 29, exception_handler
isr_err_stub    30
isr_no_err_stub 31, exception_handler
isr_no_err_stub 32, irq0_handler_c
isr_no_err_stub 33, irq1_handler_c

.section .rodata
    .globl isr_stub_table
isr_stub_table:
    .irp n,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
        .long isr_stub_\n
    .endr

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
