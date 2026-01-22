/* hal.h */

typedef unsigned char uint8;
typedef unsigned short int uint16;
typedef unsigned int uint32;
typedef unsigned long long int uint64;

void hal_put_pixel(int x, int y, uint32 rgba);
void hal_clear(uint32 rgba);
void hal_present(void);

void draw_char(char c, int x, int y, uint32 fg, uint32 bg);
