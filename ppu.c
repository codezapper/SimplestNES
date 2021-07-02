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

#include <SDL.h>
#include <string.h>
#include <unistd.h>

#include "ppu.h"
#include "rom.h"
#include "utils.h"

#define WIDTH				256
#define HEIGHT				240

#define _DATA_SIZE			WIDTH*HEIGHT*3
#define CYCLES_PER_LINE		341
#define OAMSIZE				64*4

#define HIGH				1
#define LOW					0

extern struct ROM rom;
extern unsigned char RAM[0xFFFF];
extern int cycles;

int total_cycles = 0;

unsigned char VRAM[0xFFFF];

unsigned char ppuctrl = 0x80;
unsigned char ppumask = 0;
unsigned char ppustatus = 0x80;
unsigned char oamaddr = 0;
unsigned char oamdata[OAMSIZE];
unsigned char ppuscroll;
unsigned char ppuaddr;
unsigned char ppudata;
unsigned char oamdma;

unsigned char t_address_toggle = HIGH;
unsigned char t_scroll_toggle = HIGH;

uint16_t ppu_address = 0;

unsigned char interrupt_occurred = 0;

SDL_Renderer *renderer;
SDL_Window *window;
SDL_Texture *texture;

int current_line = 0;
int ppu_cycles = 0;

unsigned char PALETTE[0x40][3] = {
   {0x80, 0x80, 0x80}, {0x00, 0x3D, 0xA6}, {0x00, 0x12, 0xB0}, {0x44, 0x00, 0x96}, {0xA1, 0x00, 0x5E},
   {0xC7, 0x00, 0x28}, {0xBA, 0x06, 0x00}, {0x8C, 0x17, 0x00}, {0x5C, 0x2F, 0x00}, {0x10, 0x45, 0x00},
   {0x05, 0x4A, 0x00}, {0x00, 0x47, 0x2E}, {0x00, 0x41, 0x66}, {0x00, 0x00, 0x00}, {0x05, 0x05, 0x05},
   {0x05, 0x05, 0x05}, {0xC7, 0xC7, 0xC7}, {0x00, 0x77, 0xFF}, {0x21, 0x55, 0xFF}, {0x82, 0x37, 0xFA},
   {0xEB, 0x2F, 0xB5}, {0xFF, 0x29, 0x50}, {0xFF, 0x22, 0x00}, {0xD6, 0x32, 0x00}, {0xC4, 0x62, 0x00},
   {0x35, 0x80, 0x00}, {0x05, 0x8F, 0x00}, {0x00, 0x8A, 0x55}, {0x00, 0x99, 0xCC}, {0x21, 0x21, 0x21},
   {0x09, 0x09, 0x09}, {0x09, 0x09, 0x09}, {0xFF, 0xFF, 0xFF}, {0x0F, 0xD7, 0xFF}, {0x69, 0xA2, 0xFF},
   {0xD4, 0x80, 0xFF}, {0xFF, 0x45, 0xF3}, {0xFF, 0x61, 0x8B}, {0xFF, 0x88, 0x33}, {0xFF, 0x9C, 0x12},
   {0xFA, 0xBC, 0x20}, {0x9F, 0xE3, 0x0E}, {0x2B, 0xF0, 0x35}, {0x0C, 0xF0, 0xA4}, {0x05, 0xFB, 0xFF},
   {0x5E, 0x5E, 0x5E}, {0x0D, 0x0D, 0x0D}, {0x0D, 0x0D, 0x0D}, {0xFF, 0xFF, 0xFF}, {0xA6, 0xFC, 0xFF},
   {0xB3, 0xEC, 0xFF}, {0xDA, 0xAB, 0xEB}, {0xFF, 0xA8, 0xF9}, {0xFF, 0xAB, 0xB3}, {0xFF, 0xD2, 0xB0},
   {0xFF, 0xEF, 0xA6}, {0xFF, 0xF7, 0x9C}, {0xD7, 0xE8, 0x95}, {0xA6, 0xED, 0xAF}, {0xA2, 0xF2, 0xDA},
   {0x99, 0xFF, 0xFC}, {0xDD, 0xDD, 0xDD}, {0x11, 0x11, 0x11}, {0x11, 0x11, 0x11}
};


void write_ppuctrl(unsigned char value) {
	ppuctrl = value;
}

void write_ppumask(unsigned char value) {
    ppumask = value;
}

unsigned char read_ppustatus() {
	unsigned char current = ppustatus;
	clear_vblank();
	t_address_toggle = HIGH;
	t_scroll_toggle = HIGH;
    return current;
}

void write_oamaddr(unsigned char value) {
    oamaddr = value;
}

unsigned char read_oamdata() {
	if (check_bit(ppustatus, 7) == 0) {
		oamaddr++;
	}
    return oamdata[oamaddr];
}

void write_oamdata(unsigned char value) {
    oamdata[oamaddr] = value;
	oamaddr++;
}

void write_dma(unsigned char address, unsigned char value) {
	oamdata[address] = value;
}

void write_ppudata(unsigned char value) {
    ppudata = value;
	if (check_bit(ppuctrl, 2) == 1) {
		ppuaddr += 32;
	} else {
		ppuaddr++;
	}
}

void write_ppuscroll(unsigned char value) {
	if (t_scroll_toggle == HIGH) {
		t_scroll_toggle = LOW;
		ppuscroll = value << 8;
	} else {
		t_scroll_toggle = HIGH;
		ppuscroll |= value;
	}
}

void write_ppuaddress(unsigned char value) {
	if (t_address_toggle == HIGH) {
		t_address_toggle = LOW;
		ppu_address = value << 8;
	} else {
		t_address_toggle = HIGH;
		ppu_address |= value;
	}
}

unsigned char read_ppudata() {
	unsigned char value = VRAM[ppu_address];
	ppu_address++;
    return value;
}

void clear_screen() {
	SDL_SetRenderDrawColor(renderer, 255, 0, 0, 0);
    SDL_RenderClear(renderer);
}

void set_vblank() {
	ppustatus = set_bit(ppustatus, 7);
}

void clear_vblank() {
	ppustatus = clear_bit(ppustatus, 7);
	t_address_toggle = HIGH;
	t_scroll_toggle = HIGH;
}

int can_generate_nmi() {
	if (ppuctrl & 0x80) {
		return 1;
	}
	return 0;
}

unsigned char init_sdl()
{
	SDL_Init(SDL_INIT_VIDEO);
	SDL_CreateWindowAndRenderer(256, 240, 0, &window, &renderer);
	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, 256, 240);
}

void free_ppu()
{
	// SDL_FreeSurface( gGameSurface );
	// gGameSurface = NULL;
	// SDL_DestroyWindow( gWindow );
	// gWindow = NULL;
	SDL_Quit();
}

void draw_background() {

}

void init_ppu() {
	if( !init_sdl() )
	{
		printf( "Failed to initialize SDL!\n" );
	}
    memcpy(VRAM, rom.chr_rom, 0x1FFF);

	ppuctrl= 0x80;
}

void ppu_clock(int cpu_cycles) {
	total_cycles++;
	ppu_cycles++;

	if (ppu_cycles > 340) {
		ppu_cycles -= 341;
		current_line++;
	}

	if (current_line < 240) {
		draw_background();
	} else if (current_line == 241) {
		interrupt_occurred = NMI_INT;
		set_vblank();
	} else if (current_line == 261) {
		clear_vblank();
	}
}

