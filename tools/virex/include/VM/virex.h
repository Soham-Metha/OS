/**
 * @file virex.h
 * @brief This file contains the declarations of functions and structures related to the virtual machine.
 *
 * The virtual machine represents the execution environment for the Game Boy program.
 *
 * @author Soham Metha
 * @date January 2025
 */

#pragma once

#define IMPL_SASM_1
#include "../../../../common/memmanager.h"
#include "sasm_assembler.h"
#include "univ_defs.h"
#include "univ_errors.h"

#define CALL_NAME_CAPACITY 256

typedef Error (*InternalVmCall)(CPU* cpu, Memory* mem, Arena* arena);

typedef struct
{
    char name[CALL_NAME_CAPACITY];
} ExternalVmCall;

typedef struct
{
    InternalVmCall VmCallI[INTERNAL_VMCALLS_CAPACITY];
    size_t internalVmCallsDefined;

    ExternalVmCall VmCallE[EXTERNAL_VMCALLS_CAPACITY];
    size_t externalVmCallsDefined;
} VmCalls;

/**
 * @struct Vm
 * Represents the virtual machine instance.
 */

typedef struct {
    Memory mem;   /**< The memory component of the virtual machine. */
    Program prog; /**< The program component of the virtual machine. */
    CPU cpu;      /**< The CPU component of the virtual machine. */
    VmCalls vmCalls;
    Arena arena;
} Vm;

#define $memory ->mem.memory
#define $stack ->mem.stack

#define $inst ->prog.instructions
#define $inst_cnt ->prog.instruction_count

#define $stack_top ->cpu.registers.reg[REG_SP].u64
#define $reg ->cpu.registers.reg

#define $vm_call ->vmCalls.VmCallI

/**
 * Executes the program loaded in the virtual machine.
 *
 * @param vm The virtual machine instance.
 * @param debug The debug level (0, 1, or 2).
 * @param i The current execution count.
 */
void executeProgram(Vm* vm, int debug, int i);

/**
 * @brief Executes an instruction in the virtual machine.
 *
 * @param prog The program containing the instructions.
 * @param mem The memory of the virtual machine.
 * @param cpu The CPU of the virtual machine.
 * @return An error code indicating the success or failure of the execution.
 */
Error executeInst(Vm* vm);

void loadInternalCallIntoVm(Vm* Vm, InternalVmCall call);
void loadStandardCallsIntoVm(Vm* Vm);
/**
 * Loads the program from the specified input file into the virtual machine.
 *
 * @param vm The virtual machine instance.
 * @param inputFile The input binary file containing the program bytecode.
 */
void loadProgramIntoVm(Vm* vm, const char* inputFile);

Error vmcall_write(CPU* cpu, Memory* mem, Arena* arena);
Error vmcall_alloc(CPU* cpu, Memory* mem, Arena* arena);
Error vmcall_free(CPU* cpu, Memory* mem, Arena* arena);
Error vmcall_print_f64(CPU* cpu, Memory* mem, Arena* arena);
Error vmcall_print_i64(CPU* cpu, Memory* mem, Arena* arena);
Error vmcall_print_u64(CPU* cpu, Memory* mem, Arena* arena);
Error vmcall_print_ptr(CPU* cpu, Memory* mem, Arena* arena);
Error vmcall_dump_memory(CPU* cpu, Memory* mem, Arena* arena);

#define LERP(START, END, T) (START * T + END * (1 - T))

enum WindowID {
    OUTPUT,
    INPUT,
    DETAILS,
    MEMORY,
    PROGRAM,
    NAME,
    STACK,
    MAX_WINDOW_COUNT
};

enum Inputs {
    EXEC_SM,
    ASSEMBLE_EXEC_SASM,
    CUSTOM_CMD,
    ASSEMBLE_SASM,
    DISASSEMBLE_SM,
    COMPILE_ORIN,
    EXIT_VM,
    MAX_INPUTS
};

#ifdef IMPL_VIREX_1
#undef IMPL_VIREX_1

void loadInternalCallIntoVm(Vm* vm, InternalVmCall call)
{
    assert(vm->vmCalls.internalVmCallsDefined < INTERNAL_VMCALLS_CAPACITY);
    vm->vmCalls.VmCallI[vm->vmCalls.internalVmCallsDefined++] = call;
}

void loadProgramIntoVm(Vm* vm, const char* filePath)
{
    memset(&vm->prog, 0, sizeof(vm->prog));
    FILE* f       = openFile(filePath, "rb");

    Metadata meta = { 0 };

    size_t n      = fread(&meta, sizeof(meta), 1, f);
    if (n < 1) {
        fprintf(stderr, "ERROR: Could not read meta data from file `%s`\n",
            filePath);
        exit(1);
    }

    if (meta.magic != FILE_MAGIC) {
        fprintf(stderr,
            "ERROR: %s does not appear to be a valid vm file. "
            "Unexpected magic %04X. Expected %04X.\n",
            filePath, meta.magic, FILE_MAGIC);
        exit(1);
    }

    if (meta.version != FILE_VERSION) {
        fprintf(stderr,
            "ERROR: %s: unsupported version of vm file %d. Expected version %d.\n",
            filePath, meta.version, FILE_VERSION);
        exit(1);
    }

    if (meta.programSize > PROGRAM_CAPACITY) {
        fprintf(stderr,
            "ERROR: %s: program section is too big. The file contains %" PRIu64 " program instruction. But the capacity is %" PRIu64 "\n",
            filePath, meta.programSize, (u64)PROGRAM_CAPACITY);
        exit(1);
    }

    if (meta.memoryCapacity > MEMORY_CAPACITY) {
        fprintf(stderr,
            "ERROR: %s: memory section is too big. The file wants %" PRIu64 " bytes. But the capacity is %" PRIu64 " bytes\n",
            filePath, meta.memoryCapacity, (u64)MEMORY_CAPACITY);
        exit(1);
    }

    if (meta.memorySize > meta.memoryCapacity) {
        fprintf(stderr,
            "ERROR: %s: memory size %" PRIu64 " is greater than declared memory capacity %" PRIu64 "\n",
            filePath, meta.memorySize, meta.memoryCapacity);
        exit(1);
    }

    if (meta.externalsSize > EXTERNAL_VMCALLS_CAPACITY) {
        fprintf(stderr,
            "ERROR: %s: external names section is too big. The file contains %" PRIu64 " external names. But the capacity is %" PRIu64 " external names\n",
            filePath, meta.externalsSize, (u64)EXTERNAL_VMCALLS_CAPACITY);
        exit(1);
    }

    vm $reg[REG_NX].u64        = meta.entry;
    vm->prog.instruction_count = fread(vm->prog.instructions, sizeof(vm->prog.instructions[0]), meta.programSize, f);

    if (vm->prog.instruction_count != meta.programSize) {
        fprintf(stderr, "ERROR: %s: read %" PRIu64 " program instructions, but expected %" PRIu64 "\n",
            filePath, vm->prog.instruction_count, meta.programSize);
        exit(1);
    }

    n = fread(vm->mem.memory, sizeof(vm->mem.memory[0]), meta.memorySize, f);

    if (n != meta.memorySize) {
        fprintf(stderr, "ERROR: %s: read %zd bytes of memory section, but expected %" PRIu64 " bytes.\n",
            filePath, n, meta.memorySize);
        exit(1);
    }

    vm->vmCalls.externalVmCallsDefined = fread(vm->vmCalls.VmCallE, sizeof(vm->vmCalls.VmCallE[0]), meta.externalsSize, f);
    if (vm->vmCalls.externalVmCallsDefined != meta.externalsSize) {
        fprintf(stderr, "ERROR: %s: read %zu external names, but expected %" PRIu64 "\n",
            filePath, vm->vmCalls.externalVmCallsDefined, meta.externalsSize);
        exit(1);
    }

    closeFile(f, filePath);
}

void loadStandardCallsIntoVm(Vm* vm)
{
    loadInternalCallIntoVm(vm, vmcall_alloc);           // 0
    loadInternalCallIntoVm(vm, vmcall_free);            // 1
    loadInternalCallIntoVm(vm, vmcall_print_f64);       // 2
    loadInternalCallIntoVm(vm, vmcall_print_i64);       // 3
    loadInternalCallIntoVm(vm, vmcall_print_u64);       // 4
    loadInternalCallIntoVm(vm, vmcall_print_ptr);       // 5
    loadInternalCallIntoVm(vm, vmcall_dump_memory);     // 6
    loadInternalCallIntoVm(vm, vmcall_write);           // 7
}

#pragma GCC diagnostic ignored "-Wunused-parameter"
Error vmcall_write(CPU* cpu, Memory* mem, Arena* arena)
{
    MemoryAddr addr = cpu->registers.L0.u64;
    uint64 count  = cpu->registers.QT.u64;

    if (addr >= MEMORY_CAPACITY) {
        return ERR_ILLEGAL_MEMORY_ACCESS;
    }

    if (addr + count < addr || addr + count >= MEMORY_CAPACITY) {
        return ERR_ILLEGAL_MEMORY_ACCESS;
    }

    for (uint64 i = 0; i < count; i += 1) {
        if (mem->memory[addr + i] == '\\') {
            i += 1;
            if (i >= count)
                return ERR_ILLEGAL_OPERAND;
            else if (mem->memory[addr + i] == 'n')
                printf("\n");
            else if (mem->memory[addr + i] == '_')
                printf("_");
            else
                return ERR_ILLEGAL_OPERAND;
        } else if (mem->memory[addr + i] == '_') {
            printf(" ");
        } else {
            printf("%c", mem->memory[addr + i]);
        }
    }

    return ERR_OK;
}

Error vmcall_alloc(CPU* cpu, Memory* mem, Arena* arena)
{

    cpu->registers.RF.ptr = region_alloc(arena, cpu->registers.QT.u64);

    return ERR_OK;
}

Error vmcall_free(CPU* cpu, Memory* mem, Arena* arena)
{
    // clearGarbage(region);

    return ERR_OK;
}

Error vmcall_print_f64(CPU* cpu, Memory* mem, Arena* arena)
{
    printf(" %lf\n", cpu->registers.L1.f64);
    return ERR_OK;
}

Error vmcall_print_i64(CPU* cpu, Memory* mem, Arena* arena)
{
    printf(" %" PRId64 "", cpu->registers.L2.i64);
    return ERR_OK;
}

Error vmcall_print_u64(CPU* cpu, Memory* mem, Arena* arena)
{
    printf(" %" PRIu64 "", cpu->registers.L3.u64);
    return ERR_OK;
}

Error vmcall_print_ptr(CPU* cpu, Memory* mem, Arena* arena)
{
    printf(" %p\n", cpu->registers.RF.ptr);
    return ERR_OK;
}

Error vmcall_dump_memory(CPU* cpu, Memory* mem, Arena* arena)
{
    MemoryAddr addr = cpu->registers.L0.u64;
    uint64 count  = cpu->registers.QT.u64;

    if (addr >= MEMORY_CAPACITY) {
        return ERR_ILLEGAL_MEMORY_ACCESS;
    }

    if (addr + count < addr || addr + count >= MEMORY_CAPACITY) {
        return ERR_ILLEGAL_MEMORY_ACCESS;
    }

    for (uint64 i = 0; i < count; ++i) {
        printf(" %02X ", mem->memory[addr + i]);
        if (i % 16 == 15) {
            printf("\n ");
        }
    }
    printf("\n");

    return ERR_OK;
}

Error vmcall_writeROM(CPU* cpu, Memory* mem, Arena* arena)
{
    MemoryAddr addr = cpu->registers.L0.u64;
    uint64 count  = cpu->registers.QT.u64;

    char* buffer    = cpu->registers.RF.ptr;

    if (addr >= MEMORY_CAPACITY) {
        return ERR_ILLEGAL_MEMORY_ACCESS;
    }

    if (addr + count < addr || addr + count >= MEMORY_CAPACITY) {
        return ERR_ILLEGAL_MEMORY_ACCESS;
    }

    memcpy(buffer, &mem->memory[addr], count);

    return ERR_OK;
}

static inline QuadWord stack_pop(Vm* vm)
{
    return vm $stack[--vm $stack_top];
}

static inline void stack_push(Vm* vm, QuadWord val)
{
    vm $stack[vm $stack_top++] = val;
}

void executeProgram(Vm* vm, int debug, int lim)
{
    Error error = executeInst(vm);

    if (lim == 0 || getFlag(META_HALT, &(vm->cpu))) {
        return;
    }

    if (error != ERR_OK)
        executionErrorWithExit(&error);

    executeProgram(vm, debug, lim - 1);
}

#define READ_OP(type, out)                             \
    {                                                  \
        if (vm $stack_top < 1) {                       \
            return ERR_STACK_UNDERFLOW;                \
        }                                              \
        const MemoryAddr addr = stack_pop(vm).u64;     \
        if (addr >= MEMORY_CAPACITY) {                 \
            return ERR_ILLEGAL_MEMORY_ACCESS;          \
        }                                              \
        type tmp;                                      \
        memcpy(&tmp, &vm $memory[addr], sizeof(type)); \
        stack_push(vm, quadwordFrom##out(tmp));        \
        vm $reg[REG_NX].u64++;                         \
    }

#define WRITE_OP(type, size)                              \
    {                                                     \
        if (vm $stack_top < 2) {                          \
            return ERR_STACK_UNDERFLOW;                   \
        }                                                 \
        const type value      = stack_pop(vm).u64;        \
        const MemoryAddr addr = stack_pop(vm).u64;        \
        if (addr >= MEMORY_CAPACITY - size) {             \
            return ERR_ILLEGAL_MEMORY_ACCESS;             \
        }                                                 \
        memcpy(&vm $memory[addr], &value, sizeof(value)); \
        vm $reg[REG_NX].u64++;                            \
    }

#define ARITH_OP(reg, in, op)  \
    {                          \
        reg = reg op in;       \
        vm $reg[REG_NX].u64++; \
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
        vm $reg[REG_NX].u64++;                          \
    }

#define CAST_OP(r1, r2, src, dst, cast) \
    {                                   \
        r1.dst = cast r2.src;           \
        vm $reg[REG_NX].u64++;          \
    }

#define STACK_CAST(src, dst, cast)                  \
    {                                               \
        src opr      = stack_pop(vm).src;           \
        QuadWord res = quadwordFrom##dst(cast opr); \
        stack_push(vm, res);                        \
        vm $reg[REG_NX].u64++;                      \
    }

Error executeInst(Vm* vm)
{
    if (vm $reg[REG_NX].u64 >= vm $inst_cnt) {
        printf("error tring to access instruction at '%" PRIu64 "', but there are only '%" PRIu64 "' instructions", vm $reg[REG_NX].u64, vm $inst_cnt);
        return ERR_ILLEGAL_INST_ACCESS;
    }

    Instruction inst = vm $inst[vm $reg[REG_NX].u64];
    // register value dereferencing
    if (inst.opr1IsReg && inst.operand.u64 > REG_COUNT) {
        inst.operand.u64 = vm $reg[inst.operand.u64 % REG_COUNT].u64;
    }
    if (inst.opr2IsReg && inst.operand2.u64 > REG_COUNT) {
        inst.operand2.u64 = vm $reg[inst.operand2.u64 % REG_COUNT].u64;
    }

    // printf("\nenter : %d %s", inst.type, OpcodeDetailsLUT[inst.type].name);
    switch (inst.type) {

    case INST_DONOP:
        vm $reg[REG_NX].u64++;
        break;

    case INST_RETVL:
        // no implementation yet!!
        break;

    case INST_SHUTS:
        setFlag(META_HALT, &vm->cpu, 1);
        break;

    case INST_INVOK:
        if (inst.operand.u64 > vm->vmCalls.internalVmCallsDefined)
            return ERR_ILLEGAL_OPERAND;

        if (!vm $vm_call[inst.operand.u64])
            return ERR_NULL_CALL;

        const Error err = vm $vm_call[inst.operand.u64](&vm->cpu, &vm->mem, &vm->arena);
        if (err != ERR_OK)
            return err;

        vm $reg[REG_NX].u64++;
        break;

    case INST_GETR: /* fallthrough */
    case INST_PUSHR:
        if (vm $stack_top >= STACK_CAPACITY)
            return ERR_STACK_OVERFLOW;

        stack_push(vm, vm $reg[inst.operand.u64]);
        vm $reg[REG_NX].u64++;
        break;

    case INST_SETR:
        vm $reg[inst.operand2.u64].u64 = inst.operand.u64;
        vm $reg[REG_NX].u64++;
        break;

    case INST_SPOPR:
        if (vm $stack_top < 1)
            return ERR_STACK_UNDERFLOW;

        vm $reg[inst.operand.u64] = stack_pop(vm);
        vm $reg[REG_NX].u64++;
        break;

    case INST_PUSH:
        if (vm $stack_top >= STACK_CAPACITY) {
            return ERR_STACK_OVERFLOW;
        }
        stack_push(vm, inst.operand);
        vm $reg[REG_NX].u64++;
        break;

    case INST_SPOP:
        if (vm $stack_top < 1) {
            return ERR_STACK_UNDERFLOW;
        }
        vm $reg[REG_QT].u64 = stack_pop(vm).u64;
        vm $reg[REG_NX].u64++;
        break;

    case INST_COPY:
        vm $reg[inst.operand.u64].u64 = vm $reg[inst.operand2.u64].u64;
        vm $reg[REG_NX].u64++;
        break;

    case INST_DUPS:
        if (vm $stack_top >= STACK_CAPACITY) {
            return ERR_STACK_OVERFLOW;
        }

        if (vm $stack_top <= inst.operand.u64) {
            return ERR_STACK_UNDERFLOW;
        }

        stack_push(vm, vm $stack[vm $stack_top - 1 - inst.operand.u64]);
        vm $reg[REG_NX].u64++;
        break;

    case INST_SWAP:
        if (inst.operand.u64 >= vm $stack_top) {
            return ERR_STACK_UNDERFLOW;
        }

        const u64 a  = vm $stack_top - 1;
        const u64 b  = vm $stack_top - 1 - inst.operand.u64;

        QuadWord tmp = vm $stack[a];
        vm $stack[a] = vm $stack[b];
        vm $stack[b] = tmp;
        vm $reg[REG_NX].u64++;
        break;

    case INST_JMPU:
        vm $reg[REG_NX].u64 = inst.operand.u64;
        break;

    case INST_JMPC:
        if (vm $stack_top < 1)
            return ERR_STACK_UNDERFLOW;

        if (stack_pop(vm).u64 > 0)
            vm $reg[REG_NX].u64 = inst.operand.u64;
        else
            vm $reg[REG_NX].u64++;

        break;

    case INST_LOOP:
        if (vm $reg[inst.operand2.u64].u64 > 0) {
            vm $reg[REG_NX].u64 = inst.operand.u64;
        } else {
            vm $reg[REG_NX].u64++;
        }
        vm $reg[inst.operand2.u64].u64 -= 1;
        break;

    case INST_CALL:
        if (vm $stack_top >= STACK_CAPACITY)
            return ERR_STACK_OVERFLOW;

        stack_push(vm, quadwordFromU64(vm $reg[REG_NX].u64 + 1));
        vm $reg[REG_NX].u64 = inst.operand.u64;
        break;

    case INST_RET:
        if (vm $stack_top < 1) {
            return ERR_STACK_UNDERFLOW;
        }

        vm $reg[REG_NX].u64 = stack_pop(vm).u64;
        break;

    case INST_NOT:
        if (vm $stack_top < 1) {
            return ERR_STACK_UNDERFLOW;
        }
        {
            u64 val = stack_pop(vm).u64;
            val     = !val;
            stack_push(vm, quadwordFromU64(val));
        }
        vm $reg[REG_NX].u64++;
        break;

    case INST_NOTB:
        if (vm $stack_top < 1) {
            return ERR_STACK_UNDERFLOW;
        }

        {
            u64 val = stack_pop(vm).u64;
            val     = ~val;
            stack_push(vm, quadwordFromU64(val));
        }
        vm $reg[REG_NX].u64++;
        break;

    case INST_ADDI:
        // ARITH_OP(vm $reg[REG_L2].i64, inst.operand.i64, +);
        BINARY_OP(i64, I64, +);
        break;

    case INST_SUBI:
        // ARITH_OP(vm $reg[REG_L2].i64, inst.operand.i64, -);
        BINARY_OP(i64, I64, -);
        break;

    case INST_MULI:
        // ARITH_OP(vm $reg[REG_L2].i64, inst.operand.i64, *);
        BINARY_OP(i64, I64, *);
        break;

    case INST_DIVI:
        if (vm $stack[vm $stack_top - 1].i64 == 0)
            return ERR_DIV_BY_ZERO;
        // ARITH_OP(vm $reg[REG_L2].i64, inst.operand.i64, /);
        BINARY_OP(i64, I64, /);
        break;

    case INST_MODI:
        if (inst.operand.i64 == 0)
            return ERR_DIV_BY_ZERO;
        // ARITH_OP(vm $reg[REG_L2].i64, inst.operand.i64, %);
        BINARY_OP(i64, I64, %);
        break;

    case INST_ADDU:
        // ARITH_OP(vm $reg[REG_L3].u64, inst.operand.u64, +);
        BINARY_OP(u64, U64, +);
        break;

    case INST_SUBU:
        // ARITH_OP(vm $reg[REG_L3].u64, inst.operand.u64, -);
        BINARY_OP(u64, U64, -);
        break;

    case INST_MULU:
        // ARITH_OP(vm $reg[REG_L3].u64, inst.operand.u64, +);
        BINARY_OP(u64, U64, *);
        break;

    case INST_DIVU:
        if (inst.operand.u64 == 0)
            return ERR_DIV_BY_ZERO;
        // ARITH_OP(vm $reg[REG_L3].u64, inst.operand.u64, /);
        BINARY_OP(u64, U64, /);
        break;

    case INST_MODU:
        if (inst.operand.u64 == 0)
            return ERR_DIV_BY_ZERO;
        // ARITH_OP(vm $reg[REG_L3].u64, inst.operand.u64, %);
        BINARY_OP(u64, U64, %);
        break;

    case INST_ADDF:
        // ARITH_OP(vm $reg[REG_L1].f64, inst.operand.f64, +);
        BINARY_OP(f64, F64, +);
        break;

    case INST_SUBF:
        // ARITH_OP(vm $reg[REG_L1].f64, inst.operand.f64, -);
        BINARY_OP(f64, F64, -);
        break;

    case INST_MULF:
        // ARITH_OP(vm $reg[REG_L1].f64, inst.operand.f64, *);
        BINARY_OP(f64, F64, *);
        break;

    case INST_DIVF:
        if (inst.operand.f64 == 0.0)
            return ERR_DIV_BY_ZERO;
        // ARITH_OP(vm $reg[REG_L1].f64, inst.operand.f64, /);
        BINARY_OP(f64, F64, /);
        break;

    case INST_ANDB:
        BINARY_OP(u64, U64, &);
        break;

    case INST_EQI:
        BINARY_OP(i64, U64, ==);
        break;

    case INST_GEI:
        BINARY_OP(i64, U64, >=);
        break;

    case INST_GTI:
        BINARY_OP(i64, U64, >);
        break;

    case INST_LEI:
        BINARY_OP(i64, U64, <=);
        break;

    case INST_LTI:
        BINARY_OP(i64, U64, <);
        break;

    case INST_NEI:
        BINARY_OP(i64, U64, !=);
        break;

    case INST_EQU:
        BINARY_OP(u64, U64, ==);
        break;

    case INST_GEU:
        BINARY_OP(u64, U64, >=);
        break;

    case INST_GTU:
        BINARY_OP(u64, U64, >);
        break;

    case INST_LEU:
        BINARY_OP(u64, U64, <=);
        break;

    case INST_LTU:
        BINARY_OP(u64, U64, <);
        break;

    case INST_NEU:
        BINARY_OP(u64, U64, !=);
        break;

    case INST_EQF:
        BINARY_OP(f64, U64, ==);
        break;

    case INST_GEF:
        BINARY_OP(f64, U64, >=);
        break;

    case INST_GTF:
        BINARY_OP(f64, U64, >);
        break;

    case INST_LEF:
        BINARY_OP(f64, U64, <=);
        break;

    case INST_LTF:
        BINARY_OP(f64, U64, <);
        break;

    case INST_NEF:
        BINARY_OP(f64, U64, !=);
        break;

    case INST_ORB:
        BINARY_OP(u64, U64, |);
        break;

    case INST_XOR:
        BINARY_OP(u64, U64, ^);
        break;

    case INST_SHR:
        BINARY_OP(u64, U64, >>);
        break;

    case INST_SHL:
        BINARY_OP(u64, U64, <<);
        break;

    case INST_I2F:
        // CAST_OP(vm $reg[REG_L1], vm $reg[REG_L2], i64, f64, (f64));
        STACK_CAST(i64, F64, (f64));
        break;

    case INST_U2F:
        // CAST_OP(vm $reg[REG_L1], vm $reg[REG_L3], u64, f64, (f64));
        STACK_CAST(u64, F64, (f64));
        break;

    case INST_F2I:
        // CAST_OP(vm $reg[REG_L2], vm $reg[REG_L1], f64, i64, (i64));
        STACK_CAST(f64, I64, (f64));
        break;

    case INST_F2U:
        // CAST_OP(vm $reg[REG_L3], vm $reg[REG_L1], f64, u64, (u64)(i64));
        STACK_CAST(f64, U64, (u64)(i64));
        break;

    case INST_READ1U:
        READ_OP(Byte, U64);
        break;

    case INST_READ2U:
        READ_OP(Word, U64);
        break;

    case INST_READ4U:
        READ_OP(DoubleWord, U64);
        break;

    case INST_READ8U:
        READ_OP(u64, U64);
        break;

    case INST_READ1I:
        READ_OP(int8, I64);
        break;

    case INST_READ2I:
        READ_OP(int16, I64);
        break;

    case INST_READ4I:
        READ_OP(int32, I64);
        break;

    case INST_READ8I:
        READ_OP(int64, I64);
        break;

    case INST_WRITE1:
        WRITE_OP(Byte, 0);
        break;

    case INST_WRITE2:
        WRITE_OP(Word, 1);
        break;

    case INST_WRITE4:
        WRITE_OP(DoubleWord, 3);
        break;

    case INST_WRITE8:
        WRITE_OP(u64, 7);
        break;

    case NUMBER_OF_INSTS:
    default:
        return ERR_ILLEGAL_INST;
    }

    return ERR_OK;
}

#endif