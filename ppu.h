#include "cpu.h"

#define PPUCTRL 	0x2000
#define PPUMASK 	0x2001
#define PPUSTATUS 	0x2002
#define OAMADDR 	0x2003
#define OAMDATA 	0x2004
#define PPUSCROLL 	0x2005
#define PPUADDR 	0x2006
#define PPUDATA 	0x2007
#define OAMDMA 	    0x4014

#define MASK_GREYSCALE      0
#define MASK_BG_LEFT        1
#define MASK_FG_LEFT        2
#define MASK_BG_ENABLE      3
#define MASK_FG_ENABLE      4

void init_ppu();
void ppu_clock(int cycles);
void write_ppuctrl(unsigned char value);
void write_ppumask(unsigned char value);
unsigned char read_ppustatus();
void write_oamaddr(unsigned char value);
unsigned char read_oamdata();
void write_oamdata(unsigned char value);
void write_ppudata(unsigned char value);
void write_ppuscroll(unsigned char value);
void write_v(unsigned char value);
unsigned char read_ppudata();
void clear_vblank();
void write_dma(unsigned char address, unsigned char value);
void draw_background(unsigned char nametable_id, int shift_x, int shift_y);
void update_palette();
unsigned char get_oamaddr();