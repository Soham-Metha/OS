/* hal.h */

#define GLYPH_W 8
#define GLYPH_H 16

typedef unsigned char uint8;
typedef unsigned short int uint16;
typedef unsigned int uint32;
typedef unsigned long long int uint64;

void hal_put_pixel(int x, int y, uint32 rgba);
void hal_clear(uint32 rgba);
void hal_present(void);
uint32 hal_get_width();
uint32 hal_get_height();

void draw_char(char c, int x, int y, uint32 fg, uint32 bg);
