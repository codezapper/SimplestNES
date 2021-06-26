#define PPUCTRL 	0x2000
#define PPUMASK 	0x2001
#define PPUSTATUS 	0x2002
#define OAMADDR 	0x2003
#define OAMDATA 	0x2004
#define PPUSCROLL 	0x2005
#define PPUADDR 	0x2006
#define PPUDATA 	0x2007
#define OAMDMA 	    0x4014

void init_ppu();
void ppu_clock();
