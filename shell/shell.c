#include "shell.h"

int main(void)
{

    store(1, 'H');
    store(1, 'i');
    store(1, '!');
    store(1, '\n');

    for (uint8 i = 0; i < 15; i++)
        store(1, '=');

    store(1, '\n');
    return 0;
}
