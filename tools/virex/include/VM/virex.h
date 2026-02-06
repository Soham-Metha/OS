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
#include "sasm_assembler.h"
#include "univ_defs.h"
#include "univ_errors.h"
#include "univ_malloc.h"

#define CALL_NAME_CAPACITY 256

typedef Error (*InternalVmCall)(CPU* cpu, Memory* mem, Region* region);

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
    Region region;
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

void OnInstructionExecution(Vm* vm, size_t instructionIndex, bool debug);

void loadInternalCallIntoVm(Vm* Vm, InternalVmCall call);
void loadStandardCallsIntoVm(Vm* Vm);
/**
 * Loads the program from the specified input file into the virtual machine.
 *
 * @param vm The virtual machine instance.
 * @param inputFile The input binary file containing the program bytecode.
 */
void loadProgramIntoVm(Vm* vm, const char* inputFile);

Error vmcall_write(CPU* cpu, Memory* mem, Region* region);
Error vmcall_alloc(CPU* cpu, Memory* mem, Region* region);
Error vmcall_free(CPU* cpu, Memory* mem, Region* region);
Error vmcall_print_f64(CPU* cpu, Memory* mem, Region* region);
Error vmcall_print_i64(CPU* cpu, Memory* mem, Region* region);
Error vmcall_print_u64(CPU* cpu, Memory* mem, Region* region);
Error vmcall_print_ptr(CPU* cpu, Memory* mem, Region* region);
Error vmcall_dump_memory(CPU* cpu, Memory* mem, Region* region);

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

void OnInstructionExecution(Vm* vm, size_t instructionIndex, bool debug);

void beforeVirexStart();

void OnPause();

void afterVirexStop();

int getUserInput();

void printOut(int id, const char* str, ...);

void printOutWithColor(int id, int colorPair, const char* str, ...);

void clearWindow(int id);

void moveCursorWithinWindow(int id, int y, int x);

char getChar(int id);

void refreshAllWindows();

void readFilePath(int id, const char* msg, const char** filePath);

void InputMenu(int* highlight, int* ch);

void SetInputEnable(bool enable);

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
        fprintf(stderr, "ERROR: Could not read meta data from file `%s`: %s\n",
            filePath, strerror(errno));
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

typedef struct
{
    WINDOW* windows[MAX_WINDOW_COUNT];
    int windowCount;
} display;

volatile display disp;

static String Inputs[] = {
    [EXEC_SM]            = {.data = "Execute the Simulated Machine Code",                .length = 34},
    [CUSTOM_CMD]         = { .data = "Execute a sasm/orin command with custom flags",    .length = 45},
    [ASSEMBLE_EXEC_SASM] = { .data = "Assemble and Execute the Simulated Assembly Code", .length = 48},
    [ASSEMBLE_SASM]      = { .data = "Assemble the Simulated Assembly Code into SM",     .length = 44},
    [DISASSEMBLE_SM]     = { .data = "Disassemble the Simulated Machine Code",           .length = 38},
    [COMPILE_ORIN]       = { .data = "Compile the ORIN Code into SASM",                  .length = 33},
    [EXIT_VM]            = { .data = "Exit the Virtual Machine",                         .length = 24}
};

static String WindowNames[MAX_WINDOW_COUNT] = {
    [OUTPUT]  = {.data = "OUTPUT",   .length = 6},
    [DETAILS] = { .data = "DETAILS", .length = 7},
    [MEMORY]  = { .data = "MEMORY",  .length = 6},
    [PROGRAM] = { .data = "PROGRAM", .length = 7},
    [INPUT]   = { .data = "INPUT",   .length = 5},
    [NAME]    = { .data = "VIREX",   .length = 5},
    [STACK]   = { .data = "STACK",   .length = 5}
};

void initColors()
{
    start_color();
    init_color(COLOR_BLACK, 102, 106, 149);
    init_color(COLOR_RED, 999, 0, 333);
    init_color(COLOR_GREEN, 278, 921, 705);
    init_color(COLOR_YELLOW, 999, 795, 419);
    // init_color(COLOR_BLUE,);
    init_color(COLOR_MAGENTA, 615, 431, 995);
    init_color(COLOR_CYAN, 537, 866, 999);
    init_color(COLOR_WHITE, 815, 815, 815);
    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(3, COLOR_CYAN, COLOR_BLACK);
    init_pair(4, COLOR_GREEN, COLOR_BLACK);
    init_pair(5, COLOR_YELLOW, COLOR_BLACK);
    init_pair(7, COLOR_RED, COLOR_BLACK);
    init_pair(8, COLOR_BLACK, COLOR_WHITE);
}

void refreshWindow(int id, int contentCol, int borderCol, int titleCol)
{
    WINDOW* win = disp.windows[id];
    String str  = WindowNames[id];
    int x, y;
    getyx(win, y, x);
    if (x < 2)
        x = 2;
    if (y < 2)
        y = 2;

    int tmp = getmaxy(win) - 2;
    while (y > tmp) {
        wmove(win, 1, 0);
        wdeleteln(win);
        wmove(win, tmp, 0);
        wdeleteln(win);
        y -= 1;
    }

    wbkgd(win, COLOR_PAIR(contentCol));

    cchar_t vline, hline, ul, ur, ll, lr;

    setcchar(&vline, L"│", 0, 0, NULL);
    setcchar(&hline, L"─", 0, 0, NULL);
    setcchar(&ul, L"╭", 0, 0, NULL);
    setcchar(&ur, L"╮", 0, 0, NULL);
    setcchar(&ll, L"╰", 0, 0, NULL);
    setcchar(&lr, L"╯", 0, 0, NULL);

    wattron(win, COLOR_PAIR(borderCol));
    wborder_set(win, &vline, &vline, &hline, &hline, &ul, &ur, &ll, &lr);
    wattroff(win, COLOR_PAIR(borderCol));

    wattron(win, COLOR_PAIR(titleCol));
    moveCursorWithinWindow(id, 0, (int)((getmaxx(win) - str.length - 4) / 2));
    // wmove(win, 0, 2);
    printOut(id, "❮ %s ❯", str.data);
    wattroff(win, COLOR_PAIR(titleCol));

    moveCursorWithinWindow(id, y, x);
    wrefresh(win);
}

bool createWindow(int x1, int y1, int x2, int y2, int colorPair)
{
    int width  = x2 - x1;
    int height = y2 - y1;

    if (width <= 0 || height <= 0) {
        fprintf(stderr, "Error: Invalid window dimensions (%d, %d, %d, %d)\n", x1, y1, x2, y2);
        return false;
    }

    WINDOW* win = newwin(height, width, y1, x1);
    if (!win) {
        fprintf(stderr, "Error: Failed to create window\n");
        return false;
    }

    wbkgd(win, COLOR_PAIR(colorPair));
    disp.windows[disp.windowCount++] = win;
    refreshWindow(disp.windowCount - 1, colorPair, 5, 3);

    return true;
}

void CreateWindows()
{
    refresh();
    disp.windowCount = 0;
    int xmin = 0, ymin = 0;
    int xmax = getmaxx(stdscr), ymax = getmaxy(stdscr);

    int xsta = LERP(xmin, xmax, 0.85);
    int xmid = LERP(xsta, xmax, 0.50);
    int xmi2 = LERP(xmin, xmax, 0.68);
    int xen2 = LERP(xmin, xmax, 0.18);
    int ybot = LERP(ymin, ymax, 0.28);
    int ymid = LERP(ymin, ybot, 0.68);

    createWindow(xsta, ymin, xmid, ybot, 1);
    createWindow(xmi2, ybot, xen2, ymax, 5);
    createWindow(xmin, ymin, xsta, ybot, 4);
    createWindow(xmid, ymin, xmax, ymid, 2);
    createWindow(xmid, ymid, xmax, ybot, 3);
    createWindow(xmin, ybot, xmi2, ymax, 7);
    createWindow(xen2, ybot, xmax, ymax, 7);
}

void setInputEnable(bool enable)
{
    keypad(disp.windows[INPUT], enable);
}

void wprintdash(int id, int col)
{
    wattron(disp.windows[id], COLOR_PAIR(col));
    int tmp = getmaxx(disp.windows[id]) - 1;
    int i, j;
    getyx(disp.windows[id], j, i);
    wmove(disp.windows[id], j, i);
    for (; i < tmp; i++) {
        wprintw(disp.windows[id], "─");
    }
    wprintw(disp.windows[id], "\n\n");
    wattroff(disp.windows[id], COLOR_PAIR(col));
}

void refreshAllWindows()
{
    refreshWindow(NAME, 7, 7, 3);
    refreshWindow(STACK, 7, 7, 3);
    refreshWindow(MEMORY, 2, 2, 3);
    refreshWindow(DETAILS, 1, 1, 3);
    refreshWindow(PROGRAM, 3, 2, 3);
    refreshWindow(OUTPUT, 4, 5, 3);
    refreshWindow(INPUT, 5, 5, 3);
}

void enterTUIMode()
{
    setlocale(LC_ALL, "");
    initscr();
    clear();
    cbreak();
    // noecho();
    initColors();

    CreateWindows();
    printOut(
        NAME,
        "\n    ██╗   ██╗██╗██████╗ ████████╗██╗   ██╗ █████╗ ██╗     "     //      ██╗   ██╗██╗██████╗ ███████╗██╗  ██╗"
        "\n    ██║   ██║██║██╔══██╗╚══██╔══╝██║   ██║██╔══██╗██║     "     //      ██║   ██║██║██████╔╝█████╗   ╚███╔╝ "
        "\n    ██║   ██║██║██████╔╝   ██║   ██║   ██║███████║██║     "     //      ╚██╗ ██╔╝██║██╔══██╗██╔══╝   ██╔██╗ "
        "\n    ╚██╗ ██╔╝██║██╔══██╗   ██║   ██║   ██║██╔══██║██║     "     //       ╚████╔╝ ██║██║  ██║███████╗██╔╝ ██╗"
        "\n     ╚████╔╝ ██║██║  ██║   ██║   ╚██████╔╝██║  ██║███████╗"     //        ╚═══╝  ╚═╝╚═╝  ╚═╝╚══════╝╚═╝  ╚═╝"
        "\n      ╚═══╝  ╚═╝╚═╝  ╚═╝   ╚═╝    ╚═════╝ ╚═╝  ╚═╝╚══════╝"
        "\n    ███████╗██╗  ██╗███████╗ ██████╗██╗   ██╗████████╗ ██████╗ ██████╗ "
        "\n    ██╔════╝╚██╗██╔╝██╔════╝██╔════╝██║   ██║╚══██╔══╝██╔═══██╗██╔══██╗"
        "\n    █████╗   ╚███╔╝ █████╗  ██║     ██║   ██║   ██║   ██║   ██║██████╔╝"
        "\n    ██╔══╝   ██╔██╗ ██╔══╝  ██║     ██║   ██║   ██║   ██║   ██║██╔══██╗"
        "\n    ███████╗██╔╝ ██╗███████╗╚██████╗╚██████╔╝   ██║   ╚██████╔╝██║  ██║"
        "\n    ╚══════╝╚═╝  ╚═╝╚══════╝ ╚═════╝ ╚═════╝    ╚═╝    ╚═════╝ ╚═╝  ╚═╝"
        "\n                                                                       ");

    // printOut(CREDITS, "\n\n    VIREX, SASM\t\t: SOHAM METHA  "
    //                   "\n    AST visualizer\t: SOHAM METHA  "
    //                   "\n    Syntax Highlighter\t: SOHAM METHA  "
    //                   "\n    ORIN Compiler\t: OMKAR JAGTAP "
    //                   "\n    Core lib(Hashtable)\t: OMKAR JAGTAP  "
    //                   "\n    Core libs(other)\t: SOHAM METHA  ");

    refreshAllWindows();
    setInputEnable(true);
}

void dumpMem(const Memory* mem)
{
    printOut(MEMORY, "\n\n   ");
    for (InstAddr i = 0; i < 256; i++) {
        printOut(MEMORY, "%02X ", mem->memory[i]);
        if (i % 32 == 31) {
            printOut(MEMORY, "\n   ");
        }
    }
}

void printOutWithColor(int id, int colorPair, const char* str, ...)
{
    va_list args;
    va_start(args, str);
    wattron(disp.windows[id], COLOR_PAIR(colorPair));
    vw_printw(disp.windows[id], str, args);
    wattroff(disp.windows[id], COLOR_PAIR(colorPair));
    va_end(args);
}

void dumpStack(Vm* vm)
{
    printOut(STACK, "\n\n");

    printOut(STACK, "  │────────────────────────│\n");
    printOut(STACK, "  │ >>>>>> STACK TOP <<<<< │\n", "");
    printOut(STACK, "  │────────────────────────│\n");

    if (vm $stack_top == 0) {
        printOut(STACK, "  │ %22s │\n", "STACK EMPTY     ");
        printOut(STACK, "  │────────────────────────│\n");
    } else if (vm $stack_top >= 5) {
        for (size_t i = 1; i <= 5; i++) {
            printOut(STACK, "  │ %4lu │ %4li │ %4lf │ 0x%-4lu\n", vm $stack[vm $stack_top - i].u64, vm $stack[vm $stack_top - i].i64, vm $stack[vm $stack_top - i].f64, vm $stack_top - i);
            printOut(STACK, "  │────────────────────────│\n");
        }
    } else if (vm $stack_top < 5) {
        for (size_t i = 1; i <= vm $stack_top; i++) {
            printOut(STACK, "  │ %4lu │ %4li │ %4lf │ 0x%-4lu\n", vm $stack[vm $stack_top - i].u64, vm $stack[vm $stack_top - i].i64, vm $stack[vm $stack_top - i].f64, vm $stack_top - i);
            printOut(STACK, "  │────────────────────────│\n");
        }
    }
}

void dumpFlags(CPU* cpu)
{
    printOut(DETAILS, "\n");
    printOutWithColor(DETAILS, 8, "  FLAGS ");
    wprintdash(DETAILS, 1);
    printOut(DETAILS,
        "  HT : %c F1 : %c\t"
        "  F2 : %c F3 : %c\n"
        "  F4 : %c F5 : %c\t"
        "  F6 : %c F7 : %c\n",
        getFlag(META_HALT, cpu) ? 'T' : 'F', getFlag(META_F1, cpu) ? 'T' : 'F', getFlag(META_F2, cpu) ? 'T' : 'F',
        getFlag(META_F3, cpu) ? 'T' : 'F', getFlag(META_F4, cpu) ? 'T' : 'F', getFlag(META_F5, cpu) ? 'T' : 'F',
        getFlag(META_F6, cpu) ? 'T' : 'F', getFlag(META_F7, cpu) ? 'T' : 'F');
}

void dumpRegs(CPU* cpu)
{
    moveCursorWithinWindow(DETAILS, 2, 1);
    printOutWithColor(DETAILS, 8, "  REGISTERS ");
    wprintdash(DETAILS, 1);
    printOut(DETAILS,
        "  H0 : %ld\t"
        "  H1 : %ld\n"
        "  P0 : %ld\t"
        "  P1 : %ld\n"
        "  P2 : %ld\t"
        "  P3 : %ld\n"
        "  JS : %ld\t"
        "  KC : %ld\n"
        "  NX : %ld\t"
        "  SP : %ld\n"
        "  I0 : %ld\n"
        "  I1 : %ld\n"
        "  L0 : %ld\n"
        "  L1 : %lf\n"
        "  L2 : %ld\n"
        "  L3 : %ld\n"
        "  OP : %ld\n"
        "  QT : %ld\n"
        "  RF : %ld\n",
        cpu->registers.H0.u64, cpu->registers.H1.u64, cpu->registers.P0.u64, cpu->registers.P1.u64,
        cpu->registers.P2.u64, cpu->registers.P3.u64, cpu->registers.JS.u64, cpu->registers.KC.u64,
        cpu->registers.NX.u64, cpu->registers.SP.u64, cpu->registers.I0.u64, cpu->registers.I1.u64,
        cpu->registers.L0.u64, cpu->registers.L1.f64, cpu->registers.L2.i64, cpu->registers.L3.u64,
        cpu->registers.OP.u64, cpu->registers.QT.u64, cpu->registers.RF.u64);
}

void dumpDetails(Instruction* inst)
{

    OpcodeDetails details = getOpcodeDetails(inst->type);
    printOut(DETAILS, "\n");
    printOutWithColor(DETAILS, 8, "  INSTRUCTION ");
    wprintdash(DETAILS, 1);
    printOut(DETAILS, "  %d\t  %s", details.type, details.name);

    if (details.has_operand) {
        printOut(DETAILS,
            "\n────────╮"
            "\n    1.U │ %ld"
            "\n    1.I │ %ld"
            "\n    1.F │ %lf"
            "\n────────╯\n",
            inst->operand.u64, inst->operand.i64, inst->operand.f64);
    }

    if (details.has_operand2) {
        printOut(DETAILS,
            "────────╮"
            "\n    2.U │ %ld"
            "\n    2.I │ %ld"
            "\n    2.F │ %lf"
            "\n────────╯\n",
            inst->operand2.u64, inst->operand2.i64, inst->operand2.f64);
    }
}

void updateProgramWindow(size_t instructionIndex, size_t instructionCount, Program* prog)
{
    WINDOW* prg = disp.windows[PROGRAM];
    wmove(prg, 1, 1);

    size_t i     = (instructionIndex > 0) ? instructionIndex : 0;

    size_t count = (instructionIndex + getmaxy(prg) - 1 > instructionCount) ? instructionCount
                                                                            : instructionIndex + getmaxy(prg) - 1;

    for (; i < count; i++) {

        OpcodeDetails details = getOpcodeDetails(prog->instructions[i].type);
        if (i == instructionIndex)
            wattron(prg, A_REVERSE);

        wprintw(prg, "\n   %ld\t│ %s ", i, details.name);
        if (details.has_operand)
            wprintw(prg, "\t %" PRIu64, prog->instructions[i].operand.u64);
        if (details.has_operand2)
            wprintw(prg, "\t %" PRIu64, prog->instructions[i].operand2.u64);
        wattroff(prg, A_REVERSE);
    }
}

void clearNonIOWindows()
{
    wclear(disp.windows[PROGRAM]);
    wclear(disp.windows[DETAILS]);
    wclear(disp.windows[MEMORY]);
    wclear(disp.windows[STACK]);
}

void updateMemoryAndDetailsWindow(Vm* vm)
{
    dumpStack(vm);
    dumpMem(&(vm->mem));
    dumpRegs(&(vm->cpu));
    dumpFlags(&(vm->cpu));
    dumpDetails(&vm->prog.instructions[vm->cpu.registers.NX.u64]);
}

void OnInstructionExecution(Vm* vm, size_t instructionIndex, bool debug)
{
    updateProgramWindow(instructionIndex, vm->prog.instruction_count, &vm->prog);
    refreshAllWindows();

    if (debug) {
        OnPause();
    }

    clearNonIOWindows();
    updateMemoryAndDetailsWindow(vm);
}

void OnPause()
{
    refreshAllWindows();
    wgetch(disp.windows[INPUT]);
}

int getUserInput()
{
    int ch;
    int highlight = 0;
    do {
        InputMenu(&highlight, &ch);
    } while (ch != '\n');

    return highlight;
}

void printOut(int id, const char* str, ...)
{
    va_list args;
    va_start(args, str);
    vw_printw(disp.windows[id], str, args);
    va_end(args);
}

void clearWindow(int id)
{
    wclear(disp.windows[id]);
}

void moveCursorWithinWindow(int id, int y, int x)
{
    wmove(disp.windows[id], y, x);
}

char getChar(int id)
{
    return wgetch(disp.windows[id]);
}

void readFilePath(int id, const char* msg, const char** filePath)
{
    char buffer[100];
    printOut(id, msg);
    wgetnstr(disp.windows[id], buffer, sizeof(buffer) - 1);
    *filePath = strdup(buffer);
}

void InputMenu(int* highlight, int* ch)
{
    clearWindow(INPUT);
    refreshAllWindows();

    for (int i = 0; i < MAX_INPUTS; i++) {
        moveCursorWithinWindow(INPUT, i + 2, 4);

        if (i == *highlight) {
            wattron(disp.windows[INPUT], A_REVERSE);
            printOut(INPUT, " ❖  ");
        }
        printOut(INPUT, Inputs[i].data);
        wattroff(disp.windows[INPUT], A_REVERSE);
    }

    *ch = wgetch(disp.windows[INPUT]);

    switch (*ch) {
    case KEY_UP:
        *highlight = (*highlight == 0) ? MAX_INPUTS - 1 : *highlight - 1;
        break;
    case KEY_DOWN:
        *highlight = (*highlight + 1) % MAX_INPUTS;
        break;
    default:
        break;
    }
}

void beforeVirexStart()
{
    enterTUIMode();
}

void afterVirexStop()
{
    while (disp.windowCount > 0) {
        delwin(disp.windows[disp.windowCount--]);
    }
    endwin();
    exit(0);
}

#pragma GCC diagnostic ignored "-Wunused-parameter"
Error vmcall_write(CPU* cpu, Memory* mem, Region* region)
{
    MemoryAddr addr = cpu->registers.L0.u64;
    uint64_t count  = cpu->registers.QT.u64;

    if (addr >= MEMORY_CAPACITY) {
        return ERR_ILLEGAL_MEMORY_ACCESS;
    }

    if (addr + count < addr || addr + count >= MEMORY_CAPACITY) {
        return ERR_ILLEGAL_MEMORY_ACCESS;
    }

    for (uint64_t i = 0; i < count; i += 1) {
        if (mem->memory[addr + i] == '\\') {
            i += 1;
            if (i >= count)
                return ERR_ILLEGAL_OPERAND;
            else if (mem->memory[addr + i] == 'n')
                printOut(OUTPUT, "\n");
            else if (mem->memory[addr + i] == '_')
                printOut(OUTPUT, "_");
            else
                return ERR_ILLEGAL_OPERAND;
        } else if (mem->memory[addr + i] == '_') {
            printOut(OUTPUT, " ");
        } else {
            printOut(OUTPUT, "%c", mem->memory[addr + i]);
        }
    }

    refreshAllWindows();

    return ERR_OK;
}

Error vmcall_alloc(CPU* cpu, Memory* mem, Region* region)
{

    cpu->registers.RF.ptr = allocateRegion(region, cpu->registers.QT.u64);

    return ERR_OK;
}

Error vmcall_free(CPU* cpu, Memory* mem, Region* region)
{
    clearGarbage(region);

    return ERR_OK;
}

Error vmcall_print_f64(CPU* cpu, Memory* mem, Region* region)
{
    printOut(OUTPUT, " %lf\n", cpu->registers.L1.f64);
    return ERR_OK;
}

Error vmcall_print_i64(CPU* cpu, Memory* mem, Region* region)
{
    printOut(OUTPUT, " %" PRId64 "", cpu->registers.L2.i64);
    return ERR_OK;
}

Error vmcall_print_u64(CPU* cpu, Memory* mem, Region* region)
{
    printOut(OUTPUT, " %" PRIu64 "", cpu->registers.L3.u64);
    return ERR_OK;
}

Error vmcall_print_ptr(CPU* cpu, Memory* mem, Region* region)
{
    printOut(OUTPUT, " %p\n", cpu->registers.RF.ptr);
    return ERR_OK;
}

Error vmcall_dump_memory(CPU* cpu, Memory* mem, Region* region)
{
    MemoryAddr addr = cpu->registers.L0.u64;
    uint64_t count  = cpu->registers.QT.u64;

    if (addr >= MEMORY_CAPACITY) {
        return ERR_ILLEGAL_MEMORY_ACCESS;
    }

    if (addr + count < addr || addr + count >= MEMORY_CAPACITY) {
        return ERR_ILLEGAL_MEMORY_ACCESS;
    }

    for (uint64_t i = 0; i < count; ++i) {
        printOut(OUTPUT, " %02X ", mem->memory[addr + i]);
        if (i % 16 == 15) {
            printOut(OUTPUT, "\n ");
        }
        refreshAllWindows();
    }
    printOut(OUTPUT, "\n");
    refreshAllWindows();

    return ERR_OK;
}

Error vmcall_writeROM(CPU* cpu, Memory* mem, Region* region)
{
    MemoryAddr addr = cpu->registers.L0.u64;
    uint64_t count  = cpu->registers.QT.u64;

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
    size_t c    = vm $reg[REG_NX].u64;
    Error error = executeInst(vm);

    if (debug > 0) {
        OnInstructionExecution(vm, c, debug == 1);
    }

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

        const Error err = vm $vm_call[inst.operand.u64](&vm->cpu, &vm->mem, &vm->region);
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
        READ_OP(int8_t, I64);
        break;

    case INST_READ2I:
        READ_OP(int16_t, I64);
        break;

    case INST_READ4I:
        READ_OP(int32_t, I64);
        break;

    case INST_READ8I:
        READ_OP(int64_t, I64);
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