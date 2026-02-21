#ifndef KERN_SASM_1
#define KERN_SASM_1

#define IMPL_SASM_1

#if defined(__clang__)
#pragma clang diagnostic ignored "-Wdollar-in-identifier-extension"
#endif

#include <common/memmanager.h>
#include <common/panic.h>
#include <common/result.h>
#include <common/strings.h>
#include <common/types.h>
#include <userspace/libs/io.h>

#define NULL ((void*)0)

typedef uint8 Byte;
typedef uint16 Word;
typedef uint32 DoubleWord;

typedef uint64 DataEntry;
typedef uint64 InstAddr;
typedef uint64 MemoryAddr;
typedef uint64 StackAddr;
typedef uint64 u64;
typedef int64 i64;
typedef double f64;

typedef union {
    uint64 u64;
    int64 i64;
    double f64;
    void* ptr;
} QuadWord;

#define MAX_MEMORY_CAPACITY 2048      // Max amount of memory initially assigned to a sasm program
#define MAX_PROGRAM_CAPACITY 1024     // Max amount of instructions in any sasm program
#define INTERNAL_VMCALLS_CAPACITY 1024
#define BINDINGS_CAPACITY 1024
#define LABELS_CAPACITY 1024
#define MAX_INCLUDE_LEVEL 10
#define STACK_CAPACITY 1024
#define FILE_MAGIC 0x484f53
#define FILE_VERSION 0x4D41
#define COMMENT_SYMBOL ';'
#define PREP_SYMBOL '%'

#define $instructionCount ->prog.instruction_count
#define $instructions ->prog.instructions
#define DEFERRED_ASSERTS_CAPACITY 1024
#define STRING_LENGTHS_CAPACITY 1024
#define INCLUDE_PATHS_CAPACITY 1024
#define MAX_PRECEDENCE 2
#define Token_Fmt "%s"
#define Token_Arg(token) token_kind_name((token).kind)

#define PRIu64 "llu"
#define PRIi64 "lli"
#define PRId64 "lld"
#define PRIX64 "llX"

// ---------------------------------------------------------------------------------------------------

typedef enum {
    META_HALT = 1 << 0,
    META_F1   = 1 << 1,
    META_F2   = 1 << 2,
    META_F3   = 1 << 3,
    META_F4   = 1 << 4,
    META_F5   = 1 << 5,
    META_F6   = 1 << 6,
    META_F7   = 1 << 7
} Meta;

typedef enum {

    INST_DONOP = 0,
    INST_INVOK,
    INST_RETVL,
    INST_PUSHR,
    INST_SPOPR,
    INST_SHUTS,
    INST_SETR,
    INST_GETR,
    INST_CALL,
    INST_LOOP,
    INST_PUSH,
    INST_SPOP,
    INST_SWAP,
    INST_ADDI,
    INST_SUBI,
    INST_MULI,
    INST_DIVI,
    INST_MODI,
    INST_ADDU,
    INST_SUBU,
    INST_MULU,
    INST_DIVU,
    INST_MODU,
    INST_ADDF,
    INST_SUBF,
    INST_MULF,
    INST_DIVF,
    INST_JMPU,
    INST_JMPC,
    INST_ANDB,
    INST_NOTB,
    INST_COPY,
    INST_DUPS,
    INST_RET,
    INST_NOT,
    INST_EQI,
    INST_GEI,
    INST_GTI,
    INST_LEI,
    INST_LTI,
    INST_NEI,
    INST_EQU,
    INST_GEU,
    INST_GTU,
    INST_LEU,
    INST_LTU,
    INST_NEU,
    INST_EQF,
    INST_GEF,
    INST_GTF,
    INST_LEF,
    INST_LTF,
    INST_NEF,
    INST_ORB,
    INST_XOR,
    INST_SHR,
    INST_SHL,
    INST_I2F,
    INST_U2F,
    INST_F2I,
    INST_F2U,
    INST_READ1U,
    INST_READ2U,
    INST_READ4U,
    INST_READ8U,
    INST_READ1I,
    INST_READ2I,
    INST_READ4I,
    INST_READ8I,
    INST_WRITE1,
    INST_WRITE2,
    INST_WRITE4,
    INST_WRITE8,
    NUMBER_OF_INSTS
} Opcode;

typedef enum {
    REG_H0,
    REG_H1,
    REG_I0,
    REG_I1,
    REG_JS,
    REG_KC,
    REG_L0,
    REG_L1,
    REG_L2,
    REG_L3,
    REG_NX,
    REG_OP,
    REG_P0,
    REG_P1,
    REG_P2,
    REG_P3,
    REG_QT,
    REG_RF,
    REG_SP,
    REG_COUNT,
} RegID;

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

typedef enum {
    ERR_OK = 0,              /**< No error */
    ERR_STACK_OVERFLOW,      /**< Stack overflow error */
    ERR_STACK_UNDERFLOW,     /**< Stack underflow error */
    ERR_DIV_BY_ZERO,         /**< Division by zero error */
    ERR_ILLEGAL_INST,        /**< Illegal instruction error */
    ERR_ILLEGAL_INST_ACCESS, /**< Illegal instruction access error */
    ERR_ILLEGAL_OPERAND,     /**< Illegal operand error */
    ERR_NULL_CALL,           /**< called NULL vmcall */
    ERR_ILLEGAL_MEMORY_ACCESS,
    ERR_NAN,
    ERR_ALREADY_BOUND
} VM_Error;

typedef QuadWord Register;
typedef struct __attribute__((__packed__)) Instruction Instruction;
typedef struct OpcodeDetails OpcodeDetails;
typedef struct Program Program;
typedef struct CPU CPU;
typedef struct Memory Memory;
typedef union Registers Registers;
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

struct Instruction {
    Opcode type;
    QuadWord operand;
    QuadWord operand2;
    bool opr1IsReg;
    bool opr2IsReg;
};

struct OpcodeDetails {
    Opcode type;
    const char* name;
    bool has_operand;
    bool has_operand2;
};

struct Program {
    Instruction instructions[MAX_PROGRAM_CAPACITY]; /**< The array of instructions */
    DataEntry instruction_count;                    /**< The number of instructions in the program */
};

union Registers {
    struct
    {
        Register H0;
        Register H1;

        Register I0;
        Register I1;

        Register JS;
        Register KC;

        Register L0;
        Register L1;
        Register L2;
        Register L3;

        Register NX;
        Register OP;

        Register P0;
        Register P1;
        Register P2;
        Register P3;

        Register QT;
        Register RF;
        Register SP;
    };
    Register reg[REG_COUNT];
};

struct CPU {
    Registers registers;
    volatile short flags;
};

struct Memory {
    QuadWord stack[STACK_CAPACITY];
    Byte memory[MAX_MEMORY_CAPACITY];
};

union ExprValue {
    String_View binding;
    u64 lit_int;
    double lit_float;
    char lit_char;
    String_View lit_str;
    Funcall* funcall;
    u64 reg_id;
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
    Expr operand;
    Expr operand2;
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
    u64 len;
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
    uint64 bindingsCnt;
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
    uint64 bindingCount;

    UnresolvedOperand symbols[LABELS_CAPACITY];
    uint64 symbolsCount;

    DeferredEntry deferredEntry;

    Program prog;

    bool hasEntry;
    InstAddr entry;
    FileLocation entryLocation;

    StringLength stringLens[STRING_LENGTHS_CAPACITY];
    uint64 strLensCnt;

    Byte memory[MAX_MEMORY_CAPACITY];
    uint64 mem_size;
    uint64 mem_capacity;

    Arena arena;

    uint64 includeLevel;
    FileLocation includeLocation;

    String_View includePaths[INCLUDE_PATHS_CAPACITY];
    uint64 includePathsCnt;
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
Sasm_Executable sasm_assemble(String_View input_prog);

#endif

// ---------------------------------------------------------------------------------------------------

#ifdef IMPL_KERN_SASM_1
#undef IMPL_KERN_SASM_1

QuadWord quadwordFromU64(uint64 u64);
QuadWord quadwordFromI64(int64 i64);
QuadWord quadwordFromF64(double f64);
QuadWord quadwordFromPtr(void* ptr);

const char* getNameOfError(const VM_Error*);
void displayStringMessageError(const char*, String_View);
void debugCommentDisplay(String_View*);
void debugMessageDisplay(String_View*);

bool getOpcodeDetailsFromName(String_View name, OpcodeDetails* outPtr);
OpcodeDetails getOpcodeDetails(Opcode type);

void setFlag(Meta f, CPU* cpu, bool state);
bool getFlag(Meta f, const CPU* cpu);

Sasm_Executable sasm_generate_executable(Sasm_Context* sasm);
bool sasm_translate_root_file(Sasm_Context* sasm, String_View input_file_data);
bool sasm_resolve_operands(Sasm_Context* sasm);
bool sasm_resolve_entry_point(Sasm_Context* sasm);
void sasm_scope_push(Sasm_Context* sasm);
void sasm_scope_pop(Sasm_Context* sasm);

bool translateSasmFile(Sasm_Context* sasm, String_View inputFileData, String_View inputFilePath);
void loadSmExecutableIntoSasm(Sasm_Context* sasm, const char* filePath);

bool pushUnresolvedOperand(Sasm_Context* sasm, InstAddr addr, Expr expr, FileLocation location);
bool pushIncludePath(Sasm_Context* sasm, const char* path);
bool bindExprLocalScope(Scope* scope, String_View name, Expr expr, FileLocation location);
Binding* sasm_binding_resolve(Sasm_Context* sasm, String_View name);
EvalResult sasm_binding_eval(Sasm_Context* sasm, Binding* binding);

bool loadSasmFileIntoSasmLexer(SasmLexer* lineInterpreter, String_View file_content, String_View filePath);
bool fetchCachedLineFromSasmLexer(SasmLexer* lineInterpreter, Line* output);
bool moveSasmLexerToNextLine(SasmLexer* lineInterpreter, Line* output);

Expr parsePrimaryOfSasmTokens(Arena* arena, Tokenizer* tokenizer, FileLocation location);
CodeBlock sasm_line_parse_codeblock(Arena* arena, SasmLexer* lineInterpreter);

bool fetchCachedSasmTokenFromSasmTokenizer(Tokenizer* tokenizer, Token* output, FileLocation location);
bool moveSasmTokenizerToNextToken(Tokenizer* tokenizer, Token* token, FileLocation location);
Tokenizer loadStringIntoTokenizer(String_View source);

const char* getNameOfBindType(BindingType type);
const char* getTokenName(TokenType type);

EvalResult resultOK(QuadWord value, BindingType type);
EvalResult resultUnresolved(Binding* unresolvedBinding);

FuncallArg* parseFuncallArgs(Arena* arena, Tokenizer* tokenizer, FileLocation location);
EvalResult evaluateExpression(Sasm_Context* sasm, Expr expr, FileLocation location);

static OpcodeDetails OpcodeDetailsLUT[NUMBER_OF_INSTS] = {
    [INST_DONOP]  = {.type = INST_DONOP,   .name = "DONOP",  .has_operand = 0, .has_operand2 = 0},
    [INST_INVOK]  = { .type = INST_INVOK,  .name = "INVOK",  .has_operand = 1, .has_operand2 = 0},
    [INST_RETVL]  = { .type = INST_RETVL,  .name = "RETVL",  .has_operand = 0, .has_operand2 = 0},
    [INST_PUSHR]  = { .type = INST_PUSHR,  .name = "PUSHR",  .has_operand = 1, .has_operand2 = 0},
    [INST_SPOPR]  = { .type = INST_SPOPR,  .name = "SPOPR",  .has_operand = 1, .has_operand2 = 0},
    [INST_SHUTS]  = { .type = INST_SHUTS,  .name = "SHUTS",  .has_operand = 0, .has_operand2 = 0},
    [INST_SETR]   = { .type = INST_SETR,   .name = "SETR",   .has_operand = 1, .has_operand2 = 1},
    [INST_GETR]   = { .type = INST_GETR,   .name = "GETR",   .has_operand = 1, .has_operand2 = 0},
    [INST_CALL]   = { .type = INST_CALL,   .name = "CALL",   .has_operand = 1, .has_operand2 = 0},
    [INST_LOOP]   = { .type = INST_LOOP,   .name = "LOOP",   .has_operand = 1, .has_operand2 = 1},
    [INST_PUSH]   = { .type = INST_PUSH,   .name = "PUSH",   .has_operand = 1, .has_operand2 = 0},
    [INST_SPOP]   = { .type = INST_SPOP,   .name = "SPOP",   .has_operand = 0, .has_operand2 = 0},
    [INST_SWAP]   = { .type = INST_SWAP,   .name = "SWAP",   .has_operand = 1, .has_operand2 = 0},
    [INST_ADDI]   = { .type = INST_ADDI,   .name = "ADDI",   .has_operand = 0, .has_operand2 = 0},
    [INST_SUBI]   = { .type = INST_SUBI,   .name = "SUBI",   .has_operand = 0, .has_operand2 = 0},
    [INST_MULI]   = { .type = INST_MULI,   .name = "MULI",   .has_operand = 0, .has_operand2 = 0},
    [INST_DIVI]   = { .type = INST_DIVI,   .name = "DIVI",   .has_operand = 0, .has_operand2 = 0},
    [INST_MODI]   = { .type = INST_MODI,   .name = "MODI",   .has_operand = 0, .has_operand2 = 0},
    [INST_ADDU]   = { .type = INST_ADDU,   .name = "ADDU",   .has_operand = 0, .has_operand2 = 0},
    [INST_SUBU]   = { .type = INST_SUBU,   .name = "SUBU",   .has_operand = 0, .has_operand2 = 0},
    [INST_MULU]   = { .type = INST_MULU,   .name = "MULU",   .has_operand = 0, .has_operand2 = 0},
    [INST_DIVU]   = { .type = INST_DIVU,   .name = "DIVU",   .has_operand = 0, .has_operand2 = 0},
    [INST_MODU]   = { .type = INST_MODU,   .name = "MODU",   .has_operand = 0, .has_operand2 = 0},
    [INST_ADDF]   = { .type = INST_ADDF,   .name = "ADDF",   .has_operand = 0, .has_operand2 = 0},
    [INST_SUBF]   = { .type = INST_SUBF,   .name = "SUBF",   .has_operand = 0, .has_operand2 = 0},
    [INST_MULF]   = { .type = INST_MULF,   .name = "MULF",   .has_operand = 0, .has_operand2 = 0},
    [INST_DIVF]   = { .type = INST_DIVF,   .name = "DIVF",   .has_operand = 0, .has_operand2 = 0},
    [INST_JMPU]   = { .type = INST_JMPU,   .name = "JMPU",   .has_operand = 1, .has_operand2 = 0},
    [INST_JMPC]   = { .type = INST_JMPC,   .name = "JMPC",   .has_operand = 1, .has_operand2 = 0},
    [INST_ANDB]   = { .type = INST_ANDB,   .name = "ANDB",   .has_operand = 0, .has_operand2 = 0},
    [INST_NOTB]   = { .type = INST_NOTB,   .name = "NOTB",   .has_operand = 0, .has_operand2 = 0},
    [INST_COPY]   = { .type = INST_COPY,   .name = "COPY",   .has_operand = 1, .has_operand2 = 1},
    [INST_DUPS]   = { .type = INST_DUPS,   .name = "DUPS",   .has_operand = 1, .has_operand2 = 0},
    [INST_RET]    = { .type = INST_RET,    .name = "RET",    .has_operand = 0, .has_operand2 = 0},
    [INST_NOT]    = { .type = INST_NOT,    .name = "NOT",    .has_operand = 0, .has_operand2 = 0},
    [INST_EQI]    = { .type = INST_EQI,    .name = "EQI",    .has_operand = 0, .has_operand2 = 0},
    [INST_GEI]    = { .type = INST_GEI,    .name = "GEI",    .has_operand = 0, .has_operand2 = 0},
    [INST_GTI]    = { .type = INST_GTI,    .name = "GTI",    .has_operand = 0, .has_operand2 = 0},
    [INST_LEI]    = { .type = INST_LEI,    .name = "LEI",    .has_operand = 0, .has_operand2 = 0},
    [INST_LTI]    = { .type = INST_LTI,    .name = "LTI",    .has_operand = 0, .has_operand2 = 0},
    [INST_NEI]    = { .type = INST_NEI,    .name = "NEI",    .has_operand = 0, .has_operand2 = 0},
    [INST_EQU]    = { .type = INST_EQU,    .name = "EQU",    .has_operand = 0, .has_operand2 = 0},
    [INST_GEU]    = { .type = INST_GEU,    .name = "GEU",    .has_operand = 0, .has_operand2 = 0},
    [INST_GTU]    = { .type = INST_GTU,    .name = "GTU",    .has_operand = 0, .has_operand2 = 0},
    [INST_LEU]    = { .type = INST_LEU,    .name = "LEU",    .has_operand = 0, .has_operand2 = 0},
    [INST_LTU]    = { .type = INST_LTU,    .name = "LTU",    .has_operand = 0, .has_operand2 = 0},
    [INST_NEU]    = { .type = INST_NEU,    .name = "NEU",    .has_operand = 0, .has_operand2 = 0},
    [INST_EQF]    = { .type = INST_EQF,    .name = "EQF",    .has_operand = 0, .has_operand2 = 0},
    [INST_GEF]    = { .type = INST_GEF,    .name = "GEF",    .has_operand = 0, .has_operand2 = 0},
    [INST_GTF]    = { .type = INST_GTF,    .name = "GTF",    .has_operand = 0, .has_operand2 = 0},
    [INST_LEF]    = { .type = INST_LEF,    .name = "LEF",    .has_operand = 0, .has_operand2 = 0},
    [INST_LTF]    = { .type = INST_LTF,    .name = "LTF",    .has_operand = 0, .has_operand2 = 0},
    [INST_NEF]    = { .type = INST_NEF,    .name = "NEF",    .has_operand = 0, .has_operand2 = 0},
    [INST_ORB]    = { .type = INST_ORB,    .name = "ORB",    .has_operand = 0, .has_operand2 = 0},
    [INST_XOR]    = { .type = INST_XOR,    .name = "XOR",    .has_operand = 0, .has_operand2 = 0},
    [INST_SHR]    = { .type = INST_SHR,    .name = "SHR",    .has_operand = 0, .has_operand2 = 0},
    [INST_SHL]    = { .type = INST_SHL,    .name = "SHL",    .has_operand = 0, .has_operand2 = 0},
    [INST_I2F]    = { .type = INST_I2F,    .name = "I2F",    .has_operand = 0, .has_operand2 = 0},
    [INST_U2F]    = { .type = INST_U2F,    .name = "U2F",    .has_operand = 0, .has_operand2 = 0},
    [INST_F2I]    = { .type = INST_F2I,    .name = "F2I",    .has_operand = 0, .has_operand2 = 0},
    [INST_F2U]    = { .type = INST_F2U,    .name = "F2U",    .has_operand = 0, .has_operand2 = 0},
    [INST_READ1U] = { .type = INST_READ1U, .name = "READ1U", .has_operand = 0, .has_operand2 = 0},
    [INST_READ2U] = { .type = INST_READ2U, .name = "READ2U", .has_operand = 0, .has_operand2 = 0},
    [INST_READ4U] = { .type = INST_READ4U, .name = "READ4U", .has_operand = 0, .has_operand2 = 0},
    [INST_READ8U] = { .type = INST_READ8U, .name = "READ8U", .has_operand = 0, .has_operand2 = 0},
    [INST_READ1I] = { .type = INST_READ1I, .name = "READ1I", .has_operand = 0, .has_operand2 = 0},
    [INST_READ2I] = { .type = INST_READ2I, .name = "READ2I", .has_operand = 0, .has_operand2 = 0},
    [INST_READ4I] = { .type = INST_READ4I, .name = "READ4I", .has_operand = 0, .has_operand2 = 0},
    [INST_READ8I] = { .type = INST_READ8I, .name = "READ8I", .has_operand = 0, .has_operand2 = 0},
    [INST_WRITE1] = { .type = INST_WRITE1, .name = "WRITE1", .has_operand = 0, .has_operand2 = 0},
    [INST_WRITE2] = { .type = INST_WRITE2, .name = "WRITE2", .has_operand = 0, .has_operand2 = 0},
    [INST_WRITE4] = { .type = INST_WRITE4, .name = "WRITE4", .has_operand = 0, .has_operand2 = 0},
    [INST_WRITE8] = { .type = INST_WRITE8, .name = "WRITE8", .has_operand = 0, .has_operand2 = 0},
};

bool getOpcodeDetailsFromName(String_View name, OpcodeDetails* out_ptr)
{
    Opcode type = 0, last = NUMBER_OF_INSTS;
    switch (name.len) {
    case 5:
        type = INST_DONOP;
        last = INST_SHUTS;
        break;
    case 4:
        type = INST_SETR;
        last = INST_DUPS;
        break;
    case 3:
        type = INST_RET;
        last = INST_F2U;
        break;
    case 6:
        type = INST_READ1U;
        last = INST_WRITE8;
        break;

    default:
        last = 0;
        break;
    }
    while (type <= last) {
        if (sv_compare(STR(OpcodeDetailsLUT[type].name), name)) {
            *out_ptr = OpcodeDetailsLUT[type];
            return 1;
        }
        type += 1;
    }

    displayStringMessageError("Unknown instruction detected!", name);
    return 0;
}

OpcodeDetails getOpcodeDetails(Opcode type)
{
    try(type < NUMBER_OF_INSTS, "Invalid opcode %d", type);
    return OpcodeDetailsLUT[type];

ret_err:
    return OpcodeDetailsLUT[INST_DONOP];
}

inline void setFlag(Meta f, CPU* cpu, bool state)
{
    cpu->flags = state ? cpu->flags | f : cpu->flags & ~(f);
}

inline bool getFlag(Meta f, const CPU* cpu)
{
    return cpu->flags & f;
}

/*
    void loadSmExecutableIntoSasm(Sasm_Context* sasm, const char* filePath)
    {
        memset(sasm, 0, sizeof(*sasm));
        FILE* f       = openFile(filePath, "rb");
        Sasm_Metadata meta = { 0 };
        uint64 n      = fread(&meta, sizeof(meta), 1, f);

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

const char* getRegName(RegID type)
{
    switch (type) {
    case REG_H0:
        return "H0";
    case REG_H1:
        return "H1";
    case REG_I0:
        return "I0";
    case REG_I1:
        return "I1";
    case REG_JS:
        return "JS";
    case REG_KC:
        return "KC";
    case REG_L0:
        return "L0";
    case REG_L1:
        return "L1";
    case REG_L2:
        return "L2";
    case REG_L3:
        return "L3";
    case REG_NX:
        return "NX";
    case REG_OP:
        return "OP";
    case REG_P0:
        return "P0";
    case REG_P1:
        return "P1";
    case REG_P2:
        return "P2";
    case REG_P3:
        return "P3";
    case REG_QT:
        return "QT";
    case REG_RF:
        return "RF";
    case REG_SP:
        return "SP";
    case REG_COUNT:
    default:
        return "";
    }
}

Binding* resolveBindingLocalScope(Scope* scope, String_View name)
{
    for (uint64 i = 0; i < scope->bindingsCnt; ++i) {
        if (sv_compare(scope->bindings[i].name, name)) {
            return &scope->bindings[i];
        }
    }

    return NULL;
}

const char* getNameOfBindType(BindingType type)
{
    switch (type) {
    case BIND_TYPE_FLOAT:
        return "Float";
    case BIND_TYPE_UINT:
        return "Unsigned_Int";
    case BIND_TYPE_MEM_ADDR:
        return "Mem_Addr";
    case BIND_TYPE_INST_ADDR:
        return "Inst_Addr";
    default:
        return "";
    }
}

bool getStrLenByAddr(Sasm_Context* sasm, InstAddr addr, QuadWord* length)
{
    for (uint64 i = 0; i < sasm->strLensCnt; ++i) {
        if (sasm->stringLens[i].addr == addr) {
            if (length) {
                *length = quadwordFromU64(sasm->stringLens[i].len);
            }
            return true;
        }
    }

    return false;
}

uint64 getFunCallArgCnt(FuncallArg* args)
{
    uint64 result = 0;
    while (args != NULL) {
        result += 1;
        args = args->next;
    }
    return result;
}

EvalResult resultOK(QuadWord value, BindingType type)
{
    return (EvalResult) {
        .status = EVAL_STATUS_OK,
        .value  = value,
        .type   = type,
    };
}

EvalResult resultUnresolved(Binding* unresolvedBinding)
{
    return (EvalResult) {
        .status            = EVAL_STATUS_DEFERRED,
        .unresolvedBinding = unresolvedBinding
    };
}

bool loadSasmFileIntoSasmLexer(SasmLexer* lineInterpreter, String_View file_content, String_View filePath)
{
    if (!lineInterpreter)
        return false;
    lineInterpreter->source            = file_content;

    lineInterpreter->location.filePath = filePath;

    return true;
}

bool fetchCachedLineFromSasmLexer(SasmLexer* lineInterpreter, Line* output)
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

bool moveSasmLexerToNextLine(SasmLexer* lineInterpreter, Line* output)
{
    if (fetchCachedLineFromSasmLexer(lineInterpreter, output)) {
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

bool moveSasmTokenizerToNextToken(Tokenizer* tokenizer, Token* token, FileLocation location)
{
    if (fetchCachedSasmTokenFromSasmTokenizer(tokenizer, token, location)) {
        tokenizer->hasCachedToken = false;
        return true;
    }

    return false;
}

Tokenizer loadStringIntoTokenizer(String_View source)
{
    return (Tokenizer) {
        .source = source
    };
}

const char* getTokenName(TokenType type)
{
    switch (type) {
    case TOKEN_TYPE_STR:
        return "string";
    case TOKEN_TYPE_CHAR:
        return "character";
    case TOKEN_TYPE_NUMBER:
        return "number";
    case TOKEN_TYPE_NAME:
        return "name";
    case TOKEN_TYPE_OPEN_PAREN:
        return "open paren";
    case TOKEN_TYPE_CLOSING_PAREN:
        return "closing paren";
    case TOKEN_TYPE_COMMA:
        return "comma";
    case TOKEN_TYPE_REGISTER:
        return "register";
    default:
        return "";
    }
}

Expr parseExprFromStr(Arena* arena, String_View source, FileLocation location)
{
    Tokenizer tokenizer = loadStringIntoTokenizer(source);
    Expr result         = parsePrimaryOfSasmTokens(arena, &tokenizer, location);

    return result;
}

void sasm_scope_push(Sasm_Context* sasm)
{
    Scope* scope    = (Scope*)region_alloc(&sasm->arena, sizeof(*sasm->scope));
    scope->previous = sasm->scope;
    sasm->scope     = scope;
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

    for (uint64 i = 0; i < sasm->symbolsCount; ++i) {
        try((sasm->symbols[i].scope), "invalid operand scope!", "");
        sasm->scope           = sasm->symbols[i].scope;

        InstAddr addr         = sasm->symbols[i].addr;
        Expr expr             = sasm->symbols[i].expr;
        FileLocation location = sasm->symbols[i].location;

        EvalResult result     = evaluateExpression(sasm, expr, location);
        try(result.status == EVAL_STATUS_OK, "invalid operand status %d", result.status);
        sasm $instructions[addr].operand = result.value;
        if (expr.type == EXPR_FUNCALL && expr.value.funcall->args->value.type == EXPR_REG) {
            sasm $instructions[addr].opr1IsReg = true;
        }

        OpcodeDetails inst_def = getOpcodeDetails(sasm $instructions[addr].type);
        try(inst_def.has_operand, "trying to resolve operand for an inst that doesnt expect an operand!", "");

        if (!inst_def.has_operand2)
            continue;
        i += 1;
        Expr expr2         = sasm->symbols[i].expr;
        EvalResult result2 = evaluateExpression(sasm, expr2, location);
        try(result2.status == EVAL_STATUS_OK, "invalid operand status %d", result2.status);
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

Binding* sasm_binding_resolve(Sasm_Context* sasm, String_View name)
{
    for (Scope* scope = sasm->scope; scope != NULL; scope = scope->previous) {
        Binding* binding = resolveBindingLocalScope(scope, name);
        if (binding)
            return binding;
    }

    return NULL;
}

EvalResult sasm_binding_eval(Sasm_Context* sasm, Binding* binding)
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
        return (EvalResult) { .status = EVAL_ERR };
    case BIND_STATUS_EVALUATED:
        return resultOK(binding->value, binding->type);
    case BIND_STATUS_DEFERRED:
        return resultUnresolved(binding);
    default:
        return (EvalResult) { .status = EVAL_ERR };
    }
}

bool sasm_resolve_entry_point(Sasm_Context* sasm)
{
    Scope* savedScope = sasm->scope;

    if (sasm->deferredEntry.bindingName.len <= 0)
        goto ret_ok;
    try(sasm->deferredEntry.scope, "INVALID SCOPE!", "");
    sasm->scope = sasm->deferredEntry.scope;

    try(!sasm->hasEntry, FLFmt ": ERROR: entry point has been already set!\n" FLFmt ": NOTE: the first entry point\n",
        FLArg(sasm->deferredEntry.location), FLArg(sasm->entryLocation));

    Binding* binding = sasm_binding_resolve(sasm, sasm->deferredEntry.bindingName);
    try(binding != NULL, FLFmt ": ERROR: unknown binding `%.*s`\n",
        FLArg(sasm->deferredEntry.location), Str_Fmt(sasm->deferredEntry.bindingName));

    try(binding->type == BIND_TYPE_INST_ADDR, FLFmt ": ERROR: Type check error. Trying to set `%.*s` that has the type of %s as an entry point. Entry point has to be %s.\n",
        FLArg(sasm->deferredEntry.location), Str_Fmt(binding->name), getNameOfBindType(binding->type), getNameOfBindType(BIND_TYPE_INST_ADDR));

    EvalResult result = sasm_binding_eval(sasm, binding);
    try(result.status == EVAL_STATUS_OK, "Unable to resolve entry point!", "");

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
    sasm_scope_push(sasm);
    try(translateSasmFile(sasm, input_file_data, STR("src")), "Unable to translate root/src file", "");
    sasm_scope_pop(sasm);
    try(sasm_resolve_operands(sasm), "unable to resolve operands", "");
    try(sasm_resolve_entry_point(sasm), "unable to resolve entry point", "");
    return true;
ret_err:
    return false;
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

bool sasm_codeblock_push(Arena* arena, CodeBlock* list, Stmt statement)
{
    try(list, "got null ptr!", "");
    StmtNode* stmtNode  = region_alloc(arena, sizeof(StmtNode));
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
    return true;
ret_err:
    return false;
}

bool confirmNoMoreTokensRemain(Tokenizer* tokenizer, FileLocation location)
{
    Token token = { 0 };
    try(false == moveSasmTokenizerToNextToken(tokenizer, &token, location),
        FLFmt ": ERROR: unexpected token `%.*s`\n",
        FLArg(location), Str_Fmt(token.text));
    return true;
ret_err:
    return false;
}

bool sasm_line_parse_directive(Arena* arena, SasmLexer* lineInterpreter, CodeBlock* output)
{
    Line line = { 0 };

    if (!moveSasmLexerToNextLine(lineInterpreter, &line) || line.kind != LINE_DIRECTIVE) {
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
        Expr path          = parseExprFromStr(arena, body, line.location);

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

        Tokenizer tokenizer = loadStringIntoTokenizer(body);
        Expr bindingName    = parsePrimaryOfSasmTokens(arena, &tokenizer, location);
        try(bindingName.type == EXPR_BINDING, FLFmt ": ERROR: expected binding name for %%bind binding\n",
            FLArg(location));

        statement.value.constant.name  = bindingName.value.binding;

        statement.value.constant.value = parsePrimaryOfSasmTokens(arena, &tokenizer, location);
        confirmNoMoreTokensRemain(&tokenizer, location);

        return sasm_codeblock_push(arena, output, statement);
    }
    if (sv_compare(name, STR("entry"))) {
        body              = sv_trim(body);
        bool inline_entry = false;

        if (sv_ends_with(body, STR(":"))) {
            sv_split_by_len_reversed(&body, 1);
            inline_entry = true;
        }

        Expr expr                   = parseExprFromStr(arena, body, line.location);

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
        statement.value.scope = sasm_line_parse_codeblock(arena, lineInterpreter).begin;

        if (!moveSasmLexerToNextLine(lineInterpreter, &line) || line.kind != LINE_DIRECTIVE || !sv_compare(line.value.directive.name, STR("end"))) {
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

CodeBlock sasm_line_parse_codeblock(Arena* arena, SasmLexer* lineInterpreter)
{
    CodeBlock result = { 0 };

    Line line        = { 0 };
    while (fetchCachedLineFromSasmLexer(lineInterpreter, &line)) {
        const FileLocation location = line.location;
        Stmt statement              = { 0 };
        statement.location          = location;

        switch (line.kind) {
        case LINE_INSTRUCTION:
            {
                String_View name        = line.value.instruction.name;
                String_View operandList = line.value.instruction.operand;

                OpcodeDetails details;
                if (!getOpcodeDetailsFromName(name, &details)) {
                    continue;
                }

                statement.kind            = STMT_INST;
                statement.value.inst.type = details.type;

                if (details.has_operand) {
                    String_View opr1             = sv_trim(sv_split_by_delim(&operandList, ' '));
                    Expr operand                 = parseExprFromStr(arena, opr1, location);
                    statement.value.inst.operand = operand;
                }

                if (details.has_operand2) {
                    String_View opr2              = sv_trim(operandList);
                    Expr operand                  = parseExprFromStr(arena, opr2, location);
                    statement.value.inst.operand2 = operand;
                }
                try(sasm_codeblock_push(arena, &result, statement), "Unable to push into codeblock!", "");
                moveSasmLexerToNextLine(lineInterpreter, NULL);
            }
            break;

        case LINE_LABEL:
            {
                Expr label = parseExprFromStr(arena, line.value.label.name, location);

                try(label.type == EXPR_BINDING, FLFmt ": ERROR: expected binding name for a label \n",
                    FLArg(location));

                statement.kind             = STMT_LABEL;
                statement.value.label.name = label.value.binding;
                try(sasm_codeblock_push(arena, &result, statement), "Unable to push into codeblock!", "");
                moveSasmLexerToNextLine(lineInterpreter, NULL);
            }

            break;

        case LINE_DIRECTIVE:
            if (sv_compare(line.value.directive.name, STR("end"))) {
                return result;
            }

            try(sasm_line_parse_directive(arena, lineInterpreter, &result), "Unable to parse directive!", "");
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

static Expr parseNumFromSasmTokens(Arena* arena, Tokenizer* tokenizer, FileLocation location)
{
    Token token = { 0 };
    Expr result = { 0 };

    try(moveSasmTokenizerToNextToken(tokenizer, &token, location), FLFmt ": ERROR: Cannot parse empty expression\n",
        FLArg(location));

    if (token.type == TOKEN_TYPE_NUMBER) {
        String_View text = token.text;

        const char* cstr = arena_sv_to_cstr(arena, text);
        char* endptr     = 0;

        if (sv_starts_with(text, STR("0x"))) {
            result.value.lit_int = strtoull(cstr, &endptr, 16);
            try((uint64)(endptr - cstr) == text.len, FLFmt ": ERROR: `%.*s` is not a hex literal\n",
                FLArg(location), Str_Fmt(text));

            result.type = EXPR_LIT_INT;
        } else {
            result.value.lit_int = strtoull(cstr, &endptr, 10);
            if ((uint64)(endptr - cstr) != text.len) {
                result.value.lit_float = strtod(cstr, &endptr);
                if ((uint64)(endptr - cstr) != text.len) {
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
            getTokenName(TOKEN_TYPE_NUMBER),
            getTokenName(token.type));
    }
ret_err:
    return result;
}

String_View ParseStrFromSasmTokens(Tokenizer* tokenizer, FileLocation location)
{
    Token token = { 0 };
    if (!moveSasmTokenizerToNextToken(tokenizer, &token, location) || token.type != TOKEN_TYPE_STR) {
        err(FLFmt ": ERROR: expected token %s\n",
            FLArg(location), getTokenName(TOKEN_TYPE_STR));
    }

    return token.text;

ret_err:
    return (String_View) { 0 };
}

Expr parsePrimaryOfSasmTokens(Arena* arena, Tokenizer* tokenizer, FileLocation location)
{
    Token token = { 0 };
    Expr result = { 0 };

    try(fetchCachedSasmTokenFromSasmTokenizer(tokenizer, &token, location), FLFmt ": ERROR: Cannot parse empty expression\n",
        FLArg(location));

    switch (token.type) {
    case TOKEN_TYPE_STR:
        result.type          = EXPR_LIT_STR;
        result.value.lit_str = ParseStrFromSasmTokens(tokenizer, location);
        break;

    case TOKEN_TYPE_CHAR:
        moveSasmTokenizerToNextToken(tokenizer, NULL, location);

        try(token.text.len == 1, FLFmt ": ERROR: the length of char literal has to be exactly one\n",
            FLArg(location));

        result.type           = EXPR_LIT_CHAR;
        result.value.lit_char = token.text.data[0];
        break;

    case TOKEN_TYPE_NAME:
        moveSasmTokenizerToNextToken(tokenizer, NULL, location);

        Token next = { 0 };
        if (fetchCachedSasmTokenFromSasmTokenizer(tokenizer, &next, location) && next.type == TOKEN_TYPE_OPEN_PAREN) {
            result.type                = EXPR_FUNCALL;
            result.value.funcall       = region_alloc(arena, sizeof(Funcall));
            result.value.funcall->name = token.text;
            result.value.funcall->args = parseFuncallArgs(arena, tokenizer, location);
        } else {
            result.value.binding = token.text;
            result.type          = EXPR_BINDING;
        }
        break;

    case TOKEN_TYPE_NUMBER:
        return parseNumFromSasmTokens(arena, tokenizer, location);

    case TOKEN_TYPE_OPEN_PAREN:
        moveSasmTokenizerToNextToken(tokenizer, NULL, location);
        Expr expr = parsePrimaryOfSasmTokens(arena, tokenizer, location);

        try(moveSasmTokenizerToNextToken(tokenizer, &token, location) && token.type == TOKEN_TYPE_CLOSING_PAREN, FLFmt ": ERROR: expected `%s`\n",
            FLArg(location), getTokenName(TOKEN_TYPE_CLOSING_PAREN));
        return expr;

    case TOKEN_TYPE_REGISTER:
        moveSasmTokenizerToNextToken(tokenizer, NULL, location);
        String_View str = token.text;
        switch (str.data[0]) {
        case 'H':
            try(str.data[1] <= '1', FLFmt ": ERROR: Invalid register %s\n",
                FLArg(location), str.data);
            result.value.reg_id = (uint64)(REG_H0 + str.data[1] - '0');
            break;
        case 'I':
            try(str.data[1] <= '1', FLFmt ": ERROR: Invalid register %s\n",
                FLArg(location), str.data);
            result.value.reg_id = (uint64)(REG_I0 + str.data[1] - '0');
            break;
        case 'L':
            try(str.data[1] <= '3', FLFmt ": ERROR: Invalid register %s\n",
                FLArg(location), str.data);
            result.value.reg_id = (uint64)(REG_L0 + str.data[1] - '0');
            break;
        case 'P':
            try(str.data[1] <= '3', FLFmt ": ERROR: Invalid register %s\n",
                FLArg(location), str.data);
            result.value.reg_id = (uint64)(REG_P0 + str.data[1] - '0');
            break;
        case 'J':
            result.value.reg_id = (uint64)REG_JS;
            break;
        case 'K':
            result.value.reg_id = (uint64)REG_KC;
            break;
        case 'O':
            result.value.reg_id = (uint64)REG_OP;
            break;
        case 'Q':
            result.value.reg_id = (uint64)REG_QT;
            break;
        case 'R':
            result.value.reg_id = (uint64)REG_RF;
            break;

        default:
            err(FLFmt ": ERROR: Invalid register %s\n",
                FLArg(location), str.data);
        };

        result.type = EXPR_REG;
        break;
    case TOKEN_TYPE_COMMA:
    case TOKEN_TYPE_CLOSING_PAREN:
        err(FLFmt ": ERROR: expected primary expression but found %s\n",
            FLArg(location), getTokenName(token.type));
        break;
    }

ret_err:
    return result;
}

bool checkFuncArgs(Funcall* funcall, uint64 expected_arity, FileLocation location)
{
    const uint64 actual_arity = getFunCallArgCnt(funcall->args);
    try(actual_arity == expected_arity,
        FLFmt ": ERROR: %.*s() expects %" PRIu64 " but got %" PRIu64,
        FLArg(location), Str_Fmt(funcall->name), expected_arity, actual_arity);
    return true;
ret_err:
    return false;
}

EvalResult resolveFuncall(Sasm_Context* sasm, Expr expr, FileLocation location)
{
    EvalResult result = { 0 };
    if (sv_compare(expr.value.funcall->name, STR("len"))) {
        try(checkFuncArgs(expr.value.funcall, 1, location), "Incorrect arity! Expected %d arg(s).", 1);

        QuadWord addr = { 0 };
        result        = evaluateExpression(sasm, expr.value.funcall->args->value, location);
        if (result.status == EVAL_STATUS_DEFERRED)
            return result;

        addr            = result.value;
        QuadWord length = { 0 };
        try(getStrLenByAddr(sasm, addr.u64, &length), FLFmt ": ERROR: Could not compute the length of string at address %" PRIu64 "\n",
            FLArg(location), addr.u64);
        return resultOK(length, BIND_TYPE_UINT);
    }
    if (sv_compare(expr.value.funcall->name, STR("res"))) {
        try(checkFuncArgs(expr.value.funcall, 1, location), "Incorrect arity! Expected %d arg(s).", 1);

        QuadWord addr = { 0 };
        result        = evaluateExpression(sasm, expr.value.funcall->args->value, location);

        try(sasm->mem_size + result.value.u64 <= MAX_MEMORY_CAPACITY, "memory cap excedeed!", "");

        addr = quadwordFromU64(sasm->mem_size);
        sasm->mem_size += result.value.u64;

        if (sasm->mem_size > sasm->mem_capacity) {
            sasm->mem_capacity = sasm->mem_size;
        }

        return resultOK(addr, BIND_TYPE_UINT);
    }
    if (sv_compare(expr.value.funcall->name, STR("ref"))) {
        try(checkFuncArgs(expr.value.funcall, 1, location), "Incorrect arity! Expected %d arg(s).", 1);

        try(expr.value.funcall->args->value.type == EXPR_REG, FLFmt ": ERROR: ref expects a register ",
            FLArg(location));

        result = evaluateExpression(sasm, expr.value.funcall->args->value, location);
        return result;
    }
    if (sv_compare(expr.value.funcall->name, STR("val"))) {
        try(checkFuncArgs(expr.value.funcall, 1, location), "Incorrect arity! Expected %d arg(s).", 1);

        try(expr.value.funcall->args->value.type == EXPR_REG, FLFmt ": ERROR: val expects a register ",
            FLArg(location));

        result = evaluateExpression(sasm, expr.value.funcall->args->value, location);
        if (result.status == EVAL_STATUS_DEFERRED)
            return result;

        uint64 val = result.value.u64 + REG_COUNT;
        return resultOK(quadwordFromU64(val), BIND_TYPE_UINT);
    }
    err(FLFmt ": ERROR: Unknown translation time function `%.*s`\n",
        FLArg(location), Str_Fmt(expr.value.funcall->name));
ret_err:
    result = (EvalResult) { .status = EVAL_ERR };
    return result;
}

QuadWord pushStringToMemory(Sasm_Context* sasm, String_View str)
{
    try(sasm->mem_size + str.len <= MAX_MEMORY_CAPACITY, "Memory capacity exceeded when trying to declare string!", "");

    QuadWord result = quadwordFromU64(sasm->mem_size);
    memcpy(sasm->memory + sasm->mem_size, str.data, str.len);
    sasm->mem_size += str.len;

    if (sasm->mem_size > sasm->mem_capacity) {
        sasm->mem_capacity = sasm->mem_size;
    }

    sasm->stringLens[sasm->strLensCnt++] = (StringLength) {
        .addr = result.u64,
        .len  = str.len,
    };

    return result;
ret_err:
    return (QuadWord) { 0 };
}

EvalResult evaluateExpression(Sasm_Context* sasm, Expr expr, FileLocation location)
{
    EvalResult res = { 0 };
    switch (expr.type) {
    case EXPR_LIT_INT:
        res = resultOK(quadwordFromU64(expr.value.lit_int), BIND_TYPE_UINT);
        break;

    case EXPR_LIT_FLOAT:
        res = resultOK(quadwordFromF64(expr.value.lit_float), BIND_TYPE_FLOAT);
        break;

    case EXPR_LIT_CHAR:
        res = resultOK(quadwordFromU64(expr.value.lit_char), BIND_TYPE_UINT);
        break;

    case EXPR_LIT_STR:
        res = resultOK(pushStringToMemory(sasm, expr.value.lit_str), BIND_TYPE_MEM_ADDR);
        break;

    case EXPR_FUNCALL:
        res = resolveFuncall(sasm, expr, location);
        break;

    case EXPR_BINDING:
        {
            String_View name = expr.value.binding;
            Binding* binding = sasm_binding_resolve(sasm, name);
            try(binding != NULL, FLFmt ": ERROR: couldnt find binding `%.*s`.\n",
                FLArg(location), Str_Fmt(name));

            res = sasm_binding_eval(sasm, binding);
        }
        break;

    case EXPR_REG:
        res = resultOK(quadwordFromU64(expr.value.reg_id), BIND_TYPE_UINT);
        break;
    }
ret_err:
    return res;
}

bool bindExprLocalScope(Scope* scope, String_View name, Expr expr, FileLocation location)
{
    try(scope->bindingsCnt < BINDINGS_CAPACITY, "max binding capacity exceeded!", "");

    Binding* existing = resolveBindingLocalScope(scope, name);
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

bool translateSasmBindDirective(Sasm_Context* sasm, ConstStmt konst, FileLocation location)
{
    try(sasm->scope != NULL, "No Scope to bind to!", "");
    return bindExprLocalScope(sasm->scope, konst.name, konst.value, location);
ret_err:
    return false;
}

bool translateSasmIncludeDirective(Sasm_Context* sasm, IncludeStmt include, FileLocation location)
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
    try(translateSasmFile(sasm, include.content, include.path), "Unable to include file", "");
    sasm->includeLocation = prev_includeLocation;
    sasm->includeLevel -= 1;
    return true;
ret_err:
    return false;
}

bool translateSasmEntryDirective(Sasm_Context* sasm, EntryStmt entry, FileLocation location)
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

bool translateSasmInstruction(Sasm_Context* sasm, InstStmt inst, FileLocation location)
{
    try(sasm $instructionCount < MAX_PROGRAM_CAPACITY, "Max instruction count exceeded!", "");
    sasm $instructions[sasm $instructionCount].type         = inst.type;
    sasm $instructions[sasm $instructionCount].operand.u64  = 0;
    sasm $instructions[sasm $instructionCount].operand2.u64 = 0;

    OpcodeDetails details                                   = getOpcodeDetails(inst.type);
    if (details.has_operand) {
        try(pushUnresolvedOperand(sasm, sasm $instructionCount, inst.operand, location), "Unable to defer operand!", "");
    }
    if (details.has_operand2) {
        try(pushUnresolvedOperand(sasm, sasm $instructionCount, inst.operand2, location), "Unable to defer operand!", "");
    }
    sasm $instructionCount += 1;
    return true;
ret_err:
    return false;
}

bool bindUnresolvedLocalScope(Scope* scope, String_View name, BindingType type, FileLocation location)
{
    try(scope->bindingsCnt < BINDINGS_CAPACITY, "Binding cap exceeded!", "");

    Binding* existing = resolveBindingLocalScope(scope, name);
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
        .value.u64 = 0
    };

    return true;
ret_err:
    return false;
}

bool translateSasmStatementChain(Sasm_Context* sasm, StmtNode* block)
{
    for (StmtNode* iter = block; iter != NULL; iter = iter->next) {
        Stmt statement = iter->statement;
        switch (statement.kind) {
        case STMT_LABEL:
            try(bindUnresolvedLocalScope(sasm->scope, statement.value.label.name, BIND_TYPE_INST_ADDR, statement.location), "Unable to bind!", "");
            break;
        case STMT_CONST:
            try(translateSasmBindDirective(sasm, statement.value.constant, statement.location), "Unable to bind!", "");
            break;
        case STMT_INCLUDE:
            try(translateSasmIncludeDirective(sasm, statement.value.include, statement.location), "Unable to bind!", "");
            break;
        case STMT_ENTRY:
            try(translateSasmEntryDirective(sasm, statement.value.entry, statement.location), "Unable to bind!", "");
            break;
        case STMT_BLOCK:     // Currently unused!
            try(translateSasmStatementChain(sasm, statement.value.block), "Unable to bind!", "");
            break;

        case STMT_SCOPE:
        case STMT_INST:
            break;
        }
    }

    for (StmtNode* iter = block; iter != NULL; iter = iter->next) {
        Stmt statement = iter->statement;
        switch (statement.kind) {
        case STMT_INST:
            try(translateSasmInstruction(sasm, statement.value.inst, statement.location), "Unable to bind!", "");
            break;

        case STMT_LABEL:
            {
                Binding* binding = sasm_binding_resolve(sasm, statement.value.label.name);
                try(binding != NULL, "binding not found", "");
                try(binding->status == BIND_STATUS_DEFERRED, "binding already defined!", "");

                binding->status    = BIND_STATUS_EVALUATED;
                binding->value.u64 = sasm $instructionCount;
            }
            break;

        case STMT_SCOPE:
            sasm_scope_push(sasm);
            try(translateSasmStatementChain(sasm, statement.value.scope), "Unable to translate scope", "");
            sasm_scope_pop(sasm);
            break;

        case STMT_BLOCK:
        case STMT_ENTRY:
        case STMT_INCLUDE:
        case STMT_CONST:
            break;
        }
    }
    return true;
ret_err:
    return false;
}

bool translateSasmFile(Sasm_Context* sasm, String_View inputFileData, String_View inputFilePath)
{
    SasmLexer SasmLexer = { 0 };

    if (!loadSasmFileIntoSasmLexer(&SasmLexer, inputFileData, inputFilePath)) {
        // todo: maybe file handling?
        if (sasm->includeLevel > 0) {
            err(FLFmt ": Could not read file", FLArg(sasm->includeLocation));
        } else {
            err("Could not read file", "");
        }
    }

    CodeBlock inputFileBlock = sasm_line_parse_codeblock(&sasm->arena, &SasmLexer);

    return translateSasmStatementChain(sasm, inputFileBlock.begin);
ret_err:
    return false;
}

bool pushUnresolvedOperand(Sasm_Context* sasm, InstAddr addr, Expr expr, FileLocation location)
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

bool pushIncludePath(Sasm_Context* sasm, const char* path)
{
    try(sasm->includePathsCnt < INCLUDE_PATHS_CAPACITY, "Include path cap exceeded!", "");
    sasm->includePaths[sasm->includePathsCnt++] = STR(path);
    return true;
ret_err:
    return false;
}

bool fetchCachedSasmTokenFromSasmTokenizer(Tokenizer* tokenizer, Token* output, FileLocation location)
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
    case '(':
        {
            token.type = TOKEN_TYPE_OPEN_PAREN;
            token.text = sv_split_by_len(&tokenizer->source, 1);
        }
        break;

    case ')':
        {
            token.type = TOKEN_TYPE_CLOSING_PAREN;
            token.text = sv_split_by_len(&tokenizer->source, 1);
        }
        break;

    case ',':
        {
            token.type = TOKEN_TYPE_COMMA;
            token.text = sv_split_by_len(&tokenizer->source, 1);
        }
        break;

    case '"':
        {
            sv_split_by_len(&tokenizer->source, 1);
            uint64 index = 0;

            try(sv_index_of(tokenizer->source, '"', &index),
                FLFmt ": ERROR: Could not find closing \"\n",
                FLArg(location));
            String_View text = sv_split_by_len(&tokenizer->source, index);
            sv_split_by_len(&tokenizer->source, 1);
            token.type = TOKEN_TYPE_STR;
            token.text = text;
        }
        break;

    case '\'':
        {
            sv_split_by_len(&tokenizer->source, 1);
            uint64 index = 0;

            try(sv_index_of(tokenizer->source, '\'', &index), FLFmt ": ERROR: Could not find closing \'\n",
                FLArg(location));
            String_View text = sv_split_by_len(&tokenizer->source, index);
            sv_split_by_len(&tokenizer->source, 1);
            token.type = TOKEN_TYPE_CHAR;
            token.text = text;
        }
        break;

    default:
        {
            if (isalphabet(*tokenizer->source.data)) {
                token.type = TOKEN_TYPE_NAME;
                token.text = sv_split_by_condition(&tokenizer->source, isName);
            } else if (is_digit(*tokenizer->source.data) || *tokenizer->source.data == '-') {
                token.type = TOKEN_TYPE_NUMBER;
                token.text = sv_split_by_condition(&tokenizer->source, isNumber);
            } else if (tokenizer->source.len >= 3 && *tokenizer->source.data == '[' && tokenizer->source.data[3] == ']') {
                if (tokenizer->source.len - 2 < 2) {
                    err(FLFmt ": ERROR: Check register name %c\n",
                        FLArg(location), *tokenizer->source.data);
                }
                sv_split_by_len(&tokenizer->source, 1);
                uint64 index = 0;
                try(sv_index_of(tokenizer->source, ']', &index), FLFmt ": ERROR: Could not find closing \'\n",
                    FLArg(location));
                token.type = TOKEN_TYPE_REGISTER;
                token.text = sv_split_by_len(&tokenizer->source, index);
                sv_split_by_len(&tokenizer->source, 1);
            } else {
                err(FLFmt ": ERROR: Unknown token starts with %c\n",
                    FLArg(location), *tokenizer->source.data);
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

FuncallArg* parseFuncallArgs(Arena* arena, Tokenizer* tokenizer, FileLocation location)
{
    Token token = { 0 };

    try(moveSasmTokenizerToNextToken(tokenizer, &token, location) && token.type == TOKEN_TYPE_OPEN_PAREN,
        FLFmt ": ERROR: expected %s\n",
        FLArg(location),
        getTokenName(TOKEN_TYPE_OPEN_PAREN));

    if (fetchCachedSasmTokenFromSasmTokenizer(tokenizer, &token, location) && token.type == TOKEN_TYPE_CLOSING_PAREN) {
        moveSasmTokenizerToNextToken(tokenizer, NULL, location);
        return NULL;
    }

    FuncallArg* first = NULL;
    FuncallArg* last  = NULL;

    do {
        FuncallArg* arg = region_alloc(arena, sizeof(FuncallArg));
        arg->value      = parsePrimaryOfSasmTokens(arena, tokenizer, location);

        if (first == NULL) {
            first = arg;
            last  = arg;
        } else {
            last->next = arg;
            last       = arg;
        }

        try(moveSasmTokenizerToNextToken(tokenizer, &token, location),
            FLFmt ": ERROR: expected %s or %s\n",
            FLArg(location),
            getTokenName(TOKEN_TYPE_CLOSING_PAREN),
            getTokenName(TOKEN_TYPE_COMMA));
    } while (token.type == TOKEN_TYPE_COMMA);

    try(token.type == TOKEN_TYPE_CLOSING_PAREN,
        FLFmt ": ERROR: expected %s\n",
        FLArg(location), getTokenName(TOKEN_TYPE_CLOSING_PAREN));

    return first;
ret_err:
    return NULL;
}

QuadWord quadwordFromU64(uint64 u64)
{
    return (QuadWord) { .u64 = u64 };
}

QuadWord quadwordFromI64(int64 i64)
{
    return (QuadWord) { .i64 = i64 };
}

QuadWord quadwordFromF64(double f64)
{
    return (QuadWord) { .f64 = f64 };
}

QuadWord quadwordFromPtr(void* ptr)
{
    return (QuadWord) { .ptr = ptr };
}

const char* getNameOfError(const VM_Error* error)
{
    switch ((*error)) {
    case ERR_OK:
        return "ERR_OK";
    case ERR_STACK_OVERFLOW:
        return "ERR_STACK_OVERFLOW";
    case ERR_STACK_UNDERFLOW:
        return "ERR_STACK_UNDERFLOW";
    case ERR_DIV_BY_ZERO:
        return "ERR_DIV_BY_ZERO";
    case ERR_ILLEGAL_INST:
        return "ERR_ILLEGAL_INST";
    case ERR_ILLEGAL_INST_ACCESS:
        return "ERR_ILLEGAL_INST_ACCESS";
    case ERR_ILLEGAL_OPERAND:
        return "ERR_ILLEGAL_OPERAND";
    case ERR_NULL_CALL:
        return "ERR_NULL_CALL";
    case ERR_ILLEGAL_MEMORY_ACCESS:
        return "ERR_ILLEGAL_MEMORY_ACCESS";
    case ERR_NAN:
        return "ERR_NAN";
    case ERR_ALREADY_BOUND:
        return "ERR_ALREADY_BOUND";
    default:
        return "";
    }
}

void displayStringMessageError(const char* msg, String_View str)
{
    printf("\n|   |                                                                                                                              |");
    printf("\n| W | ERROR | '%.*s' | %s", Str_Fmt(str), msg);

    for (uint64 i = strlen(msg) + str.len; i < 110; i++)
        printf(" ");

    printf("|"
           "\n|   |                                                                                                                              |\n");
}

void debugCommentDisplay(String_View* s)
{
    String_View seperator = sv_split_by_delim(s, ' ');
    printf("\n| %.*s |", 1, seperator.data);

    if (s->len < 125)
        printf(" %-*.*s |", (int)(124), (int)(s->len), s->data);
    else
        printf(" %.*s |", (int)(124), s->data);
}

void debugMessageDisplay(String_View* s)
{
    printf("\n| D |");

    if (s->len < 125)
        printf(" %-*.*s |", (int)(124), (int)(s->len), s->data);
    else
        printf(" %.*s |", (int)(124), s->data);
}

#endif
