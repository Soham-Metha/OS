/*
 * sasm.h
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
#ifndef KERN_SASM_1
#define KERN_SASM_1

#define IMPL_SASM_1

#if defined(__clang__)
#pragma clang diagnostic ignored "-Wdollar-in-identifier-extension"
#endif

#include <common/errors.h>
#include <common/memmanager.h>
#include <common/strings.h>
#include <common/types.h>
#include <kernel/heap.h>
#include <osapi/io.h>

#define NULL ((void*)0)

typedef uint8 Byte;
typedef uint16 Word;
typedef uint32 DoubleWord;

typedef uint32 DataEntry;
typedef uint32 InstAddr;
typedef uint32 MemoryAddr;
typedef uint32 StackAddr;
typedef uint32 u32;
typedef int32 i32;
typedef float f32;

typedef union {
    uint32 u32;
    int32 i32;
    float f32;
    void* ptr;
} QuadWord;

#define MAX_OPERAND_CNT 2             // Per instruction
#define MAX_MEMORY_CAPACITY 2048      // Max amount of memory initially assigned to a sasm program (in bytes)
#define MAX_PROGRAM_CAPACITY 2048     // Max size of any program's instructions (in bytes)
#define INTERNAL_VMCALLS_CAPACITY 64
#define BINDINGS_CAPACITY 256
#define LABELS_CAPACITY 256
#define MAX_INCLUDE_LEVEL 10
#define STACK_CAPACITY 1024
#define FILE_MAGIC 0x484f53
#define FILE_VERSION 0x4D41
#define STRING_LENGTHS_CAPACITY 1024
#define INCLUDE_PATHS_CAPACITY 128

#define COMMENT_SYMBOL ';'
#define PREP_SYMBOL '%'

#define $code ->prog.code
#define $code_size ->prog.code_size
#define Token_Fmt "%s"
#define Token_Arg(token) token_kind_name((token).kind)

#define PRIu64 "llu"
#define PRIi64 "lli"
#define PRId64 "lld"
#define PRIX64 "llX"

// ---------------------------------------------------------------------------------------------------

// X(name, op_cnt)
// NOTE: inst handler in virex.h
#define ISA_OPCODE_LIST(X) \
    X(DONOP,  0) \
    X(SHUTS,  0) \
    X(INVOK,  1) \
    X(SPOPR,  1) X(SETR,   2) X(COPY,   2) \
    X(PUSH,   1) X(SPOP,   0) X(DUPS,   1) X(SWAP,   1) \
    X(ADDI,   0) X(SUBI,   0) X(MULI,   0) X(DIVI,   0) X(MODI,   0) \
    X(ADDU,   0) X(SUBU,   0) X(MULU,   0) X(DIVU,   0) X(MODU,   0) \
    X(ADDF,   0) X(SUBF,   0) X(MULF,   0) X(DIVF,   0) \
    X(JMPU,   1) X(RET,    0) X(CALL,   1) \
    X(JMPC,   1) X(LOOP,   2) \
    X(NOT,    0) X(NOTB,   0) \
    X(EQI,    0) X(GEI,    0) X(GTI,    0) X(LEI,    0) X(LTI,    0) X(NEI,    0) \
    X(EQU,    0) X(GEU,    0) X(GTU,    0) X(LEU,    0) X(LTU,    0) X(NEU,    0) \
    X(EQF,    0) X(GEF,    0) X(GTF,    0) X(LEF,    0) X(LTF,    0) X(NEF,    0) \
    X(ANDB,   0) X(ORB,    0) X(XOR,    0) X(SHR,    0) X(SHL,    0) \
    X(I2F,    0) X(U2F,    0) X(F2I,    0) X(F2U,    0) \
    X(READ1U, 0) X(READ2U, 0) X(READ4U, 0) X(READ8U, 0) \
    X(READ1I, 0) X(READ2I, 0) X(READ4I, 0) X(READ8I, 0) \
    X(WRITE1, 0) X(WRITE2, 0) X(WRITE4, 0) X(WRITE8, 0)

#define OPCODE_ENUM(name, op_cnt) \
    INST_##name,

typedef enum {
    ISA_OPCODE_LIST(OPCODE_ENUM)
    NUMBER_OF_INSTS
} Opcode;

#undef OPCODE_ENUM

typedef enum {
    REG_U0,
    REG_U1,
    REG_U2,
    REG_U3,
    REG_U4,
    REG_U5,
    REG_U6,
    REG_U7,
    REG_U8,
    REG_U9,
    REG_S0,
    REG_S1,
    REG_S2,
    REG_S3,
    REG_S4,
    REG_S5,
    REG_S6,
    REG_IP,
    REG_SP,
    REG_COUNT,
} RegID;

typedef enum {
    OPR_NONE,
    OPR_DIRECT,
    OPR_REGISTER,
    OPR_REGISTER_INDIRECT,
    OPR_CNT,
} Opr_Kind;

enum StmtType {
    STMT_INST,
    STMT_LABEL,
    STMT_CONST,
    STMT_INCLUDE,
    STMT_ENTRY,
    STMT_BLOCK,
    STMT_SCOPE,
};

enum BindingStatus {
    BIND_STATUS_UNEVALUATED = 0,
    BIND_STATUS_EVALUATING,
    BIND_STATUS_EVALUATED,
    BIND_STATUS_DEFERRED,
};

enum EvalStatus {
    EVAL_STATUS_OK = 0,
    EVAL_STATUS_DEFERRED,
    EVAL_ERR,
};

enum TokenType {
    TOKEN_TYPE_STR,
    TOKEN_TYPE_CHAR,
    TOKEN_TYPE_NUMBER,
    TOKEN_TYPE_NAME,
    TOKEN_TYPE_OPEN_PAREN,
    TOKEN_TYPE_CLOSING_PAREN,
    TOKEN_TYPE_COMMA,
    TOKEN_TYPE_REGISTER
};

enum LineType {
    LINE_INSTRUCTION = 0,
    LINE_LABEL,
    LINE_DIRECTIVE,
};

enum ExprType {
    EXPR_BINDING,
    EXPR_LIT_INT,
    EXPR_LIT_FLOAT,
    EXPR_LIT_CHAR,
    EXPR_LIT_STR,
    EXPR_FUNCALL,
    EXPR_REG,
};

enum BindingType {
    BIND_TYPE_FLOAT,
    BIND_TYPE_UINT,
    BIND_TYPE_MEM_ADDR,
    BIND_TYPE_INST_ADDR,
};


typedef QuadWord Register;
typedef struct __attribute__((__packed__)) Instruction Instruction;
typedef struct OpcodeDetails OpcodeDetails;
typedef struct Program Program;
typedef struct Memory Memory;
typedef struct SasmLexer SasmLexer;                         // Lexer ( performs "Get Next Line", "Move To Next Line", "Load File In")
typedef struct Line Line;                                   // └───>  Line
typedef enum LineType LineType;                             //        └───> LineType
typedef union LineValue LineValue;                          //        └───> LineValue
typedef struct SasmDirectiveLine SasmDirectiveLine;         //              └───> Directives
typedef struct SasmLabelLine SasmLabelLine;                 //              └───> Labels
typedef struct SasmInstructionLine SasmInstructionLine;     //              └───> Instructions
typedef struct Tokenizer Tokenizer;                         // Tokenizer ( performs "Get Next Token", "Move To Next Token", "Load Line In")
typedef struct Token Token;                                 // └───> Token
typedef enum TokenType TokenType;                           //       └───> TokenType
typedef struct CodeBlock CodeBlock;                         // CodeBlock ( Linked List Of Statements )
typedef struct StmtNode StmtNode;                           // └───> StmtNode
typedef struct Stmt Stmt;                                   //       └───> Statement
typedef enum StmtType StmtType;                             //             └───> StatementType
typedef union StmtValue StmtValue;                          //             └───> StatementValue
typedef struct LabelStmt LabelStmt;                         //                   └───> Labels
typedef struct ConstStmt ConstStmt;                         //                   └───> Constant Declaration
typedef struct IncludeStmt IncludeStmt;                     //                   └───> Include
typedef struct EntryStmt EntryStmt;                         //                   └───> Entry Point Declaration
typedef struct InstStmt InstStmt;                           //                   └───> Instructions
typedef struct Expr Expr;                                   //                         └───>  Expression (operands passed)
typedef enum ExprType ExprType;                             //                                └───> Expression type
typedef union ExprValue ExprValue;                          //                                └───> Expression Value
typedef struct Funcall Funcall;                             //                                      └───> Compile Time Functions
typedef struct FuncallArg FuncallArg;                       //                                            └───> Arguments for Compile Time Functions (Linked List of Expressions)
typedef struct EvalResult EvalResult;                       // Evaluation Result
typedef enum EvalStatus EvalStatus;                         // └───> Evaluation Status
typedef struct Binding Binding;                             // └───> Binding Being Evaluated
typedef enum BindingType BindingType;                       //       └───> Binding Type
typedef enum BindingStatus BindingStatus;                   //       └───> Binding Status

typedef struct Sasm_Context Sasm_Context;
typedef struct StringLength StringLength;
typedef struct Scope Scope;
typedef struct UnresolvedOperand UnresolvedOperand;
typedef struct DeferredAssert DeferredAssert;
typedef struct DeferredEntry DeferredEntry;

typedef struct __attribute__((__packed__)) Sasm_Metadata Sasm_Metadata;

typedef struct {
    String_View filePath;
    int lineNumber;
} FileLocation;

#define FLFmt "%.*s:%d"
#define FLArg(location) Str_Fmt(location.filePath), location.lineNumber

struct Operand {
    Opr_Kind kind;
    QuadWord value;
};

struct Instruction {
    Opcode type;
    struct Operand opr[2];
};

struct OpcodeDetails {
    Opcode type;
    const char* name;
    uint8 operand_cnt;
};

struct Program {
    Byte code[MAX_PROGRAM_CAPACITY];
    DataEntry code_size;
};

struct Memory {
    QuadWord stack[STACK_CAPACITY];
    Byte memory[MAX_MEMORY_CAPACITY];
};

union ExprValue {
    String_View binding;
    u32 lit_int;
    double lit_float;
    char lit_char;
    String_View lit_str;
    Funcall* funcall;
    u32 reg_id;
};

struct Expr {
    ExprType type;
    ExprValue value;
};

struct FuncallArg {
    FuncallArg* next;
    Expr value;
};

struct Funcall {
    String_View name;
    FuncallArg* args;
};

struct SasmInstructionLine {
    String_View name;
    String_View operand;
    String_View operand2;
};

struct SasmLabelLine {
    String_View name;
};

struct SasmDirectiveLine {
    String_View name;
    String_View body;
};

union LineValue {
    SasmInstructionLine instruction;
    SasmLabelLine label;
    SasmDirectiveLine directive;
};

struct Line {
    LineType kind;
    LineValue value;
    FileLocation location;
};

struct SasmLexer {
    String_View source;
    FileLocation location;
    Line cachedToken;
    bool hasCachedToken;
};

struct InstStmt {
    Opcode type;
    Expr operands[MAX_OPERAND_CNT];
};

struct LabelStmt {
    String_View name;
};

struct ConstStmt {
    String_View name;
    Expr value;
};

struct IncludeStmt {
    String_View path;
    String_View content;
};

struct EntryStmt {
    Expr value;
};

union StmtValue {
    InstStmt inst;
    LabelStmt label;
    ConstStmt constant;
    IncludeStmt include;
    EntryStmt entry;
    StmtNode* block;
    StmtNode* scope;
};

struct Stmt {
    StmtType kind;
    StmtValue value;
    FileLocation location;
};

struct StmtNode {
    Stmt statement;
    StmtNode* next;
};

struct CodeBlock {
    StmtNode* begin;
    StmtNode* end;
};

struct Token {
    TokenType type;
    String_View text;
};

struct Tokenizer {
    String_View source;
    Token cachedToken;
    bool hasCachedToken;
};

struct Binding {
    BindingType type;
    String_View name;
    QuadWord value;
    Expr expr;
    BindingStatus status;
    FileLocation location;
};

struct StringLength {
    InstAddr addr;
    u32 len;
};

struct EvalResult {
    EvalStatus status;
    Binding* unresolvedBinding;
    QuadWord value;
    BindingType type;
};

struct Scope {
    Scope* previous;

    Binding bindings[BINDINGS_CAPACITY];
    uint32 bindingsCnt;
};

struct UnresolvedOperand {
    InstAddr addr;
    Expr expr;
    FileLocation location;
    Scope* scope;
};

struct DeferredEntry {
    String_View bindingName;
    FileLocation location;
    Scope* scope;
};

struct Sasm_Context {
    Scope* scope;
    Binding bindings[BINDINGS_CAPACITY];
    uint32 bindingCount;

    UnresolvedOperand symbols[LABELS_CAPACITY];
    uint32 symbolsCount;

    DeferredEntry deferredEntry;

    Program prog;

    bool hasEntry;
    InstAddr entry;
    FileLocation entryLocation;

    StringLength stringLens[STRING_LENGTHS_CAPACITY];
    uint32 strLensCnt;

    Byte memory[MAX_MEMORY_CAPACITY];
    uint32 mem_size;
    uint32 mem_capacity;

    Arena arena;

    uint32 includeLevel;
    FileLocation includeLocation;

    String_View includePaths[INCLUDE_PATHS_CAPACITY];
    uint32 includePathsCnt;
};

struct Sasm_Metadata {
    uint32 magic;               // random value
    uint32 version;             // assembler ver
    DataEntry entry;            // idx of the instruction where execution should start
    DataEntry prog_size;        // instruction count
    DataEntry mem_size;         // memory used
    DataEntry mem_capacity;     // max amt memory that can be used
};

typedef struct Sasm_Executable {
    Sasm_Metadata meta;
    Byte memory[MAX_MEMORY_CAPACITY];
    Program prog;
} Sasm_Executable;

/**
 * Expects the entire program loaded into a String_View and returns the Executable.
 * Note: File Handling Not Done Here!
 */
void sasm_assemble(Sasm_Executable* exec, String_View input_prog);

#endif

// ---------------------------------------------------------------------------------------------------

#ifdef IMPL_KERN_SASM_1
#undef IMPL_KERN_SASM_1

QuadWord quadwordFromU64(u32 u64);
QuadWord quadwordFromI64(i32 i64);
QuadWord quadwordFromF64(f32 f64);
QuadWord quadwordFromPtr(void* ptr);

bool          opcode_get_details_from_name(String_View name, OpcodeDetails* outPtr);
OpcodeDetails opcode_get_details(Opcode type);

// (Parse) Multi-file level
bool sasm_add_include_path(Sasm_Context* sasm, const char* path);
bool sasm_translate_root_file(Sasm_Context* sasm, String_View input_file_data);
// (Parse) File level
bool sasm_translate_file(Sasm_Context* sasm, String_View inputFileData, String_View inputFilePath);
bool sasm_lexer_read_file(SasmLexer* lineInterpreter, String_View file_content, String_View filePath);
bool sasm_lexer_seek(SasmLexer* lineInterpreter, Line* output);
bool sasm_lexer_consume(SasmLexer* lineInterpreter, Line* output);
CodeBlock sasm_parse_lines(Arena* arena, SasmLexer* lineInterpreter);

// (Parse) Line level
bool sasm_codeblock_push(Arena* arena, CodeBlock* list, Stmt statement);
bool sasm_parse_directive(Arena* arena, SasmLexer* lineInterpreter, CodeBlock* output);
bool sasm_defer_operand(Sasm_Context* sasm, InstAddr addr, Expr expr, FileLocation location);
bool sasm_tokenizer_seek(Tokenizer* tokenizer, Token* output, FileLocation location);
bool sasm_tokenizer_consume(Tokenizer* tokenizer, Token* token, FileLocation location);
Tokenizer sasm_tokenizer_load_sv(String_View source);
bool sasm_translate_stmt_chain(Sasm_Context* sasm, StmtNode* block);

// (Parse) Statement level
bool sasm_translate_bind_directive(Sasm_Context* sasm, ConstStmt konst, FileLocation location);
bool sasm_translate_incl_directive(Sasm_Context* sasm, IncludeStmt include, FileLocation location);
bool sasm_translate_entr_directive(Sasm_Context* sasm, EntryStmt entry, FileLocation location);
bool sasm_translate_inst(Sasm_Context* sasm, InstStmt inst, FileLocation location);
bool sasm_tokenizer_expect_none(Tokenizer* tokenizer, FileLocation location);
void sasm_scope_push(Sasm_Context* sasm);
void sasm_scope_pop(Sasm_Context* sasm);

// (Parse) Expression level
bool     sasm_local_defr_binding(Scope* scope, String_View name, BindingType type, FileLocation location);
Binding* sasm_local_find_binding(Scope* scope, String_View name);
Expr        sasm_parse_expr(Arena* arena, String_View source, FileLocation location);
Expr        sasm_tokenizer_parse_expr(Arena* arena, Tokenizer* tokenizer, FileLocation location);
Expr        sasm_tokenizer_parse_num(Arena* arena, Tokenizer* tokenizer, FileLocation location);
String_View sasm_tokenizer_parse_sv(Tokenizer* tokenizer, FileLocation location);
FuncallArg* sasm_parse_funcall_arglist(Arena* arena, Tokenizer* tokenizer, FileLocation location);
bool        sasm_scope_bind_expr(Scope* scope, String_View name, Expr expr, FileLocation location);

// (Resolve) Statement level
Binding* sasm_global_find_binding(Sasm_Context* sasm, String_View name);
EvalResult sasm_resolve_binding(Sasm_Context* sasm, Binding* binding);

// (Resolve) Expression level
bool sasm_funcall_expect_arity(Funcall* funcall, uint32 expected_arity, FileLocation location);
EvalResult sasm_resolve_funcall(Sasm_Context* sasm, Expr expr, FileLocation location);
EvalResult sasm_resolve_expr(Sasm_Context* sasm, Expr expr, FileLocation location);
QuadWord sasm_mempush_str(Sasm_Context* sasm, String_View str);
bool sasm_resolve_strlen(Sasm_Context* sasm, InstAddr str_addr, QuadWord* length);
uint32 sasm_funcall_resolve_arg_cnt(FuncallArg* args);

// (Resolve) Multi-File level
bool sasm_resolve_operands(Sasm_Context* sasm);
bool sasm_resolve_entry_point(Sasm_Context* sasm);
void sasm_generate_executable(Sasm_Executable* exec, Sasm_Context* sasm);

// void sasm_read_executable(Sasm_Context* sasm, const char* filePath);

const char* nameof_bind_type(BindingType type);
const char* nameof_token(TokenType type);
const char* nameof_register(RegID type);

EvalResult resultOK(QuadWord value, BindingType type);
EvalResult resultUnresolved(Binding* unresolvedBinding);

#define OPCODE_DETAILS(opcode, op_cnt) \
    [INST_##opcode] = {                  \
        .type = INST_##opcode,           \
        .name = #opcode,                 \
        .operand_cnt = op_cnt            \
    },

static OpcodeDetails OpcodeDetailsLUT[NUMBER_OF_INSTS] = {
    ISA_OPCODE_LIST(OPCODE_DETAILS)
};

#undef OPCODE_DETAILS

bool opcode_get_details_from_name(String_View name, OpcodeDetails* out_ptr)
{
    for (Opcode type = 0, last = NUMBER_OF_INSTS; type < last; type += 1) {
        if (sv_compare(STR(OpcodeDetailsLUT[type].name), name)) {
            *out_ptr = OpcodeDetailsLUT[type];
            return true;
        }
    }

    return false;
}

OpcodeDetails opcode_get_details(Opcode type)
{
    try(type < NUMBER_OF_INSTS, "Invalid opcode %d", type);
    return OpcodeDetailsLUT[type];

ret_err:
    return OpcodeDetailsLUT[INST_DONOP];
}

/*
    void sasm_read_executable(Sasm_Context* sasm, const char* filePath)
    {
        memset(sasm, 0, sizeof(*sasm));
        FILE* f       = openFile(filePath, "rb");
        Sasm_Metadata meta = { 0 };
        uint32 n      = fread(&meta, sizeof(meta), 1, f);

        if (n < 1) {
            fileErrorDispWithExit("Could not read meta data from file", filePath);
        }
        if (meta.magic != FILE_MAGIC) {
            printf("Unexpected magic %04X. Expected %04X.\n", meta.magic, FILE_MAGIC);
            fileErrorDispWithExit("Not a valid SASM File ", filePath);
        }
        if (meta.version != FILE_VERSION) {
            printf("Encountered version %d. Expected version %d.\n", meta.version, FILE_VERSION);
            fileErrorDispWithExit("unsupported version of SASM File ", filePath);
        }
        if (meta.prog_size > MAX_PROGRAM_CAPACITY) {
            printf(
                "The file contains %" PRIu64 " program instruction. But the capacity is %" PRIu64 "\n",
                meta.prog_size, (u64)MAX_PROGRAM_CAPACITY);
            fileErrorDispWithExit("program section is too big ", filePath);
        }
        if (meta.mem_capacity > MAX_MEMORY_CAPACITY) {
            printf(
                "The file wants %" PRIu64 " bytes. But the capacity is %" PRIu64 " bytes\n",
                meta.mem_capacity, (u64)MAX_MEMORY_CAPACITY);
            fileErrorDispWithExit(" memory section is too big ", filePath);
        }
        if (meta.mem_size > meta.mem_capacity) {
            printf(
                "ERROR: %s: memory size %" PRIu64 " is greater than declared memory capacity %" PRIu64 "\n",
                filePath, meta.mem_size, meta.mem_capacity);
            exit(1);
        }
        if (meta.externalsSize > EXTERNAL_VMCALLS_CAPACITY) {
            printf(
                "ERROR: %s: external names section is too big. The file contains %" PRIu64 " external names. But the capacity is %" PRIu64 " external names\n",
                filePath, meta.externalsSize, (u64)EXTERNAL_VMCALLS_CAPACITY);
            exit(1);
        }

        sasm $instructionCount = fread(sasm $instructions, sizeof(sasm $instructions[0]), meta.prog_size, f);
        if (sasm $instructionCount != meta.prog_size) {
            printf("ERROR: %s: read %" PRIu64 " program instructions, but expected %" PRIu64 "\n",
                filePath, sasm $instructionCount, meta.prog_size);
            exit(1);
        }
        n = fread(sasm->memory, sizeof(sasm->memory[0]), meta.mem_size, f);
        if (n != meta.mem_size) {
            printf("ERROR: %s: read %" PRIu64 " bytes of memory section, but expected %" PRIu64 " bytes.\n",
                filePath, n, meta.mem_size);
            exit(1);
        }
        closeFile(f, filePath);
    }
*/

const char* nameof_register(RegID type)
{
    switch (type) {
    case REG_U0: return "U0";
    case REG_U1: return "U1";
    case REG_U2: return "U2";
    case REG_U3: return "U3";
    case REG_U4: return "U4";
    case REG_U5: return "U5";
    case REG_U6: return "U6";
    case REG_U7: return "U7";
    case REG_U8: return "U8";
    case REG_U9: return "U9";
    case REG_S0: return "S0";
    case REG_S1: return "S1";
    case REG_S2: return "S2";
    case REG_S3: return "S3";
    case REG_S4: return "S4";
    case REG_S5: return "S5";
    case REG_S6: return "S6";
    case REG_IP: return "IP";
    case REG_SP: return "SP";
    case REG_COUNT:
    default:     return "";
    }
}

Binding* sasm_local_find_binding(Scope* scope, String_View name)
{
    for (uint32 i = 0; i < scope->bindingsCnt; ++i) {
        if (sv_compare(scope->bindings[i].name, name)) {
            return &scope->bindings[i];
        }
    }

    return NULL;
}

const char* nameof_bind_type(BindingType type)
{
    switch (type) {
    case BIND_TYPE_FLOAT:     return "Float";
    case BIND_TYPE_UINT:      return "Unsigned_Int";
    case BIND_TYPE_MEM_ADDR:  return "Mem_Addr";
    case BIND_TYPE_INST_ADDR: return "Inst_Addr";
    default: return "";
    }
}

bool sasm_resolve_strlen(Sasm_Context* sasm, InstAddr addr, QuadWord* length)
{
    for (uint32 i = 0; i < sasm->strLensCnt; ++i) {
        if (sasm->stringLens[i].addr == addr) {
            if (length) {
                *length = quadwordFromU64(sasm->stringLens[i].len);
            }
            return true;
        }
    }

    return false;
}

uint32 sasm_funcall_resolve_arg_cnt(FuncallArg* args)
{
    uint32 result = 0;
    while (args != NULL) {
        result += 1;
        args = args->next;
    }
    return result;
}

inline EvalResult resultOK(QuadWord value, BindingType type)
{
    return (EvalResult) {
        .status = EVAL_STATUS_OK,
        .value  = value,
        .type   = type,
    };
}

inline EvalResult resultUnresolved(Binding* unresolvedBinding)
{
    return (EvalResult) {
        .status            = EVAL_STATUS_DEFERRED,
        .unresolvedBinding = unresolvedBinding
    };
}

bool sasm_lexer_read_file(SasmLexer* lineInterpreter, String_View file_content, String_View filePath)
{
    if (!lineInterpreter) return false;

    lineInterpreter->source            = file_content;
    lineInterpreter->location.filePath = filePath;

    return true;
}

bool sasm_lexer_seek(SasmLexer* lineInterpreter, Line* output)
{
    if (lineInterpreter->hasCachedToken) {
        if (output) {
            *output = lineInterpreter->cachedToken;
        }
        return true;
    }

    String_View line = { 0 };
    do {
        line = sv_trim(sv_split_by_delim(&lineInterpreter->source, '\n'));
        line = sv_trim(sv_split_by_delim(&line, COMMENT_SYMBOL));
        lineInterpreter->location.lineNumber += 1;
    } while (line.len == 0 && lineInterpreter->source.len > 0);

    if (line.len == 0 && lineInterpreter->source.len == 0) {
        return false;
    }

    Line result     = { 0 };
    result.location = lineInterpreter->location;
    if (sv_starts_with(line, STR("%"))) {
        sv_split_by_len(&line, 1);
        result.kind                 = LINE_DIRECTIVE;
        result.value.directive.name = sv_trim(sv_split_by_delim(&line, ' '));
        result.value.directive.body = sv_trim(line);
    } else if (sv_ends_with(line, STR(":"))) {
        result.kind             = LINE_LABEL;
        result.value.label.name = sv_trim(sv_split_by_delim(&line, ':'));
    } else {
        result.kind                      = LINE_INSTRUCTION;
        result.value.instruction.name    = sv_trim(sv_split_by_delim(&line, ' '));
        result.value.instruction.operand = sv_trim(line);
    }

    if (output) {
        *output = result;
    }

    lineInterpreter->hasCachedToken = true;
    lineInterpreter->cachedToken    = result;

    return true;
}

bool sasm_lexer_consume(SasmLexer* lineInterpreter, Line* output)
{
    if (sasm_lexer_seek(lineInterpreter, output)) {
        lineInterpreter->hasCachedToken = false;
        return true;
    }

    return false;
}

static bool isName(char x)
{
    return (x >= 'a' && x <= 'z') || (x >= 'A' && x <= 'Z') || (x >= '0' && x <= '9') || x == '_';
}

static bool isalphabet(char x)
{
    return (x >= 'a' && x <= 'z') || (x >= 'A' && x <= 'Z');
}

static bool isNumber(char x)
{
    return (x >= '0' && x <= '9') || x == '.' || x == '-';
}

static bool is_digit(char x)
{
    return (x >= '0' && x <= '9');
}

bool sasm_tokenizer_consume(Tokenizer* tokenizer, Token* token, FileLocation location)
{
    if (sasm_tokenizer_seek(tokenizer, token, location)) {
        tokenizer->hasCachedToken = false;
        return true;
    }

    return false;
}

Tokenizer sasm_tokenizer_load_sv(String_View source)
{
    return (Tokenizer) {
        .source = source
    };
}

const char* nameof_token(TokenType type)
{
    switch (type) {
    case TOKEN_TYPE_STR:            return "string";
    case TOKEN_TYPE_CHAR:           return "character";
    case TOKEN_TYPE_NUMBER:         return "number";
    case TOKEN_TYPE_NAME:           return "name";
    case TOKEN_TYPE_OPEN_PAREN:     return "open paren";
    case TOKEN_TYPE_CLOSING_PAREN:  return "closing paren";
    case TOKEN_TYPE_COMMA:          return "comma";
    case TOKEN_TYPE_REGISTER:       return "register";
    default: return "";
    }
}

Expr sasm_parse_expr(Arena* arena, String_View source, FileLocation location)
{
    Tokenizer tokenizer = sasm_tokenizer_load_sv(source);
    Expr result         = sasm_tokenizer_parse_expr(arena, &tokenizer, location);

    return result;
}

void sasm_scope_push(Sasm_Context* sasm)
{
    ResultPtr space = region_alloc(&sasm->arena, sizeof(Scope));
    try(RESULT_OK(space), "out of space!", "");
    Scope* scope    = (Scope*)RESULT_VAL(space);
    scope->previous = sasm->scope;
    sasm->scope     = scope;
ret_err:;
}

void sasm_scope_pop(Sasm_Context* sasm)
{
    try(sasm->scope != NULL, "No scope to pop!", "");
    sasm->scope = sasm->scope->previous;
ret_err:;
}

bool sasm_resolve_operands(Sasm_Context* sasm)
{
    Scope* savedScope = sasm->scope;

    for (uint32 i = 0; i < sasm->symbolsCount; ++i) {
        UnresolvedOperand* symbol = &sasm->symbols[i];

        try(symbol->scope, "invalid operand scope!", "");
        sasm->scope = symbol->scope;

        EvalResult oper     = sasm_resolve_expr(sasm, symbol->expr, symbol->location);
        try(oper.status == EVAL_STATUS_OK, "invalid operand status %d", oper.status);

        Opr_Kind kind = OPR_DIRECT;

        if (symbol->expr.type == EXPR_REG) kind = OPR_REGISTER;
        if (symbol->expr.type == EXPR_FUNCALL && symbol->expr.value.funcall->args->value.type == EXPR_REG) {
            if (sv_compare(symbol->expr.value.funcall->name, STR("val"))) {
                kind = OPR_REGISTER_INDIRECT;
            } else if (sv_compare(symbol->expr.value.funcall->name, STR("ref"))) {
                kind = OPR_REGISTER;
            }
        }

        memcpy(&sasm $code[symbol->addr], &kind, sizeof(kind));
        memcpy(&sasm $code[symbol->addr + sizeof(kind)], &oper.value, sizeof(oper.value));
    }

    sasm->scope = savedScope;
    return true;
ret_err:
    return false;
}

Binding* sasm_global_find_binding(Sasm_Context* sasm, String_View name)
{
    for (Scope* scope = sasm->scope; scope != NULL; scope = scope->previous) {
        Binding* binding = sasm_local_find_binding(scope, name);
        if (binding)
            return binding;
    }

    return NULL;
}

EvalResult sasm_resolve_binding(Sasm_Context* sasm, Binding* binding)
{
    switch (binding->status) {
    case BIND_STATUS_UNEVALUATED:
        binding->status   = BIND_STATUS_EVALUATING;
        EvalResult result = sasm_resolve_expr(sasm, binding->expr, binding->location);
        binding->status   = BIND_STATUS_EVALUATED;

        if (result.status == EVAL_STATUS_OK) {
            binding->type  = result.type;
            binding->value = result.value;
        }

        return result;
    case BIND_STATUS_EVALUATING:
        log(FLFmt ": ERROR: cycling binding definition.\n", FLArg(binding->location));
        return (EvalResult) { .status = EVAL_ERR };
    case BIND_STATUS_EVALUATED: return resultOK(binding->value, binding->type);
    case BIND_STATUS_DEFERRED:  return resultUnresolved(binding);
    default:                    return (EvalResult) { .status = EVAL_ERR };
    }
}

bool sasm_resolve_entry_point(Sasm_Context* sasm)
{
    Scope* savedScope = sasm->scope;

    if (sasm->deferredEntry.bindingName.len <= 0)
        goto ret_ok;

    try(sasm->deferredEntry.scope, "INVALID SCOPE!", "");
    try(!sasm->hasEntry, FLFmt ": ERROR: entry point has been already set!\n" FLFmt ": NOTE: the first entry point\n",
        FLArg(sasm->deferredEntry.location), FLArg(sasm->entryLocation));

    sasm->scope = sasm->deferredEntry.scope;
    Binding* binding = sasm_global_find_binding(sasm, sasm->deferredEntry.bindingName);

    try(binding != NULL, FLFmt ": ERROR: unknown binding `%.*s`\n",
        FLArg(sasm->deferredEntry.location), Str_Fmt(sasm->deferredEntry.bindingName));
    try(binding->type == BIND_TYPE_INST_ADDR, FLFmt ": ERROR: Type check error. Trying to set `%.*s` that has the type of %s as an entry point. Entry point has to be %s.\n",
        FLArg(sasm->deferredEntry.location), Str_Fmt(binding->name), nameof_bind_type(binding->type), nameof_bind_type(BIND_TYPE_INST_ADDR));

    EvalResult result = sasm_resolve_binding(sasm, binding);
    try(result.status == EVAL_STATUS_OK, "Unable to resolve entry point!", "");

    sasm->entry         = result.value.u32;
    sasm->hasEntry      = true;
    sasm->entryLocation = sasm->deferredEntry.location;

ret_ok:
    sasm->scope = savedScope;
    return true;
ret_err:
    sasm->scope = savedScope;
    return false;
}

bool sasm_translate_root_file(Sasm_Context* sasm, String_View input_file_data)
{
    sasm_scope_push(sasm);
    try(sasm_translate_file(sasm, input_file_data, STR("src")), "Unable to translate root/src file", "");
    sasm_scope_pop(sasm);
    try(sasm_resolve_operands(sasm), "unable to resolve operands", "");
    try(sasm_resolve_entry_point(sasm), "unable to resolve entry point", "");
    return true;
ret_err:
    return false;
}

void sasm_generate_executable(Sasm_Executable* exec, Sasm_Context* sasm)
{
    *exec = (Sasm_Executable) {
        .meta = {
                 .magic        = FILE_MAGIC,
                 .version      = FILE_VERSION,
                 .entry        = sasm->entry,
                 .prog_size    = sasm $code_size,
                 .mem_size     = sasm->mem_size,
                 .mem_capacity = sasm->mem_capacity,
                 },
        .prog = {
                 .code_size = sasm $code_size,
                 }
    };

    for (uint32 i = 0; i < sasm->mem_size; i++) {
        exec->memory[i] = sasm->memory[i];
    }

    for (DataEntry i = 0; i < sasm $code_size; i++) {
        exec $code[i] = sasm $code[i];
    }
}

void sasm_assemble(Sasm_Executable* exec, String_View input_prog)
{
    Sasm_Context* sasm = malloc(sizeof(Sasm_Context));     // TODO: shouldnt use malloc here
    sasm_translate_root_file(sasm, input_prog);
    sasm_generate_executable(exec, sasm);
}

bool sasm_codeblock_push(Arena* arena, CodeBlock* list, Stmt statement)
{
    try(list, "got null ptr!", "");
    ResultPtr space = region_alloc(arena, sizeof(StmtNode));
    try(RESULT_OK(space), "out of space!", "");
    StmtNode* stmtNode  = (StmtNode*)RESULT_VAL(space);
    stmtNode->statement = statement;

    if (list->end == NULL) {
        try(list->begin == NULL, "expected empty list!", "");
        list->begin = stmtNode;
        list->end   = stmtNode;
    } else {
        try(list->begin != NULL, "expected non-empty list!", "");
        list->end->next = stmtNode;
        list->end       = stmtNode;
    }
    list->end->next = NULL;
    return true;
ret_err:
    return false;
}

bool sasm_tokenizer_expect_none(Tokenizer* tokenizer, FileLocation location)
{
    Token token = { 0 };
    try(false == sasm_tokenizer_consume(tokenizer, &token, location),
        FLFmt ": ERROR: unexpected token `%.*s`\n",
        FLArg(location), Str_Fmt(token.text));
    return true;
ret_err:
    return false;
}

bool sasm_parse_directive(Arena* arena, SasmLexer* lineInterpreter, CodeBlock* output)
{
    Line line = { 0 };

    if (!sasm_lexer_consume(lineInterpreter, &line) || line.kind != LINE_DIRECTIVE) {
        err(FLFmt ": ERROR: expected a directive line\n",
            FLArg(lineInterpreter->location));
    }

    FileLocation location = line.location;
    String_View name      = line.value.directive.name;
    String_View body      = line.value.directive.body;

    if (sv_compare(name, STR("include"))) {
        Stmt statement     = { 0 };
        statement.location = location;
        statement.kind     = STMT_INCLUDE;
        Expr path          = sasm_parse_expr(arena, body, line.location);

        try(path.type == EXPR_LIT_STR, FLFmt "ERROR: expected string literal as path for %%include directive\n",
            FLArg(location));

        statement.value.include.path = path.value.lit_str;
        // statement.value.include.content = readFile();

        return sasm_codeblock_push(arena, output, statement);
    }
    if (sv_compare(name, STR("bind"))) {
        Stmt statement      = { 0 };
        statement.location  = location;
        statement.kind      = STMT_CONST;

        Tokenizer tokenizer = sasm_tokenizer_load_sv(body);
        Expr bindingName    = sasm_tokenizer_parse_expr(arena, &tokenizer, location);
        try(bindingName.type == EXPR_BINDING, FLFmt ": ERROR: expected binding name for %%bind binding\n",
            FLArg(location));

        statement.value.constant.name  = bindingName.value.binding;

        statement.value.constant.value = sasm_tokenizer_parse_expr(arena, &tokenizer, location);
        sasm_tokenizer_expect_none(&tokenizer, location);

        return sasm_codeblock_push(arena, output, statement);
    }
    if (sv_compare(name, STR("entry"))) {
        body              = sv_trim(body);
        bool inline_entry = false;

        if (sv_ends_with(body, STR(":"))) {
            sv_split_by_len_reversed(&body, 1);
            inline_entry = true;
        }

        Expr expr                   = sasm_parse_expr(arena, body, line.location);

        Stmt statement              = { 0 };
        statement.location          = location;
        statement.kind              = STMT_ENTRY;
        statement.value.entry.value = expr;
        try(sasm_codeblock_push(arena, output, statement), "unable to push into codeblock", "");

        if (inline_entry) {
            Stmt statement     = { 0 };
            statement.location = location;
            statement.kind     = STMT_LABEL;

            if (expr.type != EXPR_BINDING) {
                printf(FLFmt ": ERROR: expected binding name for a label\n",
                    FLArg(location));
            }

            statement.value.label.name = expr.value.binding;
            return sasm_codeblock_push(arena, output, statement);
        }
        return true;
    }
    if (sv_compare(name, STR("scope"))) {
        Stmt statement        = { 0 };
        statement.location    = location;
        statement.kind        = STMT_SCOPE;
        statement.value.scope = sasm_parse_lines(arena, lineInterpreter).begin;

        if (!sasm_lexer_consume(lineInterpreter, &line) || line.kind != LINE_DIRECTIVE || !sv_compare(line.value.directive.name, STR("end"))) {
            err(FLFmt ": ERROR: expected `%%end` directive at the end of the `%%scope` block\n" FLFmt ": NOTE: the %%scope block starts here\n",
                FLArg(lineInterpreter->location), FLArg(statement.location));
        }
        return sasm_codeblock_push(arena, output, statement);
    }

    err(FLFmt ": ERROR: unknown directive `%.*s`\n",
        FLArg(line.location), Str_Fmt(name));

ret_err:
    return false;
}

CodeBlock sasm_parse_lines(Arena* arena, SasmLexer* lineInterpreter)
{
    CodeBlock result = { 0 };
    result.begin     = NULL;
    result.end       = NULL;

    Line line        = { 0 };
    while (sasm_lexer_seek(lineInterpreter, &line)) {
        const FileLocation location = line.location;
        Stmt statement              = { 0 };
        statement.location          = location;

        switch (line.kind) {
        case LINE_INSTRUCTION:
            {
                String_View name        = line.value.instruction.name;
                String_View operandList = line.value.instruction.operand;

                OpcodeDetails details;
                try(opcode_get_details_from_name(name, &details), "Unknown instruction detected! %.*s", Str_Fmt(name));

                statement.kind            = STMT_INST;
                statement.value.inst.type = details.type;

                for (uint8 i = 0; i < details.operand_cnt; ++i) {
                    operandList = sv_trim(operandList);
                    String_View opr = sv_trim(sv_split_by_delim(&operandList, ' '));
                    statement.value.inst.operands[i] = sasm_parse_expr(arena, opr, location);
                }
                try(sasm_codeblock_push(arena, &result, statement), "Unable to push into codeblock!", "");
                sasm_lexer_consume(lineInterpreter, NULL);
            }
        break; case LINE_LABEL:
            {
                Expr label = sasm_parse_expr(arena, line.value.label.name, location);

                try(label.type == EXPR_BINDING, FLFmt ": ERROR: expected binding name for a label \n",
                    FLArg(location));

                statement.kind             = STMT_LABEL;
                statement.value.label.name = label.value.binding;
                try(sasm_codeblock_push(arena, &result, statement), "Unable to push into codeblock!", "");
                sasm_lexer_consume(lineInterpreter, NULL);
            }
        break; case LINE_DIRECTIVE:
            if (sv_compare(line.value.directive.name, STR("end"))) {
                return result;
            }

            try(sasm_parse_directive(arena, lineInterpreter, &result), "Unable to parse directive!", "");
            continue;
        }
    }

ret_err:
    return result;
}

unsigned long long strtoull(const char* nptr, char** endptr, int base)
{
    const char* s             = nptr;
    unsigned long long result = 0;

    while (*s == ' ' || *s == '\t' || *s == '\n')
        s++;

    if (*s == '+')
        s++;

    if (base == 0) {
        if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
            base = 16;
            s += 2;
        } else {
            base = 10;
        }
    }

    while (*s) {
        unsigned digit;

        if (*s >= '0' && *s <= '9')
            digit = *s - '0';
        else if (*s >= 'a' && *s <= 'f')
            digit = 10 + (*s - 'a');
        else if (*s >= 'A' && *s <= 'F')
            digit = 10 + (*s - 'A');
        else
            break;

        if (digit >= (unsigned)base)
            break;

        result = result * base + digit;
        s++;
    }

    if (endptr)
        *endptr = (char*)s;

    return result;
}

double strtod(const char* nptr, char** endptr)
{
    const char* s   = nptr;
    double result   = 0.0;
    double fraction = 0.0;
    double divisor  = 1.0;
    int negative    = 0;

    while (*s == ' ' || *s == '\t' || *s == '\n')
        s++;

    if (*s == '-') {
        negative = 1;
        s++;
    } else if (*s == '+') {
        s++;
    }

    while (*s >= '0' && *s <= '9') {
        result = result * 10.0 + (*s - '0');
        s++;
    }

    if (*s == '.') {
        s++;
        while (*s >= '0' && *s <= '9') {
            fraction = fraction * 10.0 + (*s - '0');
            divisor *= 10.0;
            s++;
        }
    }

    result += fraction / divisor;

    if (negative)
        result = -result;

    if (endptr)
        *endptr = (char*)s;

    return result;
}

Expr sasm_tokenizer_parse_num(Arena* arena, Tokenizer* tokenizer, FileLocation location)
{
    Token token = { 0 };
    Expr result = { 0 };

    try(sasm_tokenizer_consume(tokenizer, &token, location), FLFmt ": ERROR: Cannot parse empty expression\n",
        FLArg(location));

    if (token.type == TOKEN_TYPE_NUMBER) {
        String_View text = token.text;

        const char* cstr = arena_sv_to_cstr(arena, text);
        char* endptr     = 0;

        if (sv_starts_with(text, STR("0x"))) {
            result.value.lit_int = strtoull(cstr, &endptr, 16);
            try((uint32)(endptr - cstr) == text.len, FLFmt ": ERROR: `%.*s` is not a hex literal\n",
                FLArg(location), Str_Fmt(text));

            result.type = EXPR_LIT_INT;
        } else {
            result.value.lit_int = strtoull(cstr, &endptr, 10);
            if ((uint32)(endptr - cstr) != text.len) {
                result.value.lit_float = strtod(cstr, &endptr);
                if ((uint32)(endptr - cstr) != text.len) {
                    printf(FLFmt ": ERROR: `%.*s` is not a number literal\n",
                        FLArg(location), Str_Fmt(text));
                } else {
                    result.type = EXPR_LIT_FLOAT;
                }
            } else {
                result.type = EXPR_LIT_INT;
            }
        }
    } else {
        err(FLFmt ": ERROR: expected %s but got %s",
            FLArg(location),
            nameof_token(TOKEN_TYPE_NUMBER),
            nameof_token(token.type));
    }
ret_err:
    return result;
}

String_View sasm_tokenizer_parse_sv(Tokenizer* tokenizer, FileLocation location)
{
    Token token = { 0 };
    if (!sasm_tokenizer_consume(tokenizer, &token, location) || token.type != TOKEN_TYPE_STR) {
        err(FLFmt ": ERROR: expected token %s\n",
            FLArg(location), nameof_token(TOKEN_TYPE_STR));
    }

    return token.text;

ret_err:
    return (String_View) { 0 };
}

Expr sasm_tokenizer_parse_expr(Arena* arena, Tokenizer* tokenizer, FileLocation location)
{
    Token token = { 0 };
    Expr result = { 0 };

    try(sasm_tokenizer_seek(tokenizer, &token, location), FLFmt ": ERROR: Cannot parse empty expression\n",
        FLArg(location));

    switch (token.type) {
    case TOKEN_TYPE_STR:
        result.type          = EXPR_LIT_STR;
        result.value.lit_str = sasm_tokenizer_parse_sv(tokenizer, location);
    break; case TOKEN_TYPE_CHAR:
        sasm_tokenizer_consume(tokenizer, NULL, location);

        try(token.text.len == 1, FLFmt ": ERROR: the length of char literal has to be exactly one\n",
            FLArg(location));

        result.type           = EXPR_LIT_CHAR;
        result.value.lit_char = token.text.data[0];
    break; case TOKEN_TYPE_NAME:
        sasm_tokenizer_consume(tokenizer, NULL, location);

        Token next = { 0 };
        if (sasm_tokenizer_seek(tokenizer, &next, location) && next.type == TOKEN_TYPE_OPEN_PAREN) {
            ResultPtr space = region_alloc(arena, sizeof(Funcall));
            try(RESULT_OK(space), "out of space!", "");
            result.type                = EXPR_FUNCALL;
            result.value.funcall       = (Funcall*)RESULT_VAL(space);
            result.value.funcall->name = token.text;
            result.value.funcall->args = sasm_parse_funcall_arglist(arena, tokenizer, location);
        } else {
            result.value.binding = token.text;
            result.type          = EXPR_BINDING;
        }
    break; case TOKEN_TYPE_NUMBER: return sasm_tokenizer_parse_num(arena, tokenizer, location);
    case TOKEN_TYPE_OPEN_PAREN:
        sasm_tokenizer_consume(tokenizer, NULL, location);
        Expr expr = sasm_tokenizer_parse_expr(arena, tokenizer, location);

        try(sasm_tokenizer_consume(tokenizer, &token, location) && token.type == TOKEN_TYPE_CLOSING_PAREN, FLFmt ": ERROR: expected `%s`\n",
            FLArg(location), nameof_token(TOKEN_TYPE_CLOSING_PAREN));
    return expr; case TOKEN_TYPE_REGISTER:
        sasm_tokenizer_consume(tokenizer, NULL, location);
        String_View str = token.text;
        switch (str.data[0]) {
        case 'U':
            try(str.data[1] >= '0' && str.data[1] <= '9', FLFmt ": ERROR: Invalid register %s\n", FLArg(location), str.data);
            result.value.reg_id = (uint32)(REG_U0 + str.data[1] - '0');
        break; case 'S':
            try(str.data[1] >= '0' && str.data[1] <= '6', FLFmt ": ERROR: Invalid register %s\n", FLArg(location), str.data);
            result.value.reg_id = (uint32)(REG_S0 + str.data[1] - '0');
        break; default:     // IP, SP not allowed
            err(FLFmt ": ERROR: Invalid register %s\n",
                FLArg(location), str.data);
        };

        result.type = EXPR_REG;
    break; case TOKEN_TYPE_COMMA:
    case TOKEN_TYPE_CLOSING_PAREN:
        err(FLFmt ": ERROR: expected primary expression but found %s\n", FLArg(location), nameof_token(token.type));
    }

ret_err:
    return result;
}

bool sasm_funcall_expect_arity(Funcall* funcall, uint32 expected_arity, FileLocation location)
{
    const uint32 actual_arity = sasm_funcall_resolve_arg_cnt(funcall->args);
    try(actual_arity == expected_arity,
        FLFmt ": ERROR: %.*s() expects %" PRIu64 " but got %" PRIu64,
        FLArg(location), Str_Fmt(funcall->name), expected_arity, actual_arity);
    return true;
ret_err:
    return false;
}

EvalResult sasm_resolve_funcall(Sasm_Context* sasm, Expr expr, FileLocation location)
{
    EvalResult result = { 0 };
    if (sv_compare(expr.value.funcall->name, STR("len"))) {
        try(sasm_funcall_expect_arity(expr.value.funcall, 1, location), "Incorrect arity! Expected %d arg(s).", 1);

        QuadWord addr = { 0 };
        result        = sasm_resolve_expr(sasm, expr.value.funcall->args->value, location);
        if (result.status == EVAL_STATUS_DEFERRED)
            return result;

        addr            = result.value;
        QuadWord length = { 0 };
        try(sasm_resolve_strlen(sasm, addr.u32, &length), FLFmt ": ERROR: Could not compute the length of string at address %" PRIu64 "\n",
            FLArg(location), addr.u32);
        return resultOK(length, BIND_TYPE_UINT);
    }
    if (sv_compare(expr.value.funcall->name, STR("res"))) {
        try(sasm_funcall_expect_arity(expr.value.funcall, 1, location), "Incorrect arity! Expected %d arg(s).", 1);

        QuadWord addr = { 0 };
        result        = sasm_resolve_expr(sasm, expr.value.funcall->args->value, location);

        try(sasm->mem_size + result.value.u32 <= MAX_MEMORY_CAPACITY, "memory cap excedeed!", "");

        addr = quadwordFromU64(sasm->mem_size);
        sasm->mem_size += result.value.u32;

        if (sasm->mem_size > sasm->mem_capacity) {
            sasm->mem_capacity = sasm->mem_size;
        }

        return resultOK(addr, BIND_TYPE_UINT);
    }
    if (sv_compare(expr.value.funcall->name, STR("ref")) || sv_compare(expr.value.funcall->name, STR("val"))) {
        // intent (register direct/indirect) is determined while resolving operands, to determine the Opr_Kind
        try(sasm_funcall_expect_arity(expr.value.funcall, 1, location), "Incorrect arity! Expected %d arg(s).", 1);
        try(expr.value.funcall->args->value.type == EXPR_REG, FLFmt ": ERROR: ref expects a register ", FLArg(location));

        result = sasm_resolve_expr(sasm, expr.value.funcall->args->value, location);
        return result;
    }
    err(FLFmt ": ERROR: Unknown translation time function `%.*s`\n",
        FLArg(location), Str_Fmt(expr.value.funcall->name));
ret_err:
    result = (EvalResult) { .status = EVAL_ERR };
    return result;
}

QuadWord sasm_mempush_str(Sasm_Context* sasm, String_View str)
{
    try(sasm->mem_size + str.len <= MAX_MEMORY_CAPACITY, "Memory capacity exceeded when trying to declare string!", "");

    QuadWord result = quadwordFromU64(sasm->mem_size);
    memcpy(sasm->memory + sasm->mem_size, str.data, str.len);
    sasm->mem_size += str.len;

    if (sasm->mem_size > sasm->mem_capacity) {
        sasm->mem_capacity = sasm->mem_size;
    }

    sasm->stringLens[sasm->strLensCnt++] = (StringLength) {
        .addr = result.u32,
        .len  = str.len,
    };

    return result;
ret_err:
    return (QuadWord) { 0 };
}

EvalResult sasm_resolve_expr(Sasm_Context* sasm, Expr expr, FileLocation location)
{
    EvalResult res = { 0 };
    switch (expr.type) {
    case EXPR_LIT_INT:          res = resultOK(quadwordFromU64(expr.value.lit_int), BIND_TYPE_UINT);
    break; case EXPR_LIT_FLOAT: res = resultOK(quadwordFromF64(expr.value.lit_float), BIND_TYPE_FLOAT);
    break; case EXPR_LIT_CHAR:  res = resultOK(quadwordFromU64(expr.value.lit_char), BIND_TYPE_UINT);
    break; case EXPR_LIT_STR:   res = resultOK(sasm_mempush_str(sasm, expr.value.lit_str), BIND_TYPE_MEM_ADDR);
    break; case EXPR_FUNCALL:   res = sasm_resolve_funcall(sasm, expr, location);
    break; case EXPR_REG:       res = resultOK(quadwordFromU64(expr.value.reg_id), BIND_TYPE_UINT);
    break; case EXPR_BINDING: {
            String_View name = expr.value.binding;
            Binding* binding = sasm_global_find_binding(sasm, name);
            try(binding != NULL, FLFmt ": ERROR: couldnt find binding `%.*s`.\n",
                FLArg(location), Str_Fmt(name));

            res = sasm_resolve_binding(sasm, binding);
        }
    }
ret_err:
    return res;
}

bool sasm_scope_bind_expr(Scope* scope, String_View name, Expr expr, FileLocation location)
{
    try(scope->bindingsCnt < BINDINGS_CAPACITY, "max binding capacity exceeded!", "");

    Binding* existing = sasm_local_find_binding(scope, name);
    try(existing == NULL, FLFmt ": ERROR: name `%.*s` is already bound\n" FLFmt ": NOTE: first binding is located here\n",
        FLArg(location), Str_Fmt(name), FLArg(existing->location));

    scope->bindings[scope->bindingsCnt++] = (Binding) {
        .name     = name,
        .expr     = expr,
        .location = location,
    };
    return true;
ret_err:
    return false;
}

bool sasm_translate_bind_directive(Sasm_Context* sasm, ConstStmt konst, FileLocation location)
{
    try(sasm->scope != NULL, "No Scope to bind to!", "");
    return sasm_scope_bind_expr(sasm->scope, konst.name, konst.value, location);
ret_err:
    return false;
}

bool sasm_translate_incl_directive(Sasm_Context* sasm, IncludeStmt include, FileLocation location)
{
    /*
        String_View resolved_path = (String_View) { 0 };
        if (resolveIncludeFilePath(sasm, include.path, &resolved_path)) {
            include.path = resolved_path;
        }
    */
    FileLocation prev_includeLocation = sasm->includeLocation;
    sasm->includeLevel += 1;
    sasm->includeLocation = location;
    try(sasm_translate_file(sasm, include.content, include.path), "Unable to include file", "");
    sasm->includeLocation = prev_includeLocation;
    sasm->includeLevel -= 1;
    return true;
ret_err:
    return false;
}

bool sasm_translate_entr_directive(Sasm_Context* sasm, EntryStmt entry, FileLocation location)
{
    try(sasm->scope, "no scope found for entry directive", "");

    try(sasm->deferredEntry.bindingName.len <= 0,
        FLFmt ": ERROR: entry point has been already set within the same scope!\n" FLFmt ": NOTE: the first entry point\n",
        FLArg(location),
        FLArg(sasm->deferredEntry.location));

    try(entry.value.type == EXPR_BINDING,
        FLFmt ": ERROR: only bindings are allowed to be set as entry points for now.\n",
        FLArg(location));

    String_View label               = entry.value.value.binding;
    sasm->deferredEntry.bindingName = label;
    sasm->deferredEntry.location    = location;
    sasm->deferredEntry.scope       = sasm->scope;
    return true;
ret_err:
    return false;
}

bool sasm_translate_inst(Sasm_Context* sasm, InstStmt inst, FileLocation location)
{
    OpcodeDetails details = opcode_get_details(inst.type);

    uint32 operand_size = sizeof(struct Operand);
    uint32 bytes_needed = sizeof(Opcode) + details.operand_cnt * operand_size;

    try(sasm $code_size + bytes_needed <= MAX_PROGRAM_CAPACITY, "Max program capacity exceeded!", "");
    sasm $code[sasm $code_size++] = (Opcode)inst.type;

    for (uint8 i = 0; i < details.operand_cnt; i++) {
        uint32 offset = sasm $code_size;
        sasm $code_size += operand_size;

        try(i < MAX_OPERAND_CNT, "The operand count specified for this instruction is greater than the max allowed! (MAX_OPERAND_CNT=%d)", MAX_OPERAND_CNT);
        try(sasm_defer_operand(sasm, offset, inst.operands[i], location), "Unable to defer operand!", "");
    }

    return true;
ret_err:
    return false;
}

bool sasm_local_defr_binding(Scope* scope, String_View name, BindingType type, FileLocation location)
{
    try(scope->bindingsCnt < BINDINGS_CAPACITY, "Binding cap exceeded!", "");

    Binding* existing = sasm_local_find_binding(scope, name);
    try(existing == NULL,
        FLFmt ": ERROR: name `%.*s` is already bound\n" FLFmt ": NOTE: first binding is located here\n",
        FLArg(location),
        Str_Fmt(name),
        FLArg(existing->location));

    scope->bindings[scope->bindingsCnt++] = (Binding) {
        .name      = name,
        .status    = BIND_STATUS_DEFERRED,
        .type      = type,
        .location  = location,
        .value.u32 = 0
    };

    return true;
ret_err:
    return false;
}

bool sasm_translate_stmt_chain(Sasm_Context* sasm, StmtNode* block)
{
    for (StmtNode* iter = block; iter != NULL; iter = iter->next) {
        Stmt statement = iter->statement;
        switch (statement.kind) {
        case STMT_LABEL:            try(sasm_local_defr_binding(sasm->scope, statement.value.label.name, BIND_TYPE_INST_ADDR, statement.location), "Unable to bind!", "");
        break; case STMT_CONST:     try(sasm_translate_bind_directive(sasm, statement.value.constant, statement.location), "Unable to bind!", "");
        break; case STMT_INCLUDE:   try(sasm_translate_incl_directive(sasm, statement.value.include, statement.location), "Unable to bind!", "");
        break; case STMT_ENTRY:     try(sasm_translate_entr_directive(sasm, statement.value.entry, statement.location), "Unable to bind!", "");
        break; case STMT_BLOCK:     try(sasm_translate_stmt_chain(sasm, statement.value.block), "Unable to bind!", "");    // Currently unused! 
        break; case STMT_SCOPE:
        case STMT_INST:
        }
    }

    for (StmtNode* iter = block; iter != NULL; iter = iter->next) {
        Stmt statement = iter->statement;
        switch (statement.kind) {
        case STMT_INST: 
            try(sasm_translate_inst(sasm, statement.value.inst, statement.location), "Unable to bind!", "");
        break; case STMT_LABEL: 
            {
                Binding* binding = sasm_global_find_binding(sasm, statement.value.label.name);
                try(binding != NULL, "binding not found: %s", statement.value.label.name.data);
                try(binding->status == BIND_STATUS_DEFERRED, "binding already defined!", "");

                binding->status    = BIND_STATUS_EVALUATED;
                binding->value.u32 = sasm $code_size;
            }
        break; case STMT_SCOPE:
            sasm_scope_push(sasm);
            try(sasm_translate_stmt_chain(sasm, statement.value.scope), "Unable to translate scope", "");
            sasm_scope_pop(sasm);
        break; case STMT_BLOCK:
        case STMT_ENTRY:
        case STMT_INCLUDE:
        case STMT_CONST:
        }
    }
    return true;
ret_err:
    return false;
}

bool sasm_translate_file(Sasm_Context* sasm, String_View inputFileData, String_View inputFilePath)
{
    SasmLexer SasmLexer = { 0 };

    if (!sasm_lexer_read_file(&SasmLexer, inputFileData, inputFilePath)) {
        // todo: maybe file handling?
        if (sasm->includeLevel > 0) {
            err(FLFmt ": Could not read file", FLArg(sasm->includeLocation));
        } else {
            err("Could not read file", "");
        }
    }

    CodeBlock inputFileBlock = sasm_parse_lines(&sasm->arena, &SasmLexer);
    return sasm_translate_stmt_chain(sasm, inputFileBlock.begin);
ret_err:
    return false;
}

bool sasm_defer_operand(Sasm_Context* sasm, InstAddr addr, Expr expr, FileLocation location)
{
    try(sasm->symbolsCount < LABELS_CAPACITY, "Label cap exceeded!", "");
    sasm->symbols[sasm->symbolsCount++] = (UnresolvedOperand) {
        .addr     = addr,
        .expr     = expr,
        .location = location,
        .scope    = sasm->scope
    };
    return true;
ret_err:
    return false;
}

bool sasm_add_include_path(Sasm_Context* sasm, const char* path)
{
    try(sasm->includePathsCnt < INCLUDE_PATHS_CAPACITY, "Include path cap exceeded!", "");
    sasm->includePaths[sasm->includePathsCnt++] = STR(path);
    return true;
ret_err:
    return false;
}

bool sasm_tokenizer_seek(Tokenizer* tokenizer, Token* output, FileLocation location)
{
    if (tokenizer->hasCachedToken) {
        if (output) {
            *output = tokenizer->cachedToken;
        }
        return true;
    }

    tokenizer->source = sv_trim_left(tokenizer->source);

    if (tokenizer->source.len == 0) {
        return false;
    }

    Token token = { 0 };
    switch (*tokenizer->source.data) {
    case '(': {
            token.type = TOKEN_TYPE_OPEN_PAREN;
            token.text = sv_split_by_len(&tokenizer->source, 1);
    } break; case ')': {
            token.type = TOKEN_TYPE_CLOSING_PAREN;
            token.text = sv_split_by_len(&tokenizer->source, 1);
    } break; case ',': {
            token.type = TOKEN_TYPE_COMMA;
            token.text = sv_split_by_len(&tokenizer->source, 1);
    } break; case '"': {
            sv_split_by_len(&tokenizer->source, 1);
            uint64 index = 0;

            try(sv_index_of(tokenizer->source, '"', &index),
                FLFmt ": ERROR: Could not find closing \"\n", FLArg(location));
            String_View text = sv_split_by_len(&tokenizer->source, index);
            sv_split_by_len(&tokenizer->source, 1);
            token.type = TOKEN_TYPE_STR;
            token.text = text;
    } break; case '\'': {
            sv_split_by_len(&tokenizer->source, 1);
            uint64 index = 0;

            try(sv_index_of(tokenizer->source, '\'', &index), FLFmt ": ERROR: Could not find closing \'\n", FLArg(location));
            String_View text = sv_split_by_len(&tokenizer->source, index);
            sv_split_by_len(&tokenizer->source, 1);
            token.type = TOKEN_TYPE_CHAR;
            token.text = text;
    } break; default: {
            if (isalphabet(*tokenizer->source.data)) {
                token.type = TOKEN_TYPE_NAME;
                token.text = sv_split_by_condition(&tokenizer->source, isName);
            } else if (is_digit(*tokenizer->source.data) || *tokenizer->source.data == '-') {
                token.type = TOKEN_TYPE_NUMBER;
                token.text = sv_split_by_condition(&tokenizer->source, isNumber);
            } else if (tokenizer->source.len >= 3 && *tokenizer->source.data == '[' && tokenizer->source.data[3] == ']') {
                try(tokenizer->source.len >= 4, FLFmt ": ERROR: Check register name %c\n", FLArg(location), *tokenizer->source.data);
                sv_split_by_len(&tokenizer->source, 1);
                uint64 index = 0;
                try(sv_index_of(tokenizer->source, ']', &index), FLFmt ": ERROR: Could not find closing \'\n", FLArg(location));
                token.type = TOKEN_TYPE_REGISTER;
                token.text = sv_split_by_len(&tokenizer->source, index);
                sv_split_by_len(&tokenizer->source, 1);
            } else {
                err(FLFmt ": ERROR: Unknown token starts with %c\n", FLArg(location), *tokenizer->source.data);
            }
        }
    }

    tokenizer->hasCachedToken = true;
    tokenizer->cachedToken    = token;

    if (output) {
        *output = token;
    }

    return true;
ret_err:
    return false;
}

FuncallArg* sasm_parse_funcall_arglist(Arena* arena, Tokenizer* tokenizer, FileLocation location)
{
    Token token = { 0 };

    try(sasm_tokenizer_consume(tokenizer, &token, location) && token.type == TOKEN_TYPE_OPEN_PAREN,
        FLFmt ": ERROR: expected %s\n", FLArg(location),
        nameof_token(TOKEN_TYPE_OPEN_PAREN));

    if (sasm_tokenizer_seek(tokenizer, &token, location) && token.type == TOKEN_TYPE_CLOSING_PAREN) {
        sasm_tokenizer_consume(tokenizer, NULL, location);
        return NULL;
    }

    FuncallArg* first = NULL;
    FuncallArg* last  = NULL;

    do {
        ResultPtr space = region_alloc(arena, sizeof(FuncallArg));
        try(RESULT_OK(space), "out of space!", "");
        FuncallArg* arg = (FuncallArg*)RESULT_VAL(space);
        arg->value      = sasm_tokenizer_parse_expr(arena, tokenizer, location);

        if (first == NULL) {
            first = arg;
            last  = arg;
        } else {
            last->next = arg;
            last       = arg;
        }

        try(sasm_tokenizer_consume(tokenizer, &token, location),
            FLFmt ": ERROR: expected %s or %s\n", FLArg(location),
            nameof_token(TOKEN_TYPE_CLOSING_PAREN),
            nameof_token(TOKEN_TYPE_COMMA));
    } while (token.type == TOKEN_TYPE_COMMA);

    try(token.type == TOKEN_TYPE_CLOSING_PAREN,
        FLFmt ": ERROR: expected %s\n", FLArg(location), nameof_token(TOKEN_TYPE_CLOSING_PAREN));

    return first;
ret_err:
    return NULL;
}

QuadWord quadwordFromU64(u32 u64)   {  return (QuadWord) { .u32 = u64 }; }
QuadWord quadwordFromI64(i32 i64)   {  return (QuadWord) { .i32 = i64 }; }
QuadWord quadwordFromF64(f32 f64)   {  return (QuadWord) { .f32 = f64 }; }
QuadWord quadwordFromPtr(void* ptr) {  return (QuadWord) { .ptr = ptr }; }

#endif
