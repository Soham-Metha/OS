#ifndef KERN_SASM_1
#define KERN_SASM_1

#include "sasm_isa.h"
#include <common/memmanager.h>
#include <common/strings.h>
#include <common/types.h>
#include <tools/virex/include/SASM/sasm_assembler.h>
#include <tools/virex/include/libs/univ_defs.h>

// #define MAX_MEMORY_CAPACITY 2048      // Max amount of memory initially assigned to a sasm program
// #define MAX_PROGRAM_CAPACITY 1024     // Max amount of instructions in any sasm program

// ---------------------------------------------------------------------------------------------------

// typedef struct Sasm_Executable Sasm_Executable;     // All data necessary for execution of the program
// typedef struct Sasm_Metadata Sasm_Metadata;         // Headers for the executable.
// typedef struct Sasm_Context Sasm_Context;           // Context for Assembling the sasm program.

/**
 * Expects the entire program loaded into a String_View and returns the Executable.
 * Note: File Handling Not Done Here!
 */
Sasm_Executable sasm_assemble(String_View input_prog);

#endif

// ---------------------------------------------------------------------------------------------------

#ifndef IMPL_KERN_SASM_1
#undef IMPL_KERN_SASM_1

Sasm_Executable sasm_generate_executable(Sasm_Context* sasm)
{
    Sasm_Executable res = (Sasm_Executable) {
        .meta = {
                 .magic        = FILE_MAGIC,
                 .version      = FILE_VERSION,
                 .entry        = sasm->entry,
                 .prog_size    = sasm $instructionCount,
                 .mem_size     = sasm->mem_size,
                 .mem_capacity = sasm->mem_capacity,
                 },
    };

    for (uint64 i = 0; i < sasm->mem_size; i++) {
        res.memory[i] = sasm->memory[i];
    }

    for (DataEntry i = 0; i < sasm->prog.instruction_count; i++) {
        res.prog.instructions[i] = sasm->prog.instructions[i];
    }

    return res;
}

Sasm_Executable sasm_assemble(String_View input_prog)
{
    Sasm_Context sasm = { 0 };
    translateSasmRootFile(&sasm, input_prog);
    return sasm_generate_executable(&sasm);
}

#endif

/**
 * Renamed:
 *  QuadWord -> Word
 */
