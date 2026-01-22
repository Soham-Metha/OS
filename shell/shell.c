#include "shell.h"

int main(void)
{
    uint8 c = load(0);

    for (uint8 i = 0; i < 15; i++)
        store(1, '=');
    store(1, '\n');

    store(1, c);
    store(1, '\n');

    return 0;
}
