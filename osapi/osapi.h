/* os.h */
#ifndef OSAPI_1
#define OSAPI_1

#include <common/types.h>
/* write 1 char */
public
void store(file_discriptor fd, uint8 c);

/* read 1 char */
public
uint8 load(file_discriptor fd);

/* restart current process later */
public
void p_yield();

/* exit current process */
public
void p_exit();

#endif
