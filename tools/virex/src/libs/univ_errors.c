#include "univ_errors.h"
#include "univ_defs.h"

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
        assert(0 && "univ_errors : getNameOfError : Unreachable");
    }
}

void fileErrorDispWithExit(const char* message, const char* filePath)
{
    printf("ERROR : %s\n %s\n", message, filePath);
    exit(1);
}

void executionErrorWithExit(const VM_Error* error)
{
    printf("Error : %s\n", getNameOfError(error));
    exit(1);
}

void displayMsgWithExit(const char* message)
{
    printf("ERROR : %s\n", message);
    exit(1);
}

void displayStringMessageError(const char* msg, String_View str)
{
    printf("\n|   |                                                                                                                              |");
    printf("\n| W | ERROR | '%.*s' | %s", Str_Fmt(str), msg);

    for (size_t i = strlen(msg) + str.len; i < 110; i++)
        printf(" ");

    printf("|"
           "\n|   |                                                                                                                              |\n");
    exit(1);
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

void displayErrorDetailsWithExit(FileLocation location, const char* msg, String_View reason)
{
    printf(FLFmt ": ERROR: %s `%.*s`\n",
        FLArg(location),
        msg,
        Str_Fmt(reason));
    exit(1);
}

void displayErrorLocationWithExit(FileLocation location, const char* msg)
{
    printf("%.*s : %d : ERROR: %s \n",
        (int)location.filePath.len,
        location.filePath.data,
        location.lineNumber,
        msg);
    exit(1);
}

void displayCErrorDetailsWithExit(FileLocation location, const char* msg, const char* reason)
{
    printf(FLFmt ": ERROR: %s `%s`\n",
        FLArg(location),
        msg,
        reason);
    exit(1);
}
