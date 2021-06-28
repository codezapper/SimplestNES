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
void write_ppuctrl(unsigned char value);
void write_ppumask(unsigned char value);
unsigned char get_ppustatus();
void write_oamaddr(unsigned char value);
unsigned char *read_oamdata();
void write_oamdata(unsigned char value);
void write_ppudata(unsigned char value);
void write_ppuscroll(unsigned char value);
void write_ppuaddress(unsigned char value);
unsigned char get_ppudata();
