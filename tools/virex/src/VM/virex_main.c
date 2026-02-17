#define IMPL_VIREX_1
#define IMPL_KMALLOC_1
#define MEM_MANAGER_IMPL
#define STRING_VIEW_IMPL
#include "virex.h"
#include <common/memmanager.h>
#include <common/strings.h>

int main()
{
    virex_test();
    return 0;
}
