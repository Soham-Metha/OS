/* os.h */
#include <kernel/kernel.h>

/* write 1 char */
public
void store(file_discriptor fd, uint8 c);

/* read 1 char */
public
uint8 load(file_discriptor fd);
