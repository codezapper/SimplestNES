// PPUCTRL 	$2000 	VPHB SINN 	NMI enable (V), PPU master/slave (P), sprite height (H), background tile select (B), sprite tile select (S), increment mode (I), nametable select (NN)
// PPUMASK 	$2001 	BGRs bMmG 	color emphasis (BGR), sprite enable (s), background enable (b), sprite left column enable (M), background left column enable (m), greyscale (G)
// PPUSTATUS 	$2002 	VSO- ---- 	vblank (V), sprite 0 hit (S), sprite overflow (O); read resets write pair for $2005/$2006
// OAMADDR 	$2003 	aaaa aaaa 	OAM read/write address
// OAMDATA 	$2004 	dddd dddd 	OAM data read/write
// PPUSCROLL 	$2005 	xxxx xxxx 	fine scroll position (two writes: X scroll, Y scroll)
// PPUADDR 	$2006 	aaaa aaaa 	PPU read/write address (two writes: most significant byte, least significant byte)
// PPUDATA 	$2007 	dddd dddd 	PPU data read/write
// OAMDMA 	$4014 	aaaa aaaa 	OAM DMA high address 


// $0000-$0FFF 	$1000 	Pattern table 0
// $1000-$1FFF 	$1000 	Pattern table 1
// $2000-$23FF 	$0400 	Nametable 0
// $2400-$27FF 	$0400 	Nametable 1
// $2800-$2BFF 	$0400 	Nametable 2
// $2C00-$2FFF 	$0400 	Nametable 3
// $3000-$3EFF 	$0F00 	Mirrors of $2000-$2EFF
// $3F00-$3F1F 	$0020 	Palette RAM indexes
// $3F20-$3FFF 	$00E0 	Mirrors of $3F00-$3F1F

// In addition, the PPU internally contains 256 bytes of memory known as Object Attribute Memory which determines how sprites are rendered. The CPU can manipulate this memory through memory mapped registers at OAMADDR ($2003), OAMDATA ($2004), and OAMDMA ($4014). OAM can be viewed as an array with 64 entries. Each entry has 4 bytes: the sprite Y coordinate, the sprite tile number, the sprite attribute, and the sprite X coordinate.
// Address Low Nibble 	Description
// $00, $04, $08, $0C 	Sprite Y coordinate
// $01, $05, $09, $0D 	Sprite tile #
// $02, $06, $0A, $0E 	Sprite attribute
// $03, $07, $0B, $0F 	Sprite X coordinate 

#include <string.h>

#include "ppu.h"
#include "rom.h"

extern struct ROM rom;
extern unsigned char RAM[0xFFFF];
unsigned char VRAM[0xFFFF];
unsigned char palette_table[32];

unsigned char ppuctrl;
unsigned char ppumask;
unsigned char ppustatus;
unsigned char oamaddr;
unsigned char oamdata[64*4];
unsigned char ppuscroll;
unsigned char ppuaddr;
unsigned char ppudata;
unsigned char oamdma;


void write_ppuctrl(unsigned char value) {
    ppuctrl = value;
}

void write_ppumask(unsigned char value) {
    ppumask = value;
}

unsigned char get_ppustatus() {
    return ppustatus;
}

void write_oamaddr(unsigned char value) {
    oamaddr = value;
}

unsigned char *read_oamdata() {
    return oamdata;
}

void write_oamdata(unsigned char *value) {
    memcpy(oamdata, value, sizeof(oamdata));
}

void write_ppudata(unsigned char *value) {
    int a = 0;
}

void write_ppuscroll(unsigned char *value) {
    int a = 0;
}

void write_ppuaddress(unsigned char *value) {
    int a = 0;
}

unsigned char *read_ppudata() {
    int a = 0;
}

void ppu_clock() {
    
}