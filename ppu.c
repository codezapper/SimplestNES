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

#define BG_ENABLE			3
#define SPRITES_ENABLE		4

#define BG_TILE_SELECT		4
#define FG_TILE_SELECT		3

#define VBLANK_BIT			7

#define FLIP_HORIZONTAL		6
#define FLIP_VERTICAL		7

extern struct ROM rom;
extern int cycles;

int total_cycles = 0;
int warmup_count = 0;
unsigned char write_enabled = 0;

unsigned char VRAM[0xFFFF];

unsigned char ppuctrl = 0x80;
unsigned char ppumask = 0;
unsigned char ppustatus = 0x0;
unsigned char oamaddr = 0;
unsigned char oamdata[OAMSIZE];
unsigned char ppuscroll;
unsigned char ppudata;
unsigned char oamdma;

unsigned char ppudata_buffer = 0;

// TODO:
// Use "v" as the VRAM address when reading/writing
// Use "t" as the latch for determining address
// Copy "t" to "v" when the frame starts

// QUOTE:
/*
$2000 updates the name table bits in the VRAM address. When accessing name tables, the address register has the following format: 0010NNYY YYYXXXXX
$2006 writes affect bits 0 through 13 (and clear bits 14 and 15), $2005 writes change the X and Y bits, and $2000 writes change the NN bits.
One weird aspect of the PPU is that it uses its addresses registers when rendering the image, which is why the scroll settings (set via $2000 and $2005) affect the address registers. Fir rendering purposes, the PPU address registers (t and v have the following format:

0yyyNNYY YYYXXXXX

XXXXX: coarse X scroll;
YYYYY: coarse Y scroll;
NN: name table;
yyy: fine Y scroll;
(the fine X scroll is kept elsewhere)

When the frame starts, the PPU will automatically copy the value from t to v and use that to determine what part of the background to render, as specified by the bits above.
*/

uint16_t t = 0;
uint16_t v = 0;
unsigned char t_toggle = HIGH;

unsigned char bg_attr_table[64];
unsigned char fg_attr_table[64];

unsigned char interrupt_occurred = 0;
unsigned char interrupt_handled = 0;

extern unsigned char mirroring;

SDL_Renderer *renderer;
SDL_Window *window;
SDL_Texture *texture;

int current_line = 0;
int current_row = 0;
int prev_row = 0;
int ppu_cycles = 0;

unsigned char PALETTE[0x40][3] = {
   {0x80, 0x80, 0x80}, {0x00, 0x3D, 0xA6}, {0x00, 0x12, 0xB0}, {0x44, 0x00, 0x96}, {0xA1, 0x00, 0x5E}, {0xC7, 0x00, 0x28}, {0xBA, 0x06, 0x00}, {0x8C, 0x17, 0x00}, {0x5C, 0x2F, 0x00}, {0x10, 0x45, 0x00},
   {0x05, 0x4A, 0x00}, {0x00, 0x47, 0x2E}, {0x00, 0x41, 0x66}, {0x00, 0x00, 0x00}, {0x05, 0x05, 0x05}, {0x05, 0x05, 0x05}, {0xC7, 0xC7, 0xC7}, {0x00, 0x77, 0xFF}, {0x21, 0x55, 0xFF}, {0x82, 0x37, 0xFA},
   {0xEB, 0x2F, 0xB5}, {0xFF, 0x29, 0x50}, {0xFF, 0x22, 0x00}, {0xD6, 0x32, 0x00}, {0xC4, 0x62, 0x00}, {0x35, 0x80, 0x00}, {0x05, 0x8F, 0x00}, {0x00, 0x8A, 0x55}, {0x00, 0x99, 0xCC}, {0x21, 0x21, 0x21},
   {0x09, 0x09, 0x09}, {0x09, 0x09, 0x09}, {0xFF, 0xFF, 0xFF}, {0x0F, 0xD7, 0xFF}, {0x69, 0xA2, 0xFF}, {0xD4, 0x80, 0xFF}, {0xFF, 0x45, 0xF3}, {0xFF, 0x61, 0x8B}, {0xFF, 0x88, 0x33}, {0xFF, 0x9C, 0x12},
   {0xFA, 0xBC, 0x20}, {0x9F, 0xE3, 0x0E}, {0x2B, 0xF0, 0x35}, {0x0C, 0xF0, 0xA4}, {0x05, 0xFB, 0xFF}, {0x5E, 0x5E, 0x5E}, {0x0D, 0x0D, 0x0D}, {0x0D, 0x0D, 0x0D}, {0xFF, 0xFF, 0xFF}, {0xA6, 0xFC, 0xFF},
   {0xB3, 0xEC, 0xFF}, {0xDA, 0xAB, 0xEB}, {0xFF, 0xA8, 0xF9}, {0xFF, 0xAB, 0xB3}, {0xFF, 0xD2, 0xB0}, {0xFF, 0xEF, 0xA6}, {0xFF, 0xF7, 0x9C}, {0xD7, 0xE8, 0x95}, {0xA6, 0xED, 0xAF}, {0xA2, 0xF2, 0xDA},
   {0x99, 0xFF, 0xFC}, {0xDD, 0xDD, 0xDD}, {0x11, 0x11, 0x11}, {0x11, 0x11, 0x11}
};

// unsigned char PALETTE[0x40][3] = {
// 	{0x7C, 0x7C, 0x7C}, {0x00, 0x00, 0xFC}, {0x00, 0x00, 0xBC}, {0x44, 0x28, 0xBC}, {0x94, 0x00, 0x84}, {0xA8, 0x00, 0x20}, {0xA8, 0x10, 0x00}, {0x88, 0x14, 0x00},
// 	{0x50, 0x30, 0x00}, {0x00, 0x78, 0x00}, {0x00, 0x68, 0x00}, {0x00, 0x58, 0x00}, {0x00, 0x40, 0x58}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00},
// 	{0xBC, 0xBC, 0xBC}, {0x00, 0x78, 0xF8}, {0x00, 0x58, 0xF8}, {0x68, 0x44, 0xFC}, {0xD8, 0x00, 0xCC}, {0xE4, 0x00, 0x58}, {0xF8, 0x38, 0x00}, {0xE4, 0x5C, 0x10},
// 	{0xAC, 0x7C, 0x00}, {0x00, 0xB8, 0x00}, {0x00, 0xA8, 0x00}, {0x00, 0xA8, 0x44}, {0x00, 0x88, 0x88}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00},
// 	{0xF8, 0xF8, 0xF8}, {0x3C, 0xBC, 0xFC}, {0x68, 0x88, 0xFC}, {0x98, 0x78, 0xF8}, {0xF8, 0x78, 0xF8}, {0xF8, 0x58, 0x98}, {0xF8, 0x78, 0x58}, {0xFC, 0xA0, 0x44},
// 	{0xF8, 0xB8, 0x00}, {0xB8, 0xF8, 0x18}, {0x58, 0xD8, 0x54}, {0x58, 0xF8, 0x98}, {0x00, 0xE8, 0xD8}, {0x78, 0x78, 0x78}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00},
// 	{0xFC, 0xFC, 0xFC}, {0xA4, 0xE4, 0xFC}, {0xB8, 0xB8, 0xF8}, {0xD8, 0xB8, 0xF8}, {0xF8, 0xB8, 0xF8}, {0xF8, 0xA4, 0xC0}, {0xF0, 0xD0, 0xB0}, {0xFC, 0xE0, 0xA8},
// 	{0xF8, 0xD8, 0x78}, {0xD8, 0xF8, 0x78}, {0xB8, 0xF8, 0xB8}, {0xB8, 0xF8, 0xD8}, {0x00, 0xFC, 0xFC}, {0xF8, 0xD8, 0xF8}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}
// };

void write_ppuctrl(unsigned char value) {
	ppuctrl = value;
}

void write_ppumask(unsigned char value) {
    ppumask = value;
}

unsigned char read_ppustatus() {
	unsigned char current = ppustatus;
	clear_vblank();
	t = 0;
	t_toggle = HIGH;
    return current;
}

void write_oamaddr(unsigned char value) {
    oamaddr = value;
}

unsigned char read_oamdata() {
	if (check_bit(ppustatus, VBLANK_BIT) == 0) {
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

void mirror_palette() {
	if ((v >= 0x3F00) && (v <= 0x3FFF)) {
		switch (v) {
			case 0x3F10:
				VRAM[0x3F00] = VRAM[v];
				break;
			case 0x3F00:
				VRAM[0x3F10] = VRAM[v];
				break;
			case 0x3F14:
				VRAM[0x3F04] = VRAM[v];
				break;
			case 0x3F04:
				VRAM[0x3F14] = VRAM[v];
				break;
			case 0x3F18:
				VRAM[0x3F08] = VRAM[v];
				break;
			case 0x3F08:
				VRAM[0x3F18] = VRAM[v];
				break;
			case 0x3F1C:
				VRAM[0x3F0C] = VRAM[v];
				break;
			case 0x3F0C:
				VRAM[0x3F1C] = VRAM[v];
				break;
		}
	}
}

void mirror_v() {
	if (mirroring == 0) { // Horizontal
		if ((v >= 0x2400) && (v <= (0x2400+0x400))) {
			v -= 0x400;
		} else if ((v >= 0x2C00) && (v <= (0x2C00+0x400))) {
			v -= 0x400;
		}
	} else { // Vertical
		if ((v >= 0x2800) && (v <= (0x2800 + 0x400))) {
			v -= 0x800;
		} else if ((v >= 0x2C00) && (v <= 0x2C00 + 0x400)) {
			v -= 0x800;
		}
	}
}

void write_ppudata(unsigned char value) {
	mirror_v();

	if (v >= 0x3F00) {
		update_palette();
	}

    VRAM[v] = value;
	mirror_palette();

	if (check_bit(ppuctrl, 2) == 1) {
		v += 32;
	} else {
		v++;
	}
}

void write_ppuscroll(unsigned char value) {
	if (t_toggle == HIGH) {
		t_toggle = LOW;
		ppuscroll = value << 8;
	} else {
		t_toggle = HIGH;
		ppuscroll |= value;
	}
}

void write_v(unsigned char value) {
	if (t_toggle == HIGH) {
		t_toggle = LOW;
		t = value << 8;
		// v = t;
	} else {
		t_toggle = HIGH;
		t |= value;
		t &= 0x3FFF;
		// if (check_bit(ppustatus, VBLANK_BIT) == 0) {
			v = t;
		// }
	}
}

unsigned char read_ppudata() {
	mirror_v();

	unsigned char value = ppudata_buffer;
	ppudata_buffer = VRAM[v];
	if ((v & 0x3F00) == 0x3F00) {
		ppudata_buffer = VRAM[v-0x1000];
		value = VRAM[v];
	}
	v++;
    return value;
}

void clear_screen() {
	SDL_SetRenderDrawColor(renderer, 255, 0, 0, 0);
    SDL_RenderClear(renderer);
}

void set_vblank() {
	ppustatus = set_bit(ppustatus, VBLANK_BIT);
}

void clear_vblank() {
	ppustatus = clear_bit(ppustatus, VBLANK_BIT);
	t_toggle = HIGH;
	t_toggle = HIGH;
}

int can_generate_nmi() {
	if (ppuctrl & 0x80) {
		return 1;
	}
	return 0;
}

unsigned char init_sdl()
{
	SDL_Init(SDL_INIT_EVERYTHING);
	window = SDL_CreateWindow("SDL2", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);
}

void free_ppu()
{
	// SDL_FreeSurface( gGameSurface );
	// gGameSurface = NULL;
	// SDL_DestroyWindow( gWindow );
	// gWindow = NULL;
	SDL_Quit();
}

unsigned char framebuffer[WIDTH * HEIGHT * sizeof(uint32_t)];

int bank_address[] = {
	0x0,
	0x1000
};

unsigned char palette[4][4];

void update_palette() {
	palette[0][0] = VRAM[0x3F00];
	palette[0][1] = VRAM[0x3F01];
	palette[0][2] = VRAM[0x3F02];
	palette[0][3] = VRAM[0x3F03];

	palette[1][0] = VRAM[0x3F00];
	palette[1][1] = VRAM[0x3F05];
	palette[1][2] = VRAM[0x3F06];
	palette[1][3] = VRAM[0x3F07];

	palette[2][0] = VRAM[0x3F00];
	palette[2][1] = VRAM[0x3F09];
	palette[2][2] = VRAM[0x3F0A];
	palette[2][3] = VRAM[0x3F0B];

	palette[3][0] = VRAM[0x3F00];
	palette[3][1] = VRAM[0x3F0D];
	palette[3][2] = VRAM[0x3F0E];
	palette[3][3] = VRAM[0x3F0F];

	palette[4][0] = VRAM[0x3F00];
	palette[4][1] = VRAM[0x3F11];
	palette[4][2] = VRAM[0x3F12];
	palette[4][3] = VRAM[0x3F13];

	palette[5][0] = VRAM[0x3F00];
	palette[5][1] = VRAM[0x3F15];
	palette[5][2] = VRAM[0x3F16];
	palette[5][3] = VRAM[0x3F17];

	palette[6][0] = VRAM[0x3F00];
	palette[6][1] = VRAM[0x3F19];
	palette[6][2] = VRAM[0x3F1A];
	palette[6][3] = VRAM[0x3F1B];

	palette[7][0] = VRAM[0x3F00];
	palette[7][1] = VRAM[0x3F1D];
	palette[7][2] = VRAM[0x3F1E];
	palette[7][3] = VRAM[0x3F1F];
}

void set_pixel(int x, int y, int color_index, int palette_index) {
	unsigned char R = PALETTE[palette[palette_index][color_index]][0];
	unsigned char G = PALETTE[palette[palette_index][color_index]][1];
	unsigned char B = PALETTE[palette[palette_index][color_index]][2];

	unsigned int offset = (WIDTH * y * sizeof(uint32_t)) + (x * sizeof(uint32_t));
	framebuffer[offset] = B;
	framebuffer[offset + 1] = G;
	framebuffer[offset + 2] = R;
	if (palette_index == 0) {
		framebuffer[offset + 3] = SDL_ALPHA_TRANSPARENT;
	} else {
		framebuffer[offset + 3] = SDL_ALPHA_OPAQUE;
	}
}

void dump_vram()
{
    FILE *dump_file = fopen("dumpv.txt", "w");
    for (int i = 0; i < sizeof(VRAM); i++)
    {
        if ((i % 8) == 0)
        {
            fprintf(dump_file, "\n%04x ", i);
        }
        fprintf(dump_file, "%02x ", VRAM[i]);
    }
}

void show_tile(int bank, int tile_n, int row, int col) {
	int tile[16];
	int start_x = col * 8;
	int start_y = row * 8;

	int block_x = col / 4;
	int block_y = row / 4;

	uint16_t attr_addr = (block_y * 8) + block_x;
	unsigned char attr_byte = bg_attr_table[attr_addr];

	unsigned char block_id = ((col % 4) / 2) + (((row % 4) / 2) * 2);
	unsigned char which_palette = (attr_byte >> (block_id * 2)) & 0x03;

	for (int y = 0; y <= 7; y++) {
		unsigned char upper = VRAM[bank + tile_n * 0x10 + y + 8];
		unsigned char lower = VRAM[bank + tile_n * 0x10 + y];

		for (int x = 7; x >= 0; x--) {
			int value = ((1 & upper) << 1) | (1 & lower);
			upper >>= 1;
			lower >>= 1;

			if ((tile_n == 0x24) && (value != 0)) {
				int d = 0;
			}

			set_pixel(x + start_x, y + start_y, value, which_palette);
		}
	}
}

void draw_background() {
	if (check_bit(ppumask, BG_ENABLE) == 0) {
		return;
	}

	unsigned char nametable_id = (check_bit(ppuctrl, 1) << 1) | check_bit(ppuctrl, 0);
	uint16_t nametable_address = 0x2000 + (nametable_id * 0x400);

	int bg_bank = check_bit(ppuctrl, BG_TILE_SELECT);
	if (bg_bank == 1) {
		memcpy(bg_attr_table, &VRAM[0x23C0], 64);
	} else {
		memcpy(bg_attr_table, &VRAM[0x27C0], 64);
	}
 	// for (int row = 0; row < 30; row++) {
		for (int col = 0; col < 32; col++) {
			uint16_t tile_n = VRAM[nametable_address + ( current_row * 32) + col];
			show_tile(bank_address[bg_bank], tile_n, current_row, col);
		}
	// }


	SDL_UpdateTexture(texture, NULL, framebuffer, WIDTH * 4);
	SDL_RenderCopy(renderer, texture, NULL, NULL);
}

struct OAM {
	unsigned char y;
	unsigned char tile_n;
	unsigned char attr_byte;
	unsigned char x;
};

struct OAM parsed_oam[64];

void show_sprite(int bank, int tile_n, int start_x, int start_y, int attr_byte) {
	unsigned char which_palette = (attr_byte & 0x03) + 4;
	unsigned char flip_h = check_bit(attr_byte, FLIP_HORIZONTAL);
	unsigned char flip_v = check_bit(attr_byte, FLIP_VERTICAL);

	if (tile_n != 0) {
		int d = 0;
	}

	for (int y = 0; y <= 7; y++) {
		unsigned char upper = VRAM[bank + tile_n * 0x10 + y + 8];
		unsigned char lower = VRAM[bank + tile_n * 0x10 + y];

		if (flip_h) {
			for (int x = 0; x < 8; x++) {
				int value = ((1 & upper) << 1) | (1 & lower);
				upper >>= 1;
				lower >>= 1;

				set_pixel(start_x + x, start_y + y, value, which_palette);
			}
		} else {
			for (int x = 7; x >= 0; x--) {
				int value = ((1 & upper) << 1) | (1 & lower);
				upper >>= 1;
				lower >>= 1;

				set_pixel(start_x + x, start_y + y, value, which_palette);
			}
		}
	}
}

void draw_sprites() {
	if (check_bit(ppumask, SPRITES_ENABLE) == 0) {
		return;
	}

	memcpy(parsed_oam, oamdata, OAMSIZE);

	// Weird 2C02 behavior (bug?)
	// if (oamaddr < 8) {
	// 	memcpy(oamdata, VRAM[OAMADDR & 0xF8], 8);
	// }

	int fg_bank = check_bit(ppuctrl, FG_TILE_SELECT);
	if (fg_bank == 1) {
		memcpy(fg_attr_table, &VRAM[0x2BFF-64], 64);
	} else {
		memcpy(fg_attr_table, &VRAM[0x2FFF-64], 64);
	}

	for (int i = 0; i < 64; i++) {
		show_sprite(bank_address[fg_bank], parsed_oam[i].tile_n, parsed_oam[i].x, parsed_oam[i].y, parsed_oam[i].attr_byte);
	}
}

void init_ppu() {
	init_sdl();
    memcpy(VRAM, rom.chr_rom, 0x1FFF);

	ppuctrl= 0;
	ppumask = 0;
	ppustatus = 0x0;
	oamaddr = 0;
	ppuscroll = 0;
	v = 0;
	ppudata = 0;
}

int total_cpu_cycles = 0;

void ppu_clock(int cpu_cycles) {
	total_cpu_cycles += cpu_cycles;
	int end_cycles = cpu_cycles * 3;

	for (int i = 0; i < end_cycles; i++) {
		if ((total_cpu_cycles % 2270) == 0) {
			clear_vblank();
		}
		total_cycles++;
		ppu_cycles++;

		if (ppu_cycles > 340) {
			ppu_cycles = 1;
			current_line++;
		}

		if ((current_line < 240) && (ppu_cycles == 1)) {
				current_row = (int)(current_line / 8);
				if (current_row != prev_row) {
					prev_row = current_row;
					draw_background();
				}
				draw_sprites();
		} else if ((current_line == 241) && (ppu_cycles == 1)) {
			set_vblank();
			if (write_enabled == 0) {
				warmup_count++;
			}

			if (warmup_count >= 2) {
				write_enabled = 1;
			}

			if (interrupt_occurred == 0) {
				SDL_RenderPresent(renderer);
				if (can_generate_nmi()) {
					interrupt_handled = 0;
					interrupt_occurred = NMI_INT;
				}
				// v = t;
			}
		} else if ((current_line == 261) && (ppu_cycles == 1)) {
			clear_vblank();
			interrupt_occurred = 0;
			current_line = 0;
		}
	}
}

