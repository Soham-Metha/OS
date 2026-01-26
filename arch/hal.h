/* hal.h */

#include <common/types.h>

void hal_put_pixel(int x, int y, uint32 rgba);
void hal_clear(uint32 rgba);
void hal_present(void);
uint32 hal_get_width();
uint32 hal_get_height();
