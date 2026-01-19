#include "shell.h"

int main(void)
{
    uint8 c = load(0);
    for (size_t i = 0; i < 15; i++)
        store(1, '=');

    printf("\nc = 0x%.02hhx = %c", c, c);
    printf("\nerrno = 0x%.02x\n", errno);

    return 0;
}
