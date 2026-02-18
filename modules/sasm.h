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

Sasm_Executable sasm_generate_executable(Sasm_Context* sasm);
void sasm_translate_root_file(Sasm_Context* sasm, String_View input_file_data);
void sasm_resolve_operands(Sasm_Context* sasm);
void scope_push(Sasm_Context* sasm);
void scope_pop(Sasm_Context* sasm);

void scope_push(Sasm_Context* sasm)
{
    Scope* scope    = (Scope*)region_alloc(&sasm->arena, sizeof(*sasm->scope));
    scope->previous = sasm->scope;
    sasm->scope     = scope;
}

void scope_pop(Sasm_Context* sasm)
{
    assert(sasm->scope != NULL);
    sasm->scope = sasm->scope->previous;
}

void sasm_resolve_operands(Sasm_Context* sasm)
{
    Scope* savedScope = sasm->scope;

    for (uint64 i = 0; i < sasm->symbolsCount; ++i) {
        assert(sasm->symbols[i].scope);
        sasm->scope           = sasm->symbols[i].scope;

        InstAddr addr         = sasm->symbols[i].addr;
        Expr expr             = sasm->symbols[i].expr;
        FileLocation location = sasm->symbols[i].location;

        EvalResult result     = evaluateExpression(sasm, expr, location);
        assert(result.status == EVAL_STATUS_OK);
        sasm $instructions[addr].operand = result.value;
        if (expr.type == EXPR_FUNCALL && expr.value.funcall->args->value.type == EXPR_REG) {
            sasm $instructions[addr].opr1IsReg = true;
        }

        OpcodeDetails inst_def = getOpcodeDetails(sasm $instructions[addr].type);
        assert(inst_def.has_operand);

        if (!inst_def.has_operand2)
            continue;
        i++;
        Expr expr2         = sasm->symbols[i].expr;
        EvalResult result2 = evaluateExpression(sasm, expr2, location);
        assert(result2.status == EVAL_STATUS_OK);
        sasm $instructions[addr].operand2 = result2.value;
        if (expr.type == EXPR_FUNCALL && expr.value.funcall->args->value.type == EXPR_REG) {
            sasm $instructions[addr].opr2IsReg = true;
        }
    }

    sasm->scope = savedScope;
}

void sasm_translate_root_file(Sasm_Context* sasm, String_View input_file_data)
{
    scope_push(sasm);
    translateSasmFile(sasm, input_file_data, STR("src"));
    scope_pop(sasm);

    sasm_resolve_operands(sasm);
    resolveProgramEntryPoint(sasm);
}

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
    sasm_translate_root_file(&sasm, input_prog);
    return sasm_generate_executable(&sasm);
}

#endif
