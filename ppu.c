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

#define WIDTH		256
#define HEIGHT		240
#define _DATA_SIZE	WIDTH*HEIGHT*3

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

unsigned char _data[_DATA_SIZE];

unsigned char interrupt_occurred = 0;

SDL_Texture *texture = NULL;

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

void ppu_clock(int allowed_cycles) {
    
}

const int SCREEN_WIDTH = 256;
const int SCREEN_HEIGHT = 240;

SDL_Window* gWindow = NULL;
SDL_Renderer *gRenderer;
SDL_Surface* gScreenSurface = NULL;
SDL_Surface* gGameSurface = NULL;


unsigned char init_sdl()
{
	unsigned char success = 1;
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
	{
		printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
		success = 0;
	}
	else
	{
		SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &gWindow, &gRenderer );
        gScreenSurface = SDL_GetWindowSurface( gWindow );
	}

	return success;
}

void free_ppu()
{
	SDL_FreeSurface( gGameSurface );
	gGameSurface = NULL;
	SDL_DestroyWindow( gWindow );
	gWindow = NULL;
	SDL_Quit();
}

void set_pixel(unsigned char x, unsigned char y, int value) { //unsigned char r, unsigned char g, unsigned char b) {
	unsigned char R = PALETTE[value][0];
	unsigned char G = PALETTE[value][1];
	unsigned char B = PALETTE[value][2];

    SDL_SetRenderDrawColor(gRenderer, 255, R, G, B);
	SDL_RenderDrawPoint(gRenderer, x, y);

	// TODO: Render using a texture and updating it

	// int start_address = y * 3 * WIDTH + x * 3;
	// if ((start_address + 2) < _DATA_SIZE) {
	// 	_data[start_address] = R;
	// 	_data[start_address + 1] = G;
	// 	_data[start_address + 2] = B;
	// }
}

void show_tile(int bank, int tile_n, int start_x, int start_y) {
	int tile[16];
	for (int y = 0; y <= 7; y++) {
		unsigned char upper = VRAM[bank + tile_n * 16 + y];
		unsigned char lower = VRAM[bank + tile_n * 16 + y + 8];

		for (int x = 0; x <= 7; x++) {
			int value = ((1 & upper) << 1) | (1 & lower);
			upper >>= 1;
			lower >>= 1;
			set_pixel(x + start_x, y + start_y, value);
		}
	}
}

void init_ppu() {
	if( !init_sdl() )
	{
		printf( "Failed to initialize!\n" );
	}
    memcpy(VRAM, rom.chr_rom, 0x2000);
	memset(_data, 0, _DATA_SIZE);
    // texture = SDL_CreateTexture(gRenderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_TARGET, 8, 8);
	int tile_index = 0;
	for (int y = 0; y < HEIGHT; y+=8) {
		for (int x = 0; x < WIDTH; x+=8) {
			show_tile(1, tile_index, x, y);
			tile_index++;
		}
	}
	SDL_RenderPresent(gRenderer);
}
