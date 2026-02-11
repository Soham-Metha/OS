/**
 * @file sasm_assembler.h
 * @brief This file contains functions for assembling and disassembling programs in a custom assembly language.
 *
 * The functions in this file are used to manipulate custom assembly code and bytecode.
 */

#pragma once

#include "../../../../common/memmanager.h"
#include "univ_defs.h"
#include "univ_errors.h"
#include "univ_fileops.h"
#include "univ_strings.h"

#define $instructionCount ->prog.instruction_count
#define $instructions ->prog.instructions
#define DEFERRED_ASSERTS_CAPACITY 1024
#define STRING_LENGTHS_CAPACITY 1024
#define INCLUDE_PATHS_CAPACITY 1024
#define MAX_PRECEDENCE 2
#define Token_Fmt "%s"
#define Token_Arg(token) token_kind_name((token).kind)

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

typedef QuadWord Register;
typedef struct __attribute__((__packed__)) Instruction Instruction;
typedef struct OpcodeDetails OpcodeDetails;
typedef struct Program Program;
typedef struct CPU CPU;
typedef struct Memory Memory;
typedef union Registers Registers;

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
    Instruction instructions[PROGRAM_CAPACITY]; /**< The array of instructions */
    DataEntry instruction_count;                /**< The number of instructions in the program */
    DataEntry instruction_size;                 /**< The size of each instruction in bytes */
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
    Byte memory[MEMORY_CAPACITY];
};
bool getOpcodeDetailsFromName(String name, OpcodeDetails* outPtr);

OpcodeDetails getOpcodeDetails(Opcode type);

void setFlag(Meta f, CPU* cpu, bool state);

bool getFlag(Meta f, const CPU* cpu);

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
    EVAL_STATUS_DEFERRED
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

typedef struct Sasm Sasm;
typedef struct StringLength StringLength;
typedef struct Scope Scope;
typedef struct UnresolvedOperand UnresolvedOperand;
typedef struct DeferredAssert DeferredAssert;
typedef struct DeferredEntry DeferredEntry;

typedef struct __attribute__((__packed__)) Metadata Metadata;

union ExprValue {
    String binding;
    u64 lit_int;
    double lit_float;
    char lit_char;
    String lit_str;
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
    String name;
    FuncallArg* args;
};

struct SasmInstructionLine {
    String name;
    String operand;
    String operand2;
};

struct SasmLabelLine {
    String name;
};

struct SasmDirectiveLine {
    String name;
    String body;
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
    String source;
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
    String name;
};

struct ConstStmt {
    String name;
    Expr value;
};

struct IncludeStmt {
    String path;
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
    String text;
};

struct Tokenizer {
    String source;
    Token cachedToken;
    bool hasCachedToken;
};

struct Binding {
    BindingType type;
    String name;
    QuadWord value;
    Expr expr;
    BindingStatus status;
    FileLocation location;
};

struct StringLength {
    InstAddr addr;
    u64 length;
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
    String bindingName;
    FileLocation location;
    Scope* scope;
};

struct Sasm {
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

    Byte memory[MEMORY_CAPACITY];
    uint64 memorySize;
    uint64 memoryCapacity;

    Arena arena;

    uint64 includeLevel;
    FileLocation includeLocation;

    String includePaths[INCLUDE_PATHS_CAPACITY];
    uint64 includePathsCnt;
};

struct Metadata {
    DoubleWord magic;
    Word version;
    DataEntry programSize;
    DataEntry entry;
    DataEntry memorySize;
    DataEntry memoryCapacity;
    DataEntry externalsSize;
};

void translateSasmRootFile(Sasm* sasm, String inputFilePath);
void generateSmExecutable(Sasm* sasm, const char* filePath);
void translateSasmFile(Sasm* sasm, String inputFilePath);
void loadSmExecutableIntoSasm(Sasm* sasm, const char* filePath);

void pushUnresolvedOperand(Sasm* sasm, InstAddr addr, Expr expr, FileLocation location);
void pushIncludePath(Sasm* sasm, const char* path);
void bindUnresolvedLocalScope(Scope* scope, String name, BindingType type, FileLocation location);
void bindExprLocalScope(Scope* scope, String name, Expr expr, FileLocation location);
Binding* resolveBinding(Sasm* sasm, String name);
EvalResult evaluateBinding(Sasm* sasm, Binding* binding);

bool loadSasmFileIntoSasmLexer(SasmLexer* lineInterpreter, Arena* arena, String filePath);
bool fetchCachedLineFromSasmLexer(SasmLexer* lineInterpreter, Line* output);
bool moveSasmLexerToNextLine(SasmLexer* lineInterpreter, Line* output);

Expr parsePrimaryOfSasmTokens(Arena* arena, Tokenizer* tokenizer, FileLocation location);
CodeBlock getCodeBlockFromLines(Arena* arena, SasmLexer* lineInterpreter);

bool fetchCachedSasmTokenFromSasmTokenizer(Tokenizer* tokenizer, Token* output, FileLocation location);
bool moveSasmTokenizerToNextToken(Tokenizer* tokenizer, Token* token, FileLocation location);
Tokenizer loadStringIntoTokenizer(String source);

const char* getNameOfBindType(BindingType type);
const char* getTokenName(TokenType type);

EvalResult resultOK(QuadWord value, BindingType type);
EvalResult resultUnresolved(Binding* unresolvedBinding);

FuncallArg* parseFuncallArgs(Arena* arena, Tokenizer* tokenizer, FileLocation location);
EvalResult evaluateExpression(Sasm* sasm, Expr expr, FileLocation location);

int dumpExprAsAST(FILE* stream, Expr expr, int* counter);
int dumpStmtNodeAsAST(FILE* stream, StmtNode* stmtNode, int* counter, int* blockNo);
int dumpStatementAsAST(FILE* stream, Stmt statement, int* counter, int* blockNo);
void generateASTPng(String inputFilePath, StmtNode* start);

void BeforeLineRead();

void AfterLineRead();

void BeforeFileProcessing(String inputFilePath);

void AfterFileProcessing(String inputFilePath, StmtNode* codeBlockBegin);

void BeforeAssembly();

void AfterAssembly();

void BeforeStatementParse();

void AfterStatementParse();

void BeforeExpressionParse();

void AfterExpressionParse();

#ifdef IMPL_SASM_1
#undef IMPL_SASM_1

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

bool getOpcodeDetailsFromName(String name, OpcodeDetails* out_ptr)
{
    Opcode type = 0, last = NUMBER_OF_INSTS;
    switch (name.length) {
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
        if (compareStr(convertCstrToStr(OpcodeDetailsLUT[type].name), name)) {
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
    assert(type < NUMBER_OF_INSTS);
    return OpcodeDetailsLUT[type];
}

inline void setFlag(Meta f, CPU* cpu, bool state)
{
    cpu->flags = state ? cpu->flags | f : cpu->flags & ~(f);
}

inline bool getFlag(Meta f, const CPU* cpu)
{
    return cpu->flags & f;
}

void pushScope(Sasm* sasm, Scope* scope)
{
    assert(scope->previous == NULL);
    scope->previous = sasm->scope;
    sasm->scope     = scope;
}

void createAndPushScope(Sasm* sasm)
{
    Scope* scope = region_alloc(&sasm->arena, sizeof(*sasm->scope));
    pushScope(sasm, scope);
}

void popScope(Sasm* sasm)
{
    assert(sasm->scope != NULL);
    sasm->scope = sasm->scope->previous;
}

bool resolveIncludeFilePath(Sasm* sasm, String filePath, String* resolvedPath)
{
    for (uint64 i = 0; i < sasm->includePathsCnt; ++i) {
        String path = appendToPath(&sasm->arena, sasm->includePaths[i],
            filePath);
        if (resolvedPath) {
            *resolvedPath = path;
        }
        return true;
    }

    return false;
}

void translateSasmEntryDirective(Sasm* sasm, EntryStmt entry, FileLocation location)
{
    assert(sasm->scope);

    if (sasm->deferredEntry.bindingName.length > 0) {
        fprintf(stderr,
            FLFmt ": ERROR: entry point has been already set within the same scope!\n",
            FLArg(location));
        fprintf(stderr, FLFmt ": NOTE: the first entry point\n",
            FLArg(sasm->deferredEntry.location));
        exit(1);
    }

    if (entry.value.type != EXPR_BINDING) {
        fprintf(stderr, FLFmt ": ERROR: only bindings are allowed to be set as entry points for now.\n",
            FLArg(location));
        exit(1);
    }

    String label                    = entry.value.value.binding;
    sasm->deferredEntry.bindingName = label;
    sasm->deferredEntry.location    = location;
    sasm->deferredEntry.scope       = sasm->scope;
}

void translateSasmIncludeDirective(Sasm* sasm, IncludeStmt include, FileLocation location)
{
    // Load file as it is, recursively!
    String resolved_path = (String) { 0 };
    if (resolveIncludeFilePath(sasm, include.path, &resolved_path)) {
        include.path = resolved_path;
    }

    FileLocation prev_includeLocation = sasm->includeLocation;
    sasm->includeLevel += 1;
    sasm->includeLocation = location;
    translateSasmFile(sasm, include.path);
    sasm->includeLocation = prev_includeLocation;
    sasm->includeLevel -= 1;
}

void translateSasmBindDirective(Sasm* sasm, ConstStmt konst, FileLocation location)
{
    // ENCPSULATED SYMBOL SHOULD BELONG TO LOCAL SCOPE!
    assert(sasm->scope != NULL);
    bindExprLocalScope(sasm->scope, konst.name, konst.value, location);
}

void translateSasmInstruction(Sasm* sasm, InstStmt inst, FileLocation location)
{
    assert(sasm $instructionCount < PROGRAM_CAPACITY);
    // push instruction into array
    sasm $instructions[sasm $instructionCount].type         = inst.type;
    sasm $instructions[sasm $instructionCount].operand.u64  = 0;
    sasm $instructions[sasm $instructionCount].operand2.u64 = 0;

    // if instruction has operand, defer it for backpatching!
    OpcodeDetails details                                   = getOpcodeDetails(inst.type);
    if (details.has_operand) {
        pushUnresolvedOperand(sasm, sasm $instructionCount, inst.operand, location);
    }
    if (details.has_operand2) {
        pushUnresolvedOperand(sasm, sasm $instructionCount, inst.operand2, location);
    }
    // printf("%s %d %d %ld %ld\n", details.name, inst.operand.type, inst.operand2.type, inst.operand.value.lit_int, inst.operand2.value.lit_int);

    sasm $instructionCount += 1;
}

void translateSasmStatementChain(Sasm* sasm, StmtNode* block)
{
    // Resolve all preprocessor directives!
    for (StmtNode* iter = block; iter != NULL; iter = iter->next) {
        Stmt statement = iter->statement;
        switch (statement.kind) {
        case STMT_LABEL:
            bindUnresolvedLocalScope(sasm->scope, statement.value.label.name, BIND_TYPE_INST_ADDR, statement.location);
            break;
        case STMT_CONST:
            translateSasmBindDirective(sasm, statement.value.constant, statement.location);
            break;
        case STMT_INCLUDE:
            translateSasmIncludeDirective(sasm, statement.value.include, statement.location);
            break;
        case STMT_ENTRY:
            translateSasmEntryDirective(sasm, statement.value.entry, statement.location);
            break;
        case STMT_BLOCK:     // Currently unused!
            translateSasmStatementChain(sasm, statement.value.block);
            break;

        case STMT_SCOPE:
        case STMT_INST:
            break;

        default:
            assert(false && "translate: unreachable");
            exit(1);
        }
    }

    // Actual processing of labels, instructions and scope blocks!
    for (StmtNode* iter = block; iter != NULL; iter = iter->next) {
        Stmt statement = iter->statement;
        switch (statement.kind) {
        case STMT_INST:
            translateSasmInstruction(sasm, statement.value.inst, statement.location);
            break;

        case STMT_LABEL:
            Binding* binding = resolveBinding(sasm, statement.value.label.name);
            assert(binding != NULL);
            assert(binding->status == BIND_STATUS_DEFERRED);

            binding->status    = BIND_STATUS_EVALUATED;
            binding->value.u64 = sasm $instructionCount;
            break;

        case STMT_SCOPE:
            createAndPushScope(sasm);
            translateSasmStatementChain(sasm, statement.value.scope);
            popScope(sasm);
            break;

        case STMT_BLOCK:
        case STMT_ENTRY:
        case STMT_INCLUDE:
        case STMT_CONST:
            break;

        default:
            assert(false && "translate: unreachable");
            exit(1);
        }
    }
}

void resolveAllUnresolvedOperands(Sasm* sasm)
{
    // Check the concept of Backpatching in single pass assemblers to understand!
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

        if (inst_def.has_operand2) {
            i++;
            Expr expr2                        = sasm->symbols[i].expr;
            EvalResult result2                = evaluateExpression(sasm, expr2, location);
            sasm $instructions[addr].operand2 = result2.value;
            if (expr.type == EXPR_FUNCALL && expr.value.funcall->args->value.type == EXPR_REG) {
                sasm $instructions[addr].opr2IsReg = true;
            }
        }
    }

    sasm->scope = savedScope;
}

void resolveProgramEntryPoint(Sasm* sasm)
{
    Scope* savedScope = sasm->scope;
    if (sasm->deferredEntry.bindingName.length > 0) {
        assert(sasm->deferredEntry.scope);
        sasm->scope = sasm->deferredEntry.scope;

        if (sasm->hasEntry) {
            fprintf(stderr,
                FLFmt ": ERROR: entry point has been already set!\n",
                FLArg(sasm->deferredEntry.location));
            fprintf(stderr, FLFmt ": NOTE: the first entry point\n",
                FLArg(sasm->entryLocation));
            exit(1);
        }

        Binding* binding = resolveBinding(
            sasm,
            sasm->deferredEntry.bindingName);
        if (binding == NULL) {
            fprintf(stderr, FLFmt ": ERROR: unknown binding `" strFmt "`\n",
                FLArg(sasm->deferredEntry.location),
                strArg(sasm->deferredEntry.bindingName));
            exit(1);
        }

        if (binding->type != BIND_TYPE_INST_ADDR) {
            fprintf(stderr, FLFmt ": ERROR: Type check error. Trying to set `" strFmt "` that has the type of %s as an entry point. Entry point has to be %s.\n",
                FLArg(sasm->deferredEntry.location),
                strArg(binding->name),
                getNameOfBindType(binding->type),
                getNameOfBindType(BIND_TYPE_INST_ADDR));
            exit(1);
        }

        EvalResult result = evaluateBinding(sasm, binding);
        assert(result.status == EVAL_STATUS_OK);

        sasm->entry         = result.value.u64;
        sasm->hasEntry      = true;
        sasm->entryLocation = sasm->deferredEntry.location;
    }

    sasm->scope = savedScope;
}

void translateSasmRootFile(Sasm* sasm, String inputFilePath)
{
    // Create the 'global scope' and start processing file.
    createAndPushScope(sasm);
    translateSasmFile(sasm, inputFilePath);
    popScope(sasm);

    // backpatching of the operands!
    resolveAllUnresolvedOperands(sasm);
    resolveProgramEntryPoint(sasm);
}

void generateSmExecutable(Sasm* sasm, const char* filePath)
{
    FILE* f       = openFile(filePath, "wb");

    Metadata meta = {
        .magic          = FILE_MAGIC,
        .version        = FILE_VERSION,
        .entry          = sasm->entry,
        .programSize    = sasm $instructionCount,
        .memorySize     = sasm->memorySize,
        .memoryCapacity = sasm->memoryCapacity,
    };
    // BeforeAssembly();
    /*
     * Try to write metadata
     */
    fwrite(&meta, sizeof(meta), 1, f);
    if (ferror(f)) {
        fileErrorDispWithExit("Could not write to file", filePath);
    }

    /*
     * Try to write instructions
     */
    fwrite(sasm $instructions, sizeof(sasm $instructions[0]), sasm $instructionCount, f);
    if (ferror(f)) {
        fileErrorDispWithExit("Could not write to file", filePath);
    }

    /*
     * Try to write program data
     */
    fwrite(sasm->memory, sizeof(sasm->memory[0]), sasm->memorySize, f);
    if (ferror(f)) {
        fileErrorDispWithExit("Could not write to file", filePath);
    }
    // AfterAssembly();
    closeFile(f, filePath);
}

void translateSasmFile(Sasm* sasm, String inputFilePath)
{
    BeforeFileProcessing(inputFilePath);

    SasmLexer SasmLexer = { 0 };

    if (!loadSasmFileIntoSasmLexer(&SasmLexer, &sasm->arena, inputFilePath)) {

        if (sasm->includeLevel > 0) {
            fprintf(stderr, FLFmt, FLArg(sasm->includeLocation));
        }
        fileErrorDispWithExit("Could not read file", inputFilePath.data);
    }
    /*
     * Convert the code written in the "string" format, to the format
     * expected by our assembler (i.e. in the form of struct 'CodeBlock')
     */
    CodeBlock inputFileBlock = getCodeBlockFromLines(&sasm->arena, &SasmLexer);
    /*
     * Perform actual processing of the directives, labels and scopes
     * and add instructions to the instructions array (which is the
     * actual contents written to the sm executable file)
     */
    translateSasmStatementChain(sasm, inputFileBlock.begin);

    AfterFileProcessing(inputFilePath, inputFileBlock.begin);
}

void loadSmExecutableIntoSasm(Sasm* sasm, const char* filePath)
{
    memset(sasm, 0, sizeof(*sasm));

    FILE* f       = openFile(filePath, "rb");

    Metadata meta = { 0 };

    uint64 n      = fread(&meta, sizeof(meta), 1, f);

    /*
     * ensure that the sm file is readable, of the correct format,
     * using correct version of sasm, within the program size limits
     */
    if (n < 1) {
        fileErrorDispWithExit("Could not read meta data from file", filePath);
    }

    if (meta.magic != FILE_MAGIC) {
        fprintf(stderr, "Unexpected magic %04X. Expected %04X.\n", meta.magic, FILE_MAGIC);
        fileErrorDispWithExit("Not a valid SASM File ", filePath);
    }

    if (meta.version != FILE_VERSION) {
        fprintf(stderr, "Encountered version %d. Expected version %d.\n", meta.version, FILE_VERSION);
        fileErrorDispWithExit("unsupported version of SASM File ", filePath);
    }

    if (meta.programSize > PROGRAM_CAPACITY) {
        fprintf(stderr,
            "The file contains %" PRIu64 " program instruction. But the capacity is %" PRIu64 "\n",
            meta.programSize, (u64)PROGRAM_CAPACITY);
        fileErrorDispWithExit("program section is too big ", filePath);
    }

    if (meta.memoryCapacity > MEMORY_CAPACITY) {
        fprintf(stderr,
            "The file wants %" PRIu64 " bytes. But the capacity is %" PRIu64 " bytes\n",
            meta.memoryCapacity, (u64)MEMORY_CAPACITY);
        fileErrorDispWithExit(" memory section is too big ", filePath);
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
    /*
     * If all the checks pass, load all instructions and data(memory contents)
     * and ensure the amount of data read matches the expected amount.
     */
    sasm $instructionCount = fread(sasm $instructions, sizeof(sasm $instructions[0]), meta.programSize, f);

    if (sasm $instructionCount != meta.programSize) {
        fprintf(stderr, "ERROR: %s: read %" PRIu64 " program instructions, but expected %" PRIu64 "\n",
            filePath, sasm $instructionCount, meta.programSize);
        exit(1);
    }

    n = fread(sasm->memory, sizeof(sasm->memory[0]), meta.memorySize, f);

    if (n != meta.memorySize) {
        fprintf(stderr, "ERROR: %s: read %" PRIu64 " bytes of memory section, but expected %" PRIu64 " bytes.\n",
            filePath, n, meta.memorySize);
        exit(1);
    }

    closeFile(f, filePath);
}
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
        assert(false && "getRegName: REG_COUNT is not a real register");
        exit(1);
    default:
        assert(false && "getRegName: unreachable");
        exit(1);
    }
}

int dumpExprAsAST(FILE* stream, Expr expr, int* counter)
{
    // Operands!
    int id = (*counter)++;

    switch (expr.type) {
    case EXPR_BINDING:
        fprintf(stream,
            "Expr_%d [shape=ellipse style=filled fillcolor=lightgoldenrod1 fontname=\"Courier\" label=\"" strFmt
            "\"]\n",
            id, strArg(expr.value.binding));
        break;
    case EXPR_LIT_INT:
        fprintf(stream,
            "Expr_%d [shape=note style=filled fillcolor=lightblue fontname=\"Courier\" label=\"%" PRIu64 "\"]\n",
            id, expr.value.lit_int);
        break;
    case EXPR_LIT_FLOAT:
        fprintf(stream, "Expr_%d [shape=note style=filled fillcolor=lightblue fontname=\"Courier\" label=\"%lf\"]\n",
            id, expr.value.lit_float);
        break;
    case EXPR_LIT_CHAR:
        fprintf(stream, "Expr_%d [shape=note style=filled fillcolor=lightblue fontname=\"Courier\" label=\"'%c'\"]\n",
            id, expr.value.lit_char);
        break;
    case EXPR_LIT_STR:
        fprintf(stream,
            "Expr_%d [shape=note style=filled fillcolor=lightblue fontname=\"Courier\" label=\"\\\"" strFmt
            "\\\"\"]\n",
            id, strArg(expr.value.lit_str));
        break;
    case EXPR_FUNCALL:
        {
            fprintf(stream,
                "Expr_%d [shape=hexagon style=filled fillcolor=lightpink fontname=\"Courier\" label=\"" strFmt "\"]\n",
                id, strArg(expr.value.funcall->name));

            int argIds[32], argCount = 0;     // lightweight static array for common cases

            for (FuncallArg* arg = expr.value.funcall->args; arg != NULL; arg = arg->next) {
                int childId = dumpExprAsAST(stream, arg->value, counter);
                fprintf(stream, "Expr_%d -> Expr_%d\n", id, childId);
                argIds[argCount++] = childId;
            }

            if (argCount > 0) {
                fprintf(stream, "{rank=same; Expr_%d", id);
                for (int i = 0; i < argCount; i++)
                    fprintf(stream, "; Expr_%d", argIds[i]);
                fprintf(stream, "}\n");
            }

            break;
        }

    case EXPR_REG:
        fprintf(stream,
            "Expr_%d [shape=cylinder style=filled fillcolor=lightyellow fontname=\"Courier\" label=\"%s\"]\n", id,
            getRegName(expr.value.reg_id));
        break;
    }

    return id;
}

int dumpStmtNodeAsAST(FILE* stream, StmtNode* stmtNode, int* counter, int* blockNo)
{
    // Encapsulated blocks!
    // Represent as a subgraph!
    int id      = (*counter)++;
    int blockid = (*blockNo)++;

    fprintf(stream, "subgraph cluster_%d {\n", id);
    fprintf(stream, "label=\"Code Block %d \";\n", blockid);
    fprintf(stream, "style=filled;\n");
    fprintf(stream, "color=gray;\n");
    fprintf(stream, "fillcolor=whitesmoke;\n");
    fprintf(stream, "fontname=\"Courier\";\n");

    int block_id = -1;
    for (; stmtNode; stmtNode = stmtNode->next) {
        int next_id = dumpStatementAsAST(stream, stmtNode->statement, counter, blockNo);

        if (block_id >= 0) {
            fprintf(stream, "Expr_%d -> Expr_%d;\n", block_id, next_id);
        }

        block_id = next_id;
    }

    fprintf(stream, "}\n");
    return id;
}

int dumpStatementAsAST(FILE* stream, Stmt statement, int* counter, int* blockNo)
{
    // statement formats in dot notation!
    switch (statement.kind) {
    case STMT_INST:
        {
            int id                = (*counter)++;
            Opcode type           = statement.value.inst.type;
            OpcodeDetails details = getOpcodeDetails(type);

            fprintf(stream,
                "Expr_%d [shape=component style=filled fillcolor=palegreen fontname=\"Courier\" label=\"%s\"]\n",
                id, details.name);

            int op1 = -1, op2 = -1;

            if (details.has_operand) {
                op1 = dumpExprAsAST(stream, statement.value.inst.operand, counter);
                fprintf(stream, "Expr_%d -> Expr_%d [style=dotted]\n", id, op1);
            }

            if (details.has_operand2) {
                op2 = dumpExprAsAST(stream, statement.value.inst.operand2, counter);
                fprintf(stream, "Expr_%d -> Expr_%d [style=dotted]\n", id, op2);
            }

            if (op1 != -1 || op2 != -1) {
                fprintf(stream, "{rank=same; Expr_%d", id);
                if (op1 != -1)
                    fprintf(stream, "; Expr_%d", op1);
                if (op2 != -1)
                    fprintf(stream, "; Expr_%d", op2);
                fprintf(stream, "}\n");
            }
            return id;
        }
        break;

    case STMT_LABEL:
        {
            int id      = (*counter)++;
            String name = statement.value.label.name;
            fprintf(stream,
                "Expr_%d [shape=tab style=filled fillcolor=lightpink fontname=\"Courier\" label=\"" strFmt "\"]\n", id,
                strArg(name));
            return id;
        }
        break;
    case STMT_CONST:
        {
            int id      = (*counter)++;
            String name = statement.value.constant.name;
            Expr value  = statement.value.constant.value;

            fprintf(stream,
                "Expr_%d [shape=note style=filled fillcolor=thistle fontname=\"Courier\" label=\"CONST " strFmt "\"]\n",
                id, strArg(name));

            int childId = dumpExprAsAST(stream, value, counter);
            fprintf(stream, "Expr_%d -> Expr_%d [style=dotted]\n", id, childId);
            fprintf(stream, "{rank=same; Expr_%d; Expr_%d}\n", id, childId);

            return id;
        }
        break;

    case STMT_INCLUDE:
        {
            int childId = (*counter)++;
            int id      = (*counter)++;

            String path = statement.value.include.path;
            fprintf(stream,
                "Expr_%d [shape=folder style=filled fillcolor=wheat fontname=\"Courier\" label=\"%%INCLUDE\"]\n", id);
            fprintf(stream, "Expr_%d [shape=box style=filled fillcolor=wheat fontname=\"Courier\" label=\"" strFmt "\"]\n",
                childId, strArg(path));
            fprintf(stream, "Expr_%d -> Expr_%d [style=dotted]\n", id, childId);
            fprintf(stream, "{rank=same; Expr_%d; Expr_%d}\n", id, childId);

            return id;
        }
        break;
    case STMT_ENTRY:
        {
            int id = (*counter)++;
            fprintf(stream, "Expr_%d [shape=pentagon style=filled fillcolor=plum1 fontname=\"Courier\" label=\"ENTRY\"]\n",
                id);
            int childId = dumpExprAsAST(stream, statement.value.entry.value, counter);
            fprintf(stream, "Expr_%d -> Expr_%d [style=dotted]\n", id, childId);
            return id;
        }
        break;

    case STMT_BLOCK:
    case STMT_SCOPE:
        {
            int oldCnt = dumpStmtNodeAsAST(stream, statement.value.block, counter, blockNo);
            fprintf(
                stream,
                "Expr_%d [shape=box3d style=filled fillcolor=aquamarine fontname=\"Courier\" label=\" Code Block %d  \"]\n",
                oldCnt, (*blockNo) - 1);
            return oldCnt;
        }
        break;

    default:
        {
            assert(false && "dumpStatementAsAST: unreachable");
            exit(1);
        }
    }
}

void generateASTPng(String inputFilePath, StmtNode* start)
{
    String tmp = inputFilePath;
    // trim untill last occurance of a '.' in the file name
    // i.e. file extension
    while (tmp.data[tmp.length - 1] != '.') {
        tmp.length -= 1;
    }
    // discard the '.' as well.
    tmp.length -= 1;
    // if empty, use default name 'a'
    if (tmp.length == 0) {
        tmp.data   = "a";
        tmp.length = 1;
    }

    char buffer[100];
    snprintf(buffer, sizeof(buffer), strFmt ".dot", strArg(tmp));
    FILE* out   = openFile(buffer, "w");
    int ID      = 0;
    int BlockNo = 0;
    // start dumping to the dot file, uses graphviz dot notation!
    fprintf(out,
        "digraph " strFmt " {\n"
        "nodesep=0.8;\n"
        "ranksep=0.5;\n",
        strArg(tmp));
    dumpStmtNodeAsAST(out, start, &ID, &BlockNo);
    fprintf(out, "}\n");

    closeFile(out, buffer);

    // try running the 'dot' command to generate the png, do nothing on failure.
    snprintf(buffer, sizeof(buffer), "dot  -Tpng -o " strFmt ".png " strFmt ".dot", strArg(tmp), strArg(tmp));
    if (system(buffer) != 0) {
        // displayMsgWithExit("Disassembly Failed");
        // printf("Failed to generate AST image, try using 'dot' command or check online\n");
    }
}
Binding* resolveBindingLocalScope(Scope* scope, String name)
{
    for (uint64 i = 0; i < scope->bindingsCnt; ++i) {
        if (compareStr(scope->bindings[i].name, name)) {
            return &scope->bindings[i];
        }
    }

    return NULL;
}

void pushUnresolvedOperand(Sasm* sasm, InstAddr addr, Expr expr, FileLocation location)
{
    assert(sasm->symbolsCount < LABELS_CAPACITY);
    sasm->symbols[sasm->symbolsCount++] = (UnresolvedOperand) {
        .addr     = addr,
        .expr     = expr,
        .location = location,
        .scope    = sasm->scope
    };
}

void pushIncludePath(Sasm* sasm, const char* path)
{
    assert(sasm->includePathsCnt < INCLUDE_PATHS_CAPACITY);
    sasm->includePaths[sasm->includePathsCnt++] = convertCstrToStr(path);
}

void bindUnresolvedLocalScope(Scope* scope, String name, BindingType type, FileLocation location)
{
    assert(scope->bindingsCnt < BINDINGS_CAPACITY);

    Binding* existing = resolveBindingLocalScope(scope, name);
    if (existing) {
        fprintf(stderr,
            FLFmt ": ERROR: name `" strFmt "` is already bound\n",
            FLArg(location),
            strArg(name));
        fprintf(stderr,
            FLFmt ": NOTE: first binding is located here\n",
            FLArg(existing->location));
        exit(1);
    }

    scope->bindings[scope->bindingsCnt++] = (Binding) {
        .name      = name,
        .status    = BIND_STATUS_DEFERRED,
        .type      = type,
        .location  = location,
        .value.u64 = 0
    };
}

void bindExprLocalScope(Scope* scope, String name, Expr expr, FileLocation location)
{
    assert(scope->bindingsCnt < BINDINGS_CAPACITY);

    Binding* existing = resolveBindingLocalScope(scope, name);
    if (existing) {
        fprintf(stderr,
            FLFmt ": ERROR: name `" strFmt "` is already bound\n",
            FLArg(location),
            strArg(name));
        fprintf(stderr,
            FLFmt ": NOTE: first binding is located here\n",
            FLArg(existing->location));
        exit(1);
    }

    scope->bindings[scope->bindingsCnt++] = (Binding) {
        .name     = name,
        .expr     = expr,
        .location = location,
    };
}

Binding* resolveBinding(Sasm* sasm, String name)
{
    for (Scope* scope = sasm->scope; scope != NULL; scope = scope->previous) {
        Binding* binding = resolveBindingLocalScope(scope, name);
        if (binding) {
            return binding;
        }
    }

    return NULL;
}

EvalResult evaluateBinding(Sasm* sasm, Binding* binding)
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
        fprintf(stderr, FLFmt ": ERROR: cycling binding definition.\n",
            FLArg(binding->location));
        exit(1);
    case BIND_STATUS_EVALUATED:
        return resultOK(binding->value, binding->type);
    case BIND_STATUS_DEFERRED:
        return resultUnresolved(binding);

    default:
        assert(false && "evaluateBinding: unreachable");
        exit(1);
    }
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
        assert(false && "type_name: unreachable");
        exit(1);
    }
}

QuadWord pushStringToMemory(Sasm* sasm, String str)
{
    assert(sasm->memorySize + str.length <= MEMORY_CAPACITY);

    QuadWord result = quadwordFromU64(sasm->memorySize);
    memcpy(sasm->memory + sasm->memorySize, str.data, str.length);
    sasm->memorySize += str.length;

    if (sasm->memorySize > sasm->memoryCapacity) {
        sasm->memoryCapacity = sasm->memorySize;
    }

    sasm->stringLens[sasm->strLensCnt++] = (StringLength) {
        .addr   = result.u64,
        .length = str.length,
    };

    return result;
}

bool getStrLenByAddr(Sasm* sasm, InstAddr addr, QuadWord* length)
{
    for (uint64 i = 0; i < sasm->strLensCnt; ++i) {
        if (sasm->stringLens[i].addr == addr) {
            if (length) {
                *length = quadwordFromU64(sasm->stringLens[i].length);
            }
            return true;
        }
    }

    return false;
}

FuncallArg* parseFuncallArgs(Arena* arena, Tokenizer* tokenizer, FileLocation location)
{
    // split arguments from single comma seperated string to linked list of strings.
    Token token = { 0 };

    if (!moveSasmTokenizerToNextToken(tokenizer, &token, location) || token.type != TOKEN_TYPE_OPEN_PAREN) {
        fprintf(stderr, FLFmt ": ERROR: expected %s\n",
            FLArg(location),
            getTokenName(TOKEN_TYPE_OPEN_PAREN));
        exit(1);
    }

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

        if (!moveSasmTokenizerToNextToken(tokenizer, &token, location)) {
            fprintf(stderr, FLFmt ": ERROR: expected %s or %s\n",
                FLArg(location),
                getTokenName(TOKEN_TYPE_CLOSING_PAREN),
                getTokenName(TOKEN_TYPE_COMMA));
            exit(1);
        }
    } while (token.type == TOKEN_TYPE_COMMA);

    if (token.type != TOKEN_TYPE_CLOSING_PAREN) {
        fprintf(stderr, FLFmt ": ERROR: expected %s\n",
            FLArg(location),
            getTokenName(TOKEN_TYPE_CLOSING_PAREN));
        exit(1);
    }

    return first;
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

void checkFuncArgs(Funcall* funcall, uint64 expected_arity, FileLocation location)
{
    // ensure arg count matches the expected count!
    const uint64 actual_arity = getFunCallArgCnt(funcall->args);
    if (actual_arity != expected_arity) {
        fprintf(stderr, FLFmt ": ERROR: " strFmt "() expects %" PRIu64 " but got %" PRIu64,
            FLArg(location),
            strArg(funcall->name),
            expected_arity,
            actual_arity);
        exit(1);
    }
}

EvalResult resolveFuncall(Sasm* sasm, Expr expr, FileLocation location)
{
    // Individual Compile-Time-Functions implementation!
    if (compareStr(expr.value.funcall->name, convertCstrToStr("len"))) {
        checkFuncArgs(expr.value.funcall, 1, location);

        QuadWord addr     = { 0 };
        EvalResult result = evaluateExpression(
            sasm,
            expr.value.funcall->args->value,
            location);
        if (result.status == EVAL_STATUS_DEFERRED) {
            return result;
        }
        addr            = result.value;
        QuadWord length = { 0 };
        if (!getStrLenByAddr(sasm, addr.u64, &length)) {
            fprintf(stderr, FLFmt ": ERROR: Could not compute the length of string at address %" PRIu64 "\n", FLArg(location), addr.u64);
            exit(1);
        }

        return resultOK(length, BIND_TYPE_UINT);
    }
    if (compareStr(expr.value.funcall->name, convertCstrToStr("res"))) {
        checkFuncArgs(expr.value.funcall, 1, location);

        QuadWord addr     = { 0 };
        EvalResult result = evaluateExpression(
            sasm,
            expr.value.funcall->args->value,
            location);

        assert(sasm->memorySize + result.value.u64 <= MEMORY_CAPACITY);

        addr = quadwordFromU64(sasm->memorySize);
        sasm->memorySize += result.value.u64;

        if (sasm->memorySize > sasm->memoryCapacity) {
            sasm->memoryCapacity = sasm->memorySize;
        }

        return resultOK(addr, BIND_TYPE_UINT);
    }
    if (compareStr(expr.value.funcall->name, convertCstrToStr("ref"))) {
        checkFuncArgs(expr.value.funcall, 1, location);

        if (expr.value.funcall->args->value.type != EXPR_REG) {
            fprintf(stderr, FLFmt ": ERROR: ref expects a register ", FLArg(location));
        }

        EvalResult result = evaluateExpression(
            sasm,
            expr.value.funcall->args->value,
            location);
        return result;
    }
    if (compareStr(expr.value.funcall->name, convertCstrToStr("val"))) {
        checkFuncArgs(expr.value.funcall, 1, location);

        if (expr.value.funcall->args->value.type != EXPR_REG) {
            fprintf(stderr, FLFmt ": ERROR: val expects a register ", FLArg(location));
        }

        EvalResult result = evaluateExpression(
            sasm,
            expr.value.funcall->args->value,
            location);

        if (result.status == EVAL_STATUS_DEFERRED) {
            return result;
        }
        uint64 val = result.value.u64 + REG_COUNT;
        return resultOK(
            quadwordFromU64(val), BIND_TYPE_UINT);
    }
    displayErrorDetailsWithExit(location, "Unknown translation time function", expr.value.funcall->name);
    exit(1);
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

EvalResult evaluateExpression(Sasm* sasm, Expr expr, FileLocation location)
{
    // Resolve Operands!
    // BeforeExpressionParse();
    EvalResult res;
    switch (expr.type) {
    case EXPR_LIT_INT:
        res = resultOK(
            quadwordFromU64(expr.value.lit_int),
            BIND_TYPE_UINT);
        break;

    case EXPR_LIT_FLOAT:
        res = resultOK(
            quadwordFromF64(expr.value.lit_float),
            BIND_TYPE_FLOAT);
        break;

    case EXPR_LIT_CHAR:
        res = resultOK(
            quadwordFromU64(expr.value.lit_char),
            BIND_TYPE_UINT);
        break;

    case EXPR_LIT_STR:
        res = resultOK(
            pushStringToMemory(sasm, expr.value.lit_str),
            BIND_TYPE_MEM_ADDR);
        break;

    case EXPR_FUNCALL:
        res = resolveFuncall(sasm, expr, location);
        break;

    case EXPR_BINDING:

        String name      = expr.value.binding;
        Binding* binding = resolveBinding(sasm, name);
        if (binding == NULL) {
            fprintf(stderr, FLFmt ": ERROR: couldnt find binding `" strFmt "`.\n",
                FLArg(location), strArg(name));
            exit(1);
        }

        res = evaluateBinding(sasm, binding);
        break;

    case EXPR_REG:
        res = resultOK(
            quadwordFromU64(expr.value.reg_id),
            BIND_TYPE_UINT);
        break;

    default:
        assert(false && "evaluateExpression: unreachable");
        exit(1);
    }
    // AfterExpressionParse();
    return res;
}
void BeforeLineRead();

void AfterLineRead();

void BeforeFileProcessing(String inputFilePath)
{
    (void)inputFilePath;
}

void AfterFileProcessing(String inputFilePath, StmtNode* codeBlockBegin)
{
    generateASTPng(inputFilePath, codeBlockBegin);
}

void BeforeAssembly();

void AfterAssembly();

void BeforeStatementParse();

void AfterStatementParse();

void BeforeExpressionParse();

void AfterExpressionParse();

bool loadSasmFileIntoSasmLexer(SasmLexer* lineInterpreter, Arena* arena, String filePath)
{
    assert(lineInterpreter);
    if (loadFileIntoRegionStr(arena, filePath, &lineInterpreter->source) < 0) {
        return false;
    }

    lineInterpreter->location.filePath = filePath;

    return true;
}

bool fetchCachedLineFromSasmLexer(SasmLexer* lineInterpreter, Line* output)
{
    // if there is cached line, return as is, else read next line
    // and return that
    if (lineInterpreter->hasCachedToken) {
        if (output) {
            *output = lineInterpreter->cachedToken;
        }
        return true;
    }

    String line = { 0 };
    do {
        line = trim(splitStrByChar(&lineInterpreter->source, '\n'));
        line = trim(splitStrByChar(&line, COMMENT_SYMBOL));
        lineInterpreter->location.lineNumber += 1;
    } while (line.length == 0 && lineInterpreter->source.length > 0);

    if (line.length == 0 && lineInterpreter->source.length == 0) {
        return false;
    }

    // BeforeLineRead();
    Line result     = { 0 };
    result.location = lineInterpreter->location;
    if (startsWith(line, convertCstrToStr("%"))) {
        splitStrByLen(&line, 1);
        result.kind                 = LINE_DIRECTIVE;
        result.value.directive.name = trim(splitStrByChar(&line, ' '));
        result.value.directive.body = trim(line);
    } else if (endsWith(line, convertCstrToStr(":"))) {
        result.kind             = LINE_LABEL;
        result.value.label.name = trim(splitStrByChar(&line, ':'));
        // printString(result.value.label.name);
    } else {
        result.kind                      = LINE_INSTRUCTION;
        result.value.instruction.name    = trim(splitStrByChar(&line, ' '));
        result.value.instruction.operand = trim(line);
    }

    if (output) {
        *output = result;
    }

    // AfterLineRead();

    lineInterpreter->hasCachedToken = true;
    lineInterpreter->cachedToken    = result;

    return true;
}

bool moveSasmLexerToNextLine(SasmLexer* lineInterpreter, Line* output)
{
    // if this line is not the last, remove it from the cache
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

bool fetchCachedSasmTokenFromSasmTokenizer(Tokenizer* tokenizer, Token* output, FileLocation location)
{
    if (tokenizer->hasCachedToken) {
        if (output) {
            *output = tokenizer->cachedToken;
        }
        return true;
    }

    tokenizer->source = ltrim(tokenizer->source);

    if (tokenizer->source.length == 0) {
        return false;
    }

    Token token = { 0 };
    switch (*tokenizer->source.data) {
    case '(':
        {
            token.type = TOKEN_TYPE_OPEN_PAREN;
            token.text = splitStrByLen(&tokenizer->source, 1);
        }
        break;

    case ')':
        {
            token.type = TOKEN_TYPE_CLOSING_PAREN;
            token.text = splitStrByLen(&tokenizer->source, 1);
        }
        break;

    case ',':
        {
            token.type = TOKEN_TYPE_COMMA;
            token.text = splitStrByLen(&tokenizer->source, 1);
        }
        break;

    case '"':
        {
            splitStrByLen(&tokenizer->source, 1);

            uint64 index = 0;

            if (getIndexOf(tokenizer->source, '"', &index)) {
                String text = splitStrByLen(&tokenizer->source, index);
                splitStrByLen(&tokenizer->source, 1);
                token.type = TOKEN_TYPE_STR;
                token.text = text;
            } else {
                fprintf(stderr, FLFmt ": ERROR: Could not find closing \"\n",
                    FLArg(location));
                exit(1);
            }
        }
        break;

    case '\'':
        {
            splitStrByLen(&tokenizer->source, 1);

            uint64 index = 0;

            if (getIndexOf(tokenizer->source, '\'', &index)) {
                String text = splitStrByLen(&tokenizer->source, index);
                splitStrByLen(&tokenizer->source, 1);
                token.type = TOKEN_TYPE_CHAR;
                token.text = text;
            } else {
                fprintf(stderr, FLFmt ": ERROR: Could not find closing \'\n",
                    FLArg(location));
                exit(1);
            }
        }
        break;

    default:
        {
            if (isalphabet(*tokenizer->source.data)) {
                token.type = TOKEN_TYPE_NAME;
                token.text = splitStrByCondition(&tokenizer->source, isName);
            } else if (is_digit(*tokenizer->source.data) || *tokenizer->source.data == '-') {
                token.type = TOKEN_TYPE_NUMBER;
                token.text = splitStrByCondition(&tokenizer->source, isNumber);
            } else if (tokenizer->source.length >= 3 && *tokenizer->source.data == '[' && tokenizer->source.data[3] == ']') {
                if (tokenizer->source.length - 2 < 2) {
                    fprintf(stderr, FLFmt ": ERROR: Check register name %c\n",
                        FLArg(location), *tokenizer->source.data);
                }
                splitStrByLen(&tokenizer->source, 1);
                uint64 index = 0;
                if (getIndexOf(tokenizer->source, ']', &index)) {
                    token.type = TOKEN_TYPE_REGISTER;
                    token.text = splitStrByLen(&tokenizer->source, index);
                    splitStrByLen(&tokenizer->source, 1);
                } else {
                    fprintf(stderr, FLFmt ": ERROR: Could not find closing \'\n",
                        FLArg(location));
                    exit(1);
                }
            } else {
                fprintf(stderr, FLFmt ": ERROR: Unknown token starts with %c\n",
                    FLArg(location), *tokenizer->source.data);
                exit(1);
            }
        }
    }

    tokenizer->hasCachedToken = true;
    tokenizer->cachedToken    = token;

    if (output) {
        *output = token;
    }

    return true;
}

bool moveSasmTokenizerToNextToken(Tokenizer* tokenizer, Token* token, FileLocation location)
{
    if (fetchCachedSasmTokenFromSasmTokenizer(tokenizer, token, location)) {
        tokenizer->hasCachedToken = false;
        return true;
    }

    return false;
}

Tokenizer loadStringIntoTokenizer(String source)
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
        {
            assert(false && "getTokenName: unreachable");
            exit(1);
        }
    }
}

void confirmNoMoreTokensRemain(Tokenizer* tokenizer, FileLocation location)
{
    Token token = { 0 };
    if (moveSasmTokenizerToNextToken(tokenizer, &token, location)) {
        fprintf(stderr, FLFmt ": ERROR: unexpected token `" strFmt "`\n",
            FLArg(location), strArg(token.text));
        exit(1);
    }
}

void pushStatementIntoBlock(Arena* arena, CodeBlock* list, Stmt statement)
{
    assert(list);
    StmtNode* stmtNode  = region_alloc(arena, sizeof(StmtNode));
    stmtNode->statement = statement;

    if (list->end == NULL) {
        assert(list->begin == NULL);
        list->begin = stmtNode;
        list->end   = stmtNode;
    } else {
        assert(list->begin != NULL);
        list->end->next = stmtNode;
        list->end       = stmtNode;
    }
    // printf("%d %d",list->end->statement.kind,list->end==NULL);
    // printString(list->end->statement.value.as_bind_label.name);
}

Expr parseExprFromStr(Arena* arena, String source, FileLocation location)
{
    Tokenizer tokenizer = loadStringIntoTokenizer(source);
    Expr result         = parsePrimaryOfSasmTokens(arena, &tokenizer, location);

    return result;
}

static Expr parseNumFromSasmTokens(Arena* arena, Tokenizer* tokenizer, FileLocation location)
{
    Token token = { 0 };

    if (!moveSasmTokenizerToNextToken(tokenizer, &token, location)) {
        fprintf(stderr, FLFmt ": ERROR: Cannot parse empty expression\n",
            FLArg(location));
        exit(1);
    }

    Expr result = { 0 };

    if (token.type == TOKEN_TYPE_NUMBER) {
        String text      = token.text;

        const char* cstr = convertStrToArenaCstr(arena, text);
        char* endptr     = 0;

        if (startsWith(text, convertCstrToStr("0x"))) {
            result.value.lit_int = strtoull(cstr, &endptr, 16);
            if ((uint64)(endptr - cstr) != text.length) {
                fprintf(stderr, FLFmt ": ERROR: `" strFmt "` is not a hex literal\n",
                    FLArg(location), strArg(text));
                exit(1);
            }

            result.type = EXPR_LIT_INT;
        } else {
            result.value.lit_int = strtoull(cstr, &endptr, 10);
            if ((uint64)(endptr - cstr) != text.length) {
                result.value.lit_float = strtod(cstr, &endptr);
                if ((uint64)(endptr - cstr) != text.length) {
                    fprintf(stderr, FLFmt ": ERROR: `" strFmt "` is not a number literal\n",
                        FLArg(location), strArg(text));
                } else {
                    result.type = EXPR_LIT_FLOAT;
                }
            } else {
                result.type = EXPR_LIT_INT;
            }
        }
    } else {
        fprintf(stderr, FLFmt ": ERROR: expected %s but got %s",
            FLArg(location),
            getTokenName(TOKEN_TYPE_NUMBER),
            getTokenName(token.type));
        exit(1);
    }

    return result;
}

void parseSasmDirectiveFromLine(Arena* arena, SasmLexer* lineInterpreter, CodeBlock* output)
{
    Line line = { 0 };

    if (!moveSasmLexerToNextLine(lineInterpreter, &line) || line.kind != LINE_DIRECTIVE) {
        fprintf(stderr, FLFmt ": ERROR: expected a directive line\n",
            FLArg(lineInterpreter->location));
        exit(1);
    }

    FileLocation location = line.location;
    String name           = line.value.directive.name;
    String body           = line.value.directive.body;
    // printString(body);

    if (compareStr(name, convertCstrToStr("include"))) {
        Stmt statement     = { 0 };
        statement.location = location;
        statement.kind     = STMT_INCLUDE;
        Expr path          = parseExprFromStr(arena, body, line.location);

        if (path.type != EXPR_LIT_STR) {
            fprintf(stderr, FLFmt "ERROR: expected string literal as path for %%include directive\n",
                FLArg(location));
            exit(1);
        }

        statement.value.include.path = path.value.lit_str;

        pushStatementIntoBlock(arena, output, statement);
    } else if (compareStr(name, convertCstrToStr("bind"))) {
        Stmt statement      = { 0 };
        statement.location  = location;
        statement.kind      = STMT_CONST;

        Tokenizer tokenizer = loadStringIntoTokenizer(body);
        Expr bindingName    = parsePrimaryOfSasmTokens(arena, &tokenizer, location);
        if (bindingName.type != EXPR_BINDING) {
            fprintf(stderr, FLFmt ": ERROR: expected binding name for %%bind binding\n",
                FLArg(location));
            exit(1);
        }
        statement.value.constant.name  = bindingName.value.binding;

        statement.value.constant.value = parsePrimaryOfSasmTokens(arena, &tokenizer, location);
        confirmNoMoreTokensRemain(&tokenizer, location);

        pushStatementIntoBlock(arena, output, statement);
    } else if (compareStr(name, convertCstrToStr("entry"))) {
        body              = trim(body);
        bool inline_entry = false;

        if (endsWith(body, convertCstrToStr(":"))) {
            splitStrByLenReversed(&body, 1);
            inline_entry = true;
        }

        Expr expr                   = parseExprFromStr(arena, body, line.location);

        Stmt statement              = { 0 };
        statement.location          = location;
        statement.kind              = STMT_ENTRY;
        statement.value.entry.value = expr;
        pushStatementIntoBlock(arena, output, statement);

        if (inline_entry) {
            Stmt statement     = { 0 };
            statement.location = location;
            statement.kind     = STMT_LABEL;

            if (expr.type != EXPR_BINDING) {
                fprintf(stderr, FLFmt ": ERROR: expected binding name for a label\n",
                    FLArg(location));
                exit(1);
            }

            statement.value.label.name = expr.value.binding;
            // printf("%d", statement.kind);
            pushStatementIntoBlock(arena, output, statement);
        }
    } else if (compareStr(name, convertCstrToStr("scope"))) {
        Stmt statement        = { 0 };
        statement.location    = location;
        statement.kind        = STMT_SCOPE;
        statement.value.scope = getCodeBlockFromLines(arena, lineInterpreter).begin;

        if (!moveSasmLexerToNextLine(lineInterpreter, &line) || line.kind != LINE_DIRECTIVE || !compareStr(line.value.directive.name, convertCstrToStr("end"))) {
            fprintf(stderr, FLFmt ": ERROR: expected `%%end` directive at the end of the `%%scope` block\n",
                FLArg(lineInterpreter->location));
            fprintf(stderr, FLFmt ": NOTE: the %%scope block starts here\n",
                FLArg(statement.location));
            exit(1);
        }
        pushStatementIntoBlock(arena, output, statement);
    } else {
        fprintf(stderr, FLFmt ": ERROR: unknown directive `" strFmt "`\n",
            FLArg(line.location), strArg(name));
        exit(1);
    }
}

String ParseStrFromSasmTokens(Tokenizer* tokenizer, FileLocation location)
{
    Token token = { 0 };
    if (!moveSasmTokenizerToNextToken(tokenizer, &token, location) || token.type != TOKEN_TYPE_STR) {
        fprintf(stderr, FLFmt ": ERROR: expected token %s\n",
            FLArg(location), getTokenName(token.type));
        exit(1);
    }

    return token.text;
}

Expr parsePrimaryOfSasmTokens(Arena* arena, Tokenizer* tokenizer, FileLocation location)
{
    Token token = { 0 };

    if (!fetchCachedSasmTokenFromSasmTokenizer(tokenizer, &token, location)) {
        fprintf(stderr, FLFmt ": ERROR: Cannot parse empty expression\n",
            FLArg(location));
        exit(1);
    }

    Expr result = { 0 };

    switch (token.type) {
    case TOKEN_TYPE_STR:
        result.type          = EXPR_LIT_STR;
        result.value.lit_str = ParseStrFromSasmTokens(tokenizer, location);
        break;

    case TOKEN_TYPE_CHAR:
        moveSasmTokenizerToNextToken(tokenizer, NULL, location);

        if (token.text.length != 1) {
            fprintf(stderr, FLFmt ": ERROR: the length of char literal has to be exactly one\n",
                FLArg(location));
            exit(1);
        }

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

        if (!moveSasmTokenizerToNextToken(tokenizer, &token, location) || token.type != TOKEN_TYPE_CLOSING_PAREN) {
            fprintf(stderr, FLFmt ": ERROR: expected `%s`\n",
                FLArg(location), getTokenName(TOKEN_TYPE_CLOSING_PAREN));
            exit(1);
        }
        return expr;
        break;
    case TOKEN_TYPE_REGISTER:
        moveSasmTokenizerToNextToken(tokenizer, NULL, location);
        String str = token.text;
        // printString(token.text);
        switch (str.data[0]) {
        case 'H':
            if (str.data[1] > '1') {
                fprintf(stderr, FLFmt ": ERROR: Invalid register %s\n",
                    FLArg(location), str.data);
                exit(1);
            }
            result.value.reg_id = (uint64)(REG_H0 + str.data[1] - '0');
            break;
        case 'I':
            if (str.data[1] > '1') {
                fprintf(stderr, FLFmt ": ERROR: Invalid register %s\n",
                    FLArg(location), str.data);
                exit(1);
            }
            result.value.reg_id = (uint64)(REG_I0 + str.data[1] - '0');
            break;
        case 'L':
            if (str.data[1] > '3') {
                fprintf(stderr, FLFmt ": ERROR: Invalid register %s\n",
                    FLArg(location), str.data);
                exit(1);
            }
            result.value.reg_id = (uint64)(REG_L0 + str.data[1] - '0');
            break;
        case 'P':
            if (str.data[1] > '3') {
                fprintf(stderr, FLFmt ": ERROR: Invalid register %s\n",
                    FLArg(location), str.data);
                exit(1);
            }
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
            fprintf(stderr, FLFmt ": ERROR: Invalid register %s\n",
                FLArg(location), str.data);
            exit(1);
        };

        result.type = EXPR_REG;

        break;
    case TOKEN_TYPE_COMMA:
    case TOKEN_TYPE_CLOSING_PAREN:
        fprintf(stderr, FLFmt ": ERROR: expected primary expression but found %s\n",
            FLArg(location), getTokenName(token.type));
        exit(1);
        break;

    default:
        assert(false && "parsePrimaryOfSasmTokens: unreachable");
        exit(1);
    }

    return result;
}

CodeBlock getCodeBlockFromLines(Arena* arena, SasmLexer* lineInterpreter)
{
    CodeBlock result = { 0 };

    Line line        = { 0 };
    while (fetchCachedLineFromSasmLexer(lineInterpreter, &line)) {
        const FileLocation location = line.location;
        Stmt statement              = { 0 };
        statement.location          = location;
        // BeforeStatementParse();
        switch (line.kind) {
        case LINE_INSTRUCTION:
            String name        = line.value.instruction.name;
            String operandList = line.value.instruction.operand;

            OpcodeDetails details;
            if (!getOpcodeDetailsFromName(name, &details)) {
                continue;
            }

            statement.kind            = STMT_INST;
            statement.value.inst.type = details.type;

            if (details.has_operand) {
                String opr1                  = trim(splitStrByChar(&operandList, ' '));
                // printString(opr1);
                Expr operand                 = parseExprFromStr(arena, opr1, location);
                statement.value.inst.operand = operand;
            }

            if (details.has_operand2) {
                String opr2                   = trim(operandList);
                // printString(opr2);
                Expr operand                  = parseExprFromStr(arena, opr2, location);
                statement.value.inst.operand2 = operand;
            }
            pushStatementIntoBlock(arena, &result, statement);
            moveSasmLexerToNextLine(lineInterpreter, NULL);
            // AfterStatementParse();
            break;

        case LINE_LABEL:
            Expr label = parseExprFromStr(arena, line.value.label.name, location);

            if (label.type != EXPR_BINDING) {
                displayErrorLocationWithExit(location, "expected binding name for a label");
            }

            statement.kind             = STMT_LABEL;
            statement.value.label.name = label.value.binding;
            pushStatementIntoBlock(arena, &result, statement);
            moveSasmLexerToNextLine(lineInterpreter, NULL);
            // AfterStatementParse();
            break;

        case LINE_DIRECTIVE:
            if (compareStr(line.value.directive.name, convertCstrToStr("end"))) {
                // AfterStatementParse();
                return result;
            }

            parseSasmDirectiveFromLine(arena, lineInterpreter, &result);
            // AfterStatementParse();
            continue;
        }
    }

    return result;
}

#endif