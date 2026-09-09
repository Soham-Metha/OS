/*
 * vmapi.h
 *  Copyright (C) 2026 Soham Metha
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef VMAPI_1
#define VMAPI_1
#include <common/virex/virex.h>

bool virex_run(Vm* vm, Sasm_Executable* exec, int lim);
bool virex_test(void);

#endif

#ifdef IMPL_VMAPI_1
#undef IMPL_VMAPI_1

#define IMPL_GRAPHICS_1
#define IMPL_GRAPHICS3D_1
#define IMPL_KERN_VIREX_1
#define IMPL_TERMINAL_1
#define IMPL_USPACE_IO_1
#define IMPL_WM_1
#define MEM_MANAGER_IMPL
#define STRING_VIEW_IMPL

#include <common/event.h>
#include <common/gfx/gfx_tests.h>
#include <common/memmanager.h>
#include <common/strings.h>
#include <common/virex/virex.h>
#include <osapi/gfx/wm.h>
#include <osapi/io.h>

VM_Error vmcall_write(Vm* vm)
{
    uint32 count    = STACK_POP().u32;
    MemoryAddr addr = STACK_POP().u32;

    if (addr >= MAX_MEMORY_CAPACITY) {
        return ERR_ILLEGAL_MEMORY_ACCESS;
    }

    if (addr + count < addr || addr + count >= MAX_MEMORY_CAPACITY) {
        return ERR_ILLEGAL_MEMORY_ACCESS;
    }

    for (uint32 i = 0; i < count; i += 1) {
        if (vm $memory[addr + i] == '\\') {
            i += 1;
            if (i >= count)
                return ERR_ILLEGAL_OPERAND;
            else if (vm $memory[addr + i] == 'n')
                printf("\n");
            else if (vm $memory[addr + i] == '_')
                printf("_");
            else
                return ERR_ILLEGAL_OPERAND;
        } else if (vm $memory[addr + i] == '_') {
            printf(" ");
        } else {
            printf("%c", vm $memory[addr + i]);
        }
    }

    return ERR_OK;
}

VM_Error vmcall_alloc(Vm* vm)
{
    (void)vm;
    // stack_push(vm, region_alloc(arena, STACK_POP()));

    return ERR_OK;
}

VM_Error vmcall_free(Vm* vm)
{
    (void)vm;
    // clearGarbage(region);

    return ERR_OK;
}

VM_Error vmcall_print_f64(Vm* vm)
{
    printf(" %lf\n", STACK_POP().f32);
    return ERR_OK;
}

VM_Error vmcall_print_i64(Vm* vm)
{
    printf(" %" PRId64 "", STACK_POP().i32);
    return ERR_OK;
}

VM_Error vmcall_print_u64(Vm* vm)
{
    printf(" %" PRIu64 "", STACK_POP().u32);
    return ERR_OK;
}

VM_Error vmcall_print_ptr(Vm* vm)
{
    printf(" %p\n", STACK_POP().ptr);
    return ERR_OK;
}

VM_Error vmcall_dump_memory(Vm* vm)
{
    uint32 count    = STACK_POP().u32;
    MemoryAddr addr = STACK_POP().u32;

    if (addr >= MAX_MEMORY_CAPACITY) {
        return ERR_ILLEGAL_MEMORY_ACCESS;
    }

    if (addr + count < addr || addr + count >= MAX_MEMORY_CAPACITY) {
        return ERR_ILLEGAL_MEMORY_ACCESS;
    }

    for (uint32 i = 0; i < count; ++i) {
        printf(" %02X ", vm $memory[addr + i]);
        if (i % 16 == 15) {
            printf("\n ");
        }
    }
    printf("\n");

    return ERR_OK;
}

VM_Error vmcall_writeROM(Vm* vm)
{
    char* buffer    = STACK_POP().ptr;
    uint32 count    = STACK_POP().u32;
    MemoryAddr addr = STACK_POP().u32;

    if (addr >= MAX_MEMORY_CAPACITY) {
        return ERR_ILLEGAL_MEMORY_ACCESS;
    }

    if (addr + count < addr || addr + count >= MAX_MEMORY_CAPACITY) {
        return ERR_ILLEGAL_MEMORY_ACCESS;
    }

    memcpy(buffer, &vm $memory[addr], count);

    return ERR_OK;
}

bool loadStandardCallsIntoVm(Vm* vm)
{
    try(loadInternalCallIntoVm(vm, vmcall_alloc), "Unable to load vm call '%d'!", 0);
    try(loadInternalCallIntoVm(vm, vmcall_free), "Unable to load vm call '%d'!", 1);
    try(loadInternalCallIntoVm(vm, vmcall_print_f64), "Unable to load vm call '%d'!", 2);
    try(loadInternalCallIntoVm(vm, vmcall_print_i64), "Unable to load vm call '%d'!", 3);
    try(loadInternalCallIntoVm(vm, vmcall_print_u64), "Unable to load vm call '%d'!", 4);
    try(loadInternalCallIntoVm(vm, vmcall_print_ptr), "Unable to load vm call '%d'!", 5);
    try(loadInternalCallIntoVm(vm, vmcall_dump_memory), "Unable to load vm call '%d'!", 6);
    try(loadInternalCallIntoVm(vm, vmcall_write), "Unable to load vm call '%d'!", 7);
    return true;
ret_err:
    return false;
}

bool virex_run(Vm* vm, Sasm_Executable* exec, int lim)
{
    try(loadStandardCallsIntoVm(vm), "Unable to load vm calls", "");
    try(loadProgramIntoVm(vm, exec), "Unable to load program", "");
    try(executeProgram(vm, lim), "Unable to exec prog", "");
    return true;
ret_err:
    return false;
}

const char* prog     = "\n%bind       hello       \"\\n Hello, World\""
                       "\n%entry      main                      ; ENTRY POINT"
                       "\n"
                       "\nsay_hello:                              ; GLOBAL 'say_hello'"
                       "\n"
                       "\nmain:"
                       "\n%scope"
                       "\n    SETR    ref([U1])   2            ; iteration count"
                       "\nsay_hello:                              ; LOCAL 'say_hello'"
                       "\n    PUSH   hello                        ; ptr to string start"
                       "\n    PUSH   len(hello)                   ; length of string"
                       "\n    INVOK  7                         ; print vmcall, expects above 2 arguments"
                       "\n    LOOP    ref([U1])   say_hello       ; CORRECTLY RESOLVE TO LOCAL 'say_hello'"
                       "\n%end"
                       "\nSHUTS";

Vm vm                = { 0 };
Sasm_Executable exec = { 0 };

bool virex_test(void)
{
    vm                  = (Vm) { 0 };
    exec                = (Sasm_Executable) { 0 };
    String_View sv_prog = STR(prog);
    printf("\nTest Program:");
    printf("\n-------------");
    printf("\n%s", sv_prog.data);
    printf("\n");
    sasm_assemble(&exec, sv_prog);
    printf("\nOutput:");
    printf("\n-------------");
    return virex_run(&vm, &exec, -1);     // TODO: cleanup not handled after exectuion
}

#endif
