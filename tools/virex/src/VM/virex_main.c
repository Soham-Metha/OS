#define IMPL_VIREX_1
#define IMPL_KMALLOC_1
#define MEM_MANAGER_IMPL
#define STRING_VIEW_IMPL
#include "../../../../common/memmanager.h"
#include "../../../../common/strings.h"
#include "virex.h"

void processFlag(const char* program, const char* flag, int* argc, char*** argv);
void __exec_sm(Vm* vm);

const char* inputFile  = NULL;
const char* outputFile = NULL;
char buffer[256];
int limit = -1;
int debug = 0;

int main(int argc, char** argv)
{
    static Vm vm = { 0 };
    loadStandardCallsIntoVm(&vm);

    const char* program = getNextCmdLineArg(&argc, &argv);

    while (argc > 0) {
        const char* flag = getNextCmdLineArg(&argc, &argv);
        processFlag(program, flag, &argc, &argv);
    }

    if (inputFile) {
        loadProgramIntoVm(&vm, inputFile);
        executeProgram(&vm, debug, -1);
    }
    return 0;
}

void processFlag(const char* program, const char* flag, int* argc, char*** argv)
{
    switch (flag[1]) {
    case 'l':
        limit = atoi(getNextCmdLineArg(argc, argv));
        return;
    case 'i':
        inputFile = getNextCmdLineArg(argc, argv);
        return;
    default:
        printf("Usage: %s [-l <limit>]\n", program);
        displayMsgWithExit("Unknown option provided.");
    }
}
