#ifndef KERN_SASM_1
#define KERN_SASM_1

#define IMPL_SASM_1
#include "sasm_assembler.h"
#include "univ_defs.h"

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

#ifdef IMPL_KERN_SASM_1
#undef IMPL_KERN_SASM_1

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

bool sasm_resolve_operands(Sasm_Context* sasm)
{
    Scope* savedScope = sasm->scope;

    for (uint64 i = 0; i < sasm->symbolsCount; ++i) {
        if (!sasm->symbols[i].scope)
            err("invalid operand scope!", "");
        sasm->scope           = sasm->symbols[i].scope;

        InstAddr addr         = sasm->symbols[i].addr;
        Expr expr             = sasm->symbols[i].expr;
        FileLocation location = sasm->symbols[i].location;

        EvalResult result     = evaluateExpression(sasm, expr, location);
        if (result.status != EVAL_STATUS_OK)
            err("invalid operand status %d", result.status);
        sasm $instructions[addr].operand = result.value;
        if (expr.type == EXPR_FUNCALL && expr.value.funcall->args->value.type == EXPR_REG) {
            sasm $instructions[addr].opr1IsReg = true;
        }

        OpcodeDetails inst_def = getOpcodeDetails(sasm $instructions[addr].type);
        if (!inst_def.has_operand)
            err("trying to resolve operand for an inst that doesnt expect an operand!", "");

        if (!inst_def.has_operand2)
            continue;
        i += 1;
        Expr expr2         = sasm->symbols[i].expr;
        EvalResult result2 = evaluateExpression(sasm, expr2, location);
        if (result2.status != EVAL_STATUS_OK)
            err("invalid operand status %d", result2.status);
        sasm $instructions[addr].operand2 = result2.value;
        if (expr.type == EXPR_FUNCALL && expr.value.funcall->args->value.type == EXPR_REG) {
            sasm $instructions[addr].opr2IsReg = true;
        }
    }

    sasm->scope = savedScope;
    return true;
ret_err:
    return false;
}

Binding* binding_resolve(Sasm_Context* sasm, String_View name)
{
    for (Scope* scope = sasm->scope; scope != NULL; scope = scope->previous) {
        Binding* binding = resolveBindingLocalScope(scope, name);
        if (binding)
            return binding;
    }

    return NULL;
}

EvalResult binding_eval(Sasm_Context* sasm, Binding* binding)
{
    switch (binding->status) {
    case BIND_STATUS_UNEVALUATED:
        binding->status   = BIND_STATUS_EVALUATING;
        EvalResult result = evaluateExpression(sasm, binding->expr, binding->location);
        binding->status   = BIND_STATUS_EVALUATED;

        if (result.status == EVAL_STATUS_OK) {
            binding->type  = result.type;
            binding->value = result.value;
        }

        return result;
    case BIND_STATUS_EVALUATING:
        log(FLFmt ": ERROR: cycling binding definition.\n", FLArg(binding->location));
        return (EvalResult) { .status = EVAL_CYCLIC };
    case BIND_STATUS_EVALUATED:
        return resultOK(binding->value, binding->type);
    case BIND_STATUS_DEFERRED:
        return resultUnresolved(binding);
    }
}

bool sasm_resolve_entry_point(Sasm_Context* sasm)
{
    Scope* savedScope = sasm->scope;

    if (sasm->deferredEntry.bindingName.len <= 0)
        goto ret_ok;
    if (!sasm->deferredEntry.scope)
        err("INVALID SCOPE!", "");
    sasm->scope = sasm->deferredEntry.scope;

    if (sasm->hasEntry)
        err(FLFmt ": ERROR: entry point has been already set!\n" FLFmt ": NOTE: the first entry point\n",
            FLArg(sasm->deferredEntry.location), FLArg(sasm->entryLocation));

    Binding* binding = binding_resolve(sasm, sasm->deferredEntry.bindingName);
    if (binding == NULL)
        err(FLFmt ": ERROR: unknown binding `%.*s`\n",
            FLArg(sasm->deferredEntry.location), Str_Fmt(sasm->deferredEntry.bindingName));

    if (binding->type != BIND_TYPE_INST_ADDR)
        err(FLFmt ": ERROR: Type check error. Trying to set `%.*s` that has the type of %s as an entry point. Entry point has to be %s.\n",
            FLArg(sasm->deferredEntry.location), Str_Fmt(binding->name), getNameOfBindType(binding->type), getNameOfBindType(BIND_TYPE_INST_ADDR));

    EvalResult result = binding_eval(sasm, binding);
    if (result.status != EVAL_STATUS_OK)
        err("Unable to resolve entry point!", "");

    sasm->entry         = result.value.u64;
    sasm->hasEntry      = true;
    sasm->entryLocation = sasm->deferredEntry.location;

ret_ok:
    sasm->scope = savedScope;
    return true;
ret_err:
    return false;
}

bool sasm_translate_root_file(Sasm_Context* sasm, String_View input_file_data)
{
    scope_push(sasm);
    translateSasmFile(sasm, input_file_data, STR("src"));
    scope_pop(sasm);

    return sasm_resolve_operands(sasm) || sasm_resolve_entry_point(sasm);
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
        .prog = {
                 .instruction_count = sasm $instructionCount,
                 }
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
