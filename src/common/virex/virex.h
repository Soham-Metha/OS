/*
 * virex.h
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
#ifndef KERN_VIREX_1
#define KERN_VIREX_1

#define IMPL_KERN_SASM_1
#include "sasm.h"

#define CALL_NAME_CAPACITY 256

typedef struct Vm Vm;
typedef struct VmCalls VmCalls;

typedef VM_Error (*InternalVmCall)(Vm* vm);

struct VmCalls
{
    InternalVmCall VmCallI[INTERNAL_VMCALLS_CAPACITY];
    uint32 internalVmCallsDefined;
};

struct Vm {
    Memory mem;
    Program prog;
    CPU cpu;
    VmCalls vmCalls;
    Arena arena;
};

#define $memory ->mem.memory
#define $stack ->mem.stack

#define $inst ->prog.instructions
#define $inst_cnt ->prog.instruction_count

#define $stack_top ->cpu.registers.reg[REG_SP].u32
#define $reg ->cpu.registers.reg

#define $vm_call ->vmCalls.VmCallI

bool virex_run(Vm* vm, Sasm_Executable* exec, int lim);
bool virex_test(void);

#endif

#ifdef IMPL_KERN_VIREX_1
#undef IMPL_KERN_VIREX_1

bool loadInternalCallIntoVm(Vm* Vm, InternalVmCall call);
bool loadStandardCallsIntoVm(Vm* Vm);
bool loadProgramIntoVm(Vm* vm, Sasm_Executable* exec);

bool executeProgram(Vm* vm, int debug, int i);
VM_Error executeInst(Vm* vm);

VM_Error vmcall_write(Vm* vm);
VM_Error vmcall_alloc(Vm* vm);
VM_Error vmcall_free(Vm* vm);
VM_Error vmcall_print_f64(Vm* vm);
VM_Error vmcall_print_i64(Vm* vm);
VM_Error vmcall_print_u64(Vm* vm);
VM_Error vmcall_print_ptr(Vm* vm);
VM_Error vmcall_dump_memory(Vm* vm);

bool loadInternalCallIntoVm(Vm* vm, InternalVmCall call)
{
    try(vm->vmCalls.internalVmCallsDefined < INTERNAL_VMCALLS_CAPACITY, "VMCall cap exceeded!", "");
    vm->vmCalls.VmCallI[vm->vmCalls.internalVmCallsDefined++] = call;
    return true;
ret_err:
    return false;
}

bool loadProgramIntoVm(Vm* vm, Sasm_Executable* exec)
{
    memset(&vm->prog, 0, sizeof(vm->prog));
    // FILE* f       = openFile(filePath, "rb");

    Sasm_Metadata meta = exec->meta;

    // uint32 n      = fread(&meta, sizeof(meta), 1, f);
    // if (n < 1) {
    //     printf( "ERROR: Could not read meta data from file `%s`\n",
    //         filePath);
    //     exit(1);
    // }

    try(meta.magic == FILE_MAGIC,
        "ERROR: executable does not appear to be a valid vm executable. "
        "Unexpected magic %04X. Expected %04X.\n",
        meta.magic, FILE_MAGIC);

    try(meta.version == FILE_VERSION,
        "ERROR: unsupported version of vm file %d. Expected version %d.\n",
        meta.version, FILE_VERSION);

    try(meta.prog_size <= MAX_PROGRAM_CAPACITY,
        "ERROR: program section is too big. The file contains %" PRIu64 " program instruction. But the capacity is %" PRIu64 "\n",
        meta.prog_size, (u32)MAX_PROGRAM_CAPACITY);

    try(meta.mem_capacity <= MAX_MEMORY_CAPACITY,
        "ERROR: memory section is too big. The file wants %" PRIu64 " bytes. But the capacity is %" PRIu64 " bytes\n",
        meta.mem_capacity, (u32)MAX_MEMORY_CAPACITY);

    try(meta.mem_size <= meta.mem_capacity,
        "ERROR: memory size %" PRIu64 " is greater than declared memory capacity %" PRIu64 "\n",
        meta.mem_size, meta.mem_capacity);

    vm $reg[REG_IP].u32 = meta.entry;
    // vm->prog.instruction_count = fread(vm->prog.instructions, sizeof(vm->prog.instructions[0]), meta.prog_size, f);
    vm->prog            = exec->prog;
    try(vm->prog.instruction_count == meta.prog_size, "ERROR: read %" PRIu64 " program instructions, but expected %" PRIu64 "\n",
        vm->prog.instruction_count, meta.prog_size);

    for (DataEntry i = 0; i < meta.mem_size; i++) {
        vm->mem.memory[i] = exec->memory[i];
    }

    // n = fread(vm->mem.memory, sizeof(vm->mem.memory[0]), meta.mem_size, f);

    // if (n != meta.mem_size) {
    //     printf( "ERROR: %s: read %zd bytes of memory section, but expected %" PRIu64 " bytes.\n",
    //         filePath, n, meta.mem_size);
    //     exit(1);
    // }

    // closeFile(f, filePath);
    return true;
ret_err:
    return false;
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

static inline QuadWord stack_pop(Vm* vm)
{
    return vm $stack[--vm $stack_top];
}

static inline void stack_push(Vm* vm, QuadWord val)
{
    vm $stack[vm $stack_top++] = val;
}

VM_Error vmcall_write(Vm* vm)
{
    uint32 count    = stack_pop(vm).u32;
    MemoryAddr addr = stack_pop(vm).u32;

    if (addr >= MAX_MEMORY_CAPACITY) {
        return ERR_ILLEGAL_MEMORY_ACCESS;
    }

    if (addr + count < addr || addr + count >= MAX_MEMORY_CAPACITY) {
        return ERR_ILLEGAL_MEMORY_ACCESS;
    }

    for (uint32 i = 0; i < count; i += 1) {
        if (vm $memory [addr + i] == '\\') {
            i += 1;
            if (i >= count)
                return ERR_ILLEGAL_OPERAND;
            else if (vm $memory [addr + i] == 'n')
                printf("\n");
            else if (vm $memory [addr + i] == '_')
                printf("_");
            else
                return ERR_ILLEGAL_OPERAND;
        } else if (vm $memory [addr + i] == '_') {
            printf(" ");
        } else {
            printf("%c", vm $memory [addr + i]);
        }
    }

    return ERR_OK;
}

VM_Error vmcall_alloc(Vm* vm)
{
    (void)vm;
    // stack_push(vm, region_alloc(arena, stack_pop(vm)));

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
    printf(" %lf\n", stack_pop(vm).f32);
    return ERR_OK;
}

VM_Error vmcall_print_i64(Vm* vm)
{
    printf(" %" PRId64 "", stack_pop(vm).i32);
    return ERR_OK;
}

VM_Error vmcall_print_u64(Vm* vm)
{
    printf(" %" PRIu64 "", stack_pop(vm).u32);
    return ERR_OK;
}

VM_Error vmcall_print_ptr(Vm* vm)
{
    printf(" %p\n", stack_pop(vm).ptr);
    return ERR_OK;
}

VM_Error vmcall_dump_memory(Vm* vm)
{
    uint32 count    = stack_pop(vm).u32;
    MemoryAddr addr = stack_pop(vm).u32;

    if (addr >= MAX_MEMORY_CAPACITY) {
        return ERR_ILLEGAL_MEMORY_ACCESS;
    }

    if (addr + count < addr || addr + count >= MAX_MEMORY_CAPACITY) {
        return ERR_ILLEGAL_MEMORY_ACCESS;
    }

    for (uint32 i = 0; i < count; ++i) {
        printf(" %02X ", vm $memory [addr + i]);
        if (i % 16 == 15) {
            printf("\n ");
        }
    }
    printf("\n");

    return ERR_OK;
}

VM_Error vmcall_writeROM(Vm* vm)
{
    char* buffer    = stack_pop(vm).ptr;
    uint32 count    = stack_pop(vm).u32;
    MemoryAddr addr = stack_pop(vm).u32;

    if (addr >= MAX_MEMORY_CAPACITY) {
        return ERR_ILLEGAL_MEMORY_ACCESS;
    }

    if (addr + count < addr || addr + count >= MAX_MEMORY_CAPACITY) {
        return ERR_ILLEGAL_MEMORY_ACCESS;
    }

    memcpy(buffer, &vm $memory [addr], count);

    return ERR_OK;
}

bool executeProgram(Vm* vm, int debug, int lim)
{
    VM_Error error = executeInst(vm);

    if (lim == 0 || getFlag(META_HALT, &(vm->cpu))) {
        return true;
    }

    try(error == ERR_OK, "Error when executing inst! ecode: %d", error);

    return executeProgram(vm, debug, lim - 1);
ret_err:
    return false;
}

#define READ_OP(type, out)                             \
    {                                                  \
        if (vm $stack_top < 1) {                       \
            return ERR_STACK_UNDERFLOW;                \
        }                                              \
        const MemoryAddr addr = stack_pop(vm).u32;     \
        if (addr >= MAX_MEMORY_CAPACITY) {             \
            return ERR_ILLEGAL_MEMORY_ACCESS;          \
        }                                              \
        type tmp;                                      \
        memcpy(&tmp, &vm $memory[addr], sizeof(type)); \
        stack_push(vm, quadwordFrom##out(tmp));        \
    }

#define WRITE_OP(type, size)                              \
    {                                                     \
        if (vm $stack_top < 2) {                          \
            return ERR_STACK_UNDERFLOW;                   \
        }                                                 \
        const type value      = stack_pop(vm).u32;        \
        const MemoryAddr addr = stack_pop(vm).u32;        \
        if (addr >= MAX_MEMORY_CAPACITY - size) {         \
            return ERR_ILLEGAL_MEMORY_ACCESS;             \
        }                                                 \
        memcpy(&vm $memory[addr], &value, sizeof(value)); \
    }

#define BINARY_OP(in, out, op)                          \
    {                                                   \
        if (vm $stack_top < 2) {                        \
            return ERR_STACK_UNDERFLOW;                 \
        }                                               \
        in opr2      = stack_pop(vm).in;                \
        in opr1      = stack_pop(vm).in;                \
        QuadWord res = quadwordFrom##out(opr1 op opr2); \
        stack_push(vm, res);                            \
    }

#define STACK_CAST(src, dst, cast)                  \
    {                                               \
        src opr      = stack_pop(vm).src;           \
        QuadWord res = quadwordFrom##dst(cast opr); \
        stack_push(vm, res);                        \
    }

VM_Error executeInst(Vm* vm)
{
    if (vm $reg[REG_IP].u32 >= vm $inst_cnt) {
        printf("error tring to access instruction at '%d', but there are only '%d' instructions", vm $reg[REG_IP].u32, vm $inst_cnt);
        return ERR_ILLEGAL_INST_ACCESS;
    }

    Instruction inst = vm $inst[vm $reg[REG_IP].u32];
    // register value dereferencing
    // 0         <= val < reg count   => val == register id
    // reg_count <= val < 2*reg_count => val == value of register with ID (val - reg_count)
    if (inst.opr1IsReg && inst.operand.u32 > REG_COUNT) {
        inst.operand.u32 = vm $reg[inst.operand.u32 % REG_COUNT].u32;
    }
    if (inst.opr2IsReg && inst.operand2.u32 > REG_COUNT) {
        inst.operand2.u32 = vm $reg[inst.operand2.u32 % REG_COUNT].u32;
    }

    // printf("\nenter : %d %s", inst.type, OpcodeDetailsLUT[inst.type].name);
    switch (inst.type) {
    // =============================== Misc ==============================
    case INST_DONOP:
    break; case INST_SHUTS: setFlag(META_HALT, &vm->cpu, 1);
    // ========================= env interaction =========================
    break; case INST_INVOK:
        if (inst.operand.u32 > vm->vmCalls.internalVmCallsDefined) return ERR_ILLEGAL_OPERAND;
        if (!vm $vm_call[inst.operand.u32])                        return ERR_NULL_CALL;
        const VM_Error err = vm $vm_call[inst.operand.u32](vm);
        if (err != ERR_OK) return err;
    // ============================ Registers ============================
    break; case INST_SETR:
        if (inst.operand.u32 < REG_U0 || inst.operand.u32 > REG_U9) return ERR_ILLEGAL_OPERAND;
        vm $reg[inst.operand.u32].u32 = inst.operand2.u32;
    break; case INST_COPY:
        if (inst.operand.u32 < REG_U0 || inst.operand.u32 > REG_U9) return ERR_ILLEGAL_OPERAND;
        vm $reg[inst.operand.u32].u32 = vm $reg[inst.operand2.u32].u32;
    break; case INST_SPOPR:
        if (vm $stack_top < 1) return ERR_STACK_UNDERFLOW;
        if (inst.operand.u32 < REG_U0 || inst.operand.u32 > REG_U9) return ERR_ILLEGAL_OPERAND;
        vm $reg[inst.operand.u32] = stack_pop(vm);
    // =============================== Stack ===============================
    break; case INST_PUSH:
        if (vm $stack_top >= STACK_CAPACITY) return ERR_STACK_OVERFLOW;
        stack_push(vm, inst.operand);
    break; case INST_SPOP:
        if (vm $stack_top < 1) return ERR_STACK_UNDERFLOW;
        stack_pop(vm);
    break; case INST_DUPS:
        if (vm $stack_top >= STACK_CAPACITY)   return ERR_STACK_OVERFLOW;
        if (vm $stack_top <= inst.operand.u32) return ERR_STACK_UNDERFLOW;
        stack_push(vm, vm $stack[vm $stack_top - 1 - inst.operand.u32]);
    break; case INST_SWAP:
        if (inst.operand.u32 >= vm $stack_top) return ERR_STACK_UNDERFLOW;
        const u32 a  = vm $stack_top - 1;
        const u32 b  = vm $stack_top - 1 - inst.operand.u32;
        QuadWord tmp = vm $stack[a];
        vm $stack[a] = vm $stack[b];
        vm $stack[b] = tmp;
    // ==================== Branching (Unconditional) ====================
    break; case INST_JMPU:
        vm $reg[REG_IP].u32 = inst.operand.u32;
    return ERR_OK; case INST_CALL:
        if (vm $stack_top >= STACK_CAPACITY) return ERR_STACK_OVERFLOW;
        stack_push(vm, quadwordFromU64(vm $reg[REG_IP].u32 + 1));
        vm $reg[REG_IP].u32 = inst.operand.u32;
    return ERR_OK; case INST_RET:
        if (vm $stack_top < 1) return ERR_STACK_UNDERFLOW;
        vm $reg[REG_IP].u32 = stack_pop(vm).u32;
    // ===================== Branching (Conditional) =====================
    return ERR_OK; case INST_JMPC:
        if (vm $stack_top < 1) return ERR_STACK_UNDERFLOW;
        if (stack_pop(vm).u32 > 0) {
            vm $reg[REG_IP].u32 = inst.operand.u32;
            return ERR_OK;
        }
    break; case INST_LOOP:
        vm $reg[inst.operand.u32].u32 -= 1;
        if (vm $reg[inst.operand.u32].u32 > 0) {
            vm $reg[REG_IP].u32 = inst.operand2.u32;
            return ERR_OK;
        }
    // ========================= Logical (Unary) =========================
    break; case INST_NOT:
        if (vm $stack_top < 1) return ERR_STACK_UNDERFLOW;
        {
            u32 val = stack_pop(vm).u32;
            val     = !val;
            stack_push(vm, quadwordFromU64(val));
        }
    // ========================= Binary (Unary) ==========================
    break; case INST_NOTB:
        if (vm $stack_top < 1) return ERR_STACK_UNDERFLOW;
        {
            u32 val = stack_pop(vm).u32;
            val     = ~val;
            stack_push(vm, quadwordFromU64(val));
        }
    // ======================== Logical (Binary) =========================
    break; case INST_EQI: BINARY_OP(i32, U64, ==);
    break; case INST_EQU: BINARY_OP(u32, U64, ==);
    break; case INST_EQF: BINARY_OP(f32, U64, ==);
    break; case INST_GEI: BINARY_OP(i32, U64, >=);
    break; case INST_GEU: BINARY_OP(u32, U64, >=);
    break; case INST_GEF: BINARY_OP(f32, U64, >=);
    break; case INST_GTI: BINARY_OP(i32, U64, >);
    break; case INST_GTU: BINARY_OP(u32, U64, >);
    break; case INST_GTF: BINARY_OP(f32, U64, >);
    break; case INST_LEI: BINARY_OP(i32, U64, <=);
    break; case INST_LEU: BINARY_OP(u32, U64, <=);
    break; case INST_LEF: BINARY_OP(f32, U64, <=);
    break; case INST_LTI: BINARY_OP(i32, U64, <);
    break; case INST_LTU: BINARY_OP(u32, U64, <);
    break; case INST_LTF: BINARY_OP(f32, U64, <);
    break; case INST_NEI: BINARY_OP(i32, U64, !=);
    break; case INST_NEU: BINARY_OP(u32, U64, !=);
    break; case INST_NEF: BINARY_OP(f32, U64, !=);
    // ========================= Binary (Binary) =========================
    break; case INST_ANDB: BINARY_OP(u32, U64, &);
    break; case INST_ORB: BINARY_OP(u32, U64, |);
    break; case INST_XOR: BINARY_OP(u32, U64, ^);
    break; case INST_SHR: BINARY_OP(u32, U64, >>);
    break; case INST_SHL: BINARY_OP(u32, U64, <<);
    // ============================ Typecast =============================
    break; case INST_I2F: STACK_CAST(i32, F64, (f32));
    break; case INST_U2F: STACK_CAST(u32, F64, (f32));
    break; case INST_F2I: STACK_CAST(f32, I64, (f32));
    break; case INST_F2U: STACK_CAST(f32, U64, (u32)(i32));
    // =============================== I/O ===============================
    break; case INST_READ1U: READ_OP(Byte, U64);
    break; case INST_READ1I: READ_OP(int8, I64);
    break; case INST_WRITE1: WRITE_OP(Byte, 0);
    break; case INST_READ2U: READ_OP(Word, U64);
    break; case INST_READ2I: READ_OP(int16, I64);
    break; case INST_READ4U: READ_OP(DoubleWord, U64);
    break; case INST_WRITE2: WRITE_OP(Word, 1);
    break; case INST_READ4I: READ_OP(int32, I64);
    break; case INST_WRITE4: WRITE_OP(DoubleWord, 3);
    break; case INST_READ8U: READ_OP(u32, U64);
    break; case INST_READ8I: READ_OP(int64, I64);
    break; case INST_WRITE8: WRITE_OP(u32, 7);
    // =========================== Arithmetic ============================
    break; case INST_ADDI: BINARY_OP(i32, I64, +);
    break; case INST_ADDU: BINARY_OP(u32, U64, +);
    break; case INST_ADDF: BINARY_OP(f32, F64, +);
    break; case INST_SUBI: BINARY_OP(i32, I64, -);
    break; case INST_SUBU: BINARY_OP(u32, U64, -);
    break; case INST_SUBF: BINARY_OP(f32, F64, -);
    break; case INST_MULI: BINARY_OP(i32, I64, *);
    break; case INST_MULU: BINARY_OP(u32, U64, *);
    break; case INST_MULF: BINARY_OP(f32, F64, *);
    break; case INST_DIVI:
        if (vm $stack[vm $stack_top - 1].i32 == 0) return ERR_DIV_BY_ZERO;
        BINARY_OP(i32, I64, /);
    break; case INST_DIVU:
        if (vm $stack[vm $stack_top - 1].u32 == 0) return ERR_DIV_BY_ZERO;
        BINARY_OP(u32, U64, /);
    break; case INST_DIVF:
        if (vm $stack[vm $stack_top - 1].f32 == 0.0) return ERR_DIV_BY_ZERO;
        BINARY_OP(f32, F64, /);
    break; case INST_MODI:
        if (vm $stack[vm $stack_top - 1].i32 == 0) return ERR_DIV_BY_ZERO;
        BINARY_OP(i32, I64, %);
    break; case INST_MODU:
        if (vm $stack[vm $stack_top - 1].u32 == 0) return ERR_DIV_BY_ZERO;
        BINARY_OP(u32, U64, %);
    break; case NUMBER_OF_INSTS:
    default:
        return ERR_ILLEGAL_INST;
    }

    vm $reg[REG_IP].u32++;
    return ERR_OK;
}

bool virex_run(Vm* vm, Sasm_Executable* exec, int lim)
{
    try(loadStandardCallsIntoVm(vm), "Unable to load vm calls", "");
    try(loadProgramIntoVm(vm, exec), "Unable to load program", "");
    try(executeProgram(vm, 0, lim), "Unable to exec prog", "");
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
