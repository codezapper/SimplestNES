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

extern struct ROM rom;
extern unsigned char RAM[0xFFFF];
extern int cycles;

unsigned char VRAM[0xFFFF];
unsigned char palette_table[32];

unsigned char ppuctrl = 0x80;
unsigned char ppumask = 0;
unsigned char ppustatus = 0x80;
unsigned char oamaddr = 0;
unsigned char oamdata[OAMSIZE];
unsigned char ppuscroll;
unsigned char ppuaddr;
unsigned char ppudata;
unsigned char oamdma;

unsigned char t_address = 0;
unsigned char t_scroll = 0;
uint16_t ppu_address = 0;

unsigned char _data[_DATA_SIZE];

unsigned char interrupt_occurred = 0;

struct Tile {
	unsigned char sprite[8][8];
	char scroll_x;
	char scroll_y;
	int palette_id;
};

int total_cycles = 0;

struct Tile background[1024];

SDL_Renderer *renderer, *renderer_nt, *renderer_oam;
SDL_Window *window, *window_nt, *window_oam;
SDL_Texture *texture, *texture_nt, *texture_oam;


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
	clear_vblank();
    return ppustatus;
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
    ppuscroll = value;
}

void write_ppustatus(unsigned char value) {
    ppustatus = value;
}

void write_ppuaddress(unsigned char value) {
	ppu_address = t_address << 8;
	t_address = value;
	ppu_address |= t_address;
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
	t_address = 0;
}

int can_generate_nmi() {
	if (ppuctrl & 0x80) {
		return 1;
	}
	return 0;
}

int current_line = 0;
int ppu_cycles = 0;

void render() {
	for (int i = 0; i <= sizeof(background); i++) {
		for (int y = 0; y < 8; y++) {
			for (int x = 0; x < 8; x++) {
				if (background[i].sprite[x][y] != 0) {
					int color = background[i].sprite[x][y];
					SDL_SetRenderDrawColor(renderer, PALETTE[color][0], PALETTE[color][1], PALETTE[color][2], 255);
					SDL_RenderDrawPoint(renderer, x, y);
				}
			}
		}
	}
	SDL_RenderPresent(renderer);
}

void ppu_clock(int cpu_cycles) {
	total_cycles++;
	ppu_cycles++;
	if (ppu_cycles > 340) {
		ppu_cycles -= 341;
		current_line++;
	}

	if (0 <= current_line && current_line <= 239) { //  drawing
		current_line++;
	} else if (current_line == 241 && ppu_cycles == 1) {    //  VBlank
		set_vblank();
		interrupt_occurred = NMI_INT;
		// drawNameTables();
		// handleWindowEvents();
	} else if (current_line == 261 && ppu_cycles == 1) {    //  VBlank off / pre-render line
		clear_vblank();
		interrupt_occurred = 0;
		current_line = 0;
		ppu_cycles = 0;
	}
}

void old_ppu_clock(int cpu_cycles) {
	int cycles = ppu_cycles + cpu_cycles;
	ppu_cycles = cycles;
    if (cycles < CYCLES_PER_LINE) {
		return;
	}

	if (current_line == 0) {
		clear_screen();
		// build_sprites();
	}

	if (cycles >= CYCLES_PER_LINE) {
		ppu_cycles = cycles - CYCLES_PER_LINE;
		current_line++;

	// 	if this.hasSpriteHit() {
	// 		this.setSpriteHit()
	// 	}
		if ((current_line <= 240) && (current_line % 8 == 0)) {
			build_background();
			render();
		}

		if (current_line == 241) {
			set_vblank();
			if (can_generate_nmi() > 0) {
				interrupt_occurred = NMI_INT;
			}
		}

		if (current_line == 262) {
			clear_vblank();
	// 		this.clearSpriteHit()
			current_line = 0;
			interrupt_occurred = 0;

	// 		background := Background{}
	// 		if this.isBackgroundEnable() {
	// 			background = this.Background
	// 		}
	// 		sprites := []SpriteWithAttribute{}
	// 		if this.isSpriteEnable() {
	// 			sprites = this.Sprites
	// 		}
	// 		this.RenderingData = RenderingData{
	// 			Palette:    this.Palette.Read(),
	// 			Background: background,
	// 			Sprites:    sprites,
	// 		}
	// 		return true
		}
	}

	return;
}

unsigned char init_sdl()
{
	SDL_Init(SDL_INIT_VIDEO);
	SDL_CreateWindowAndRenderer(256, 240, 0, &window, &renderer);
	SDL_SetWindowSize(window, 512, 480);
	SDL_SetWindowResizable(window, SDL_TRUE);
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

void set_pixel(unsigned char x, unsigned char y, int value) { //unsigned char r, unsigned char g, unsigned char b) {
	unsigned char R;
	unsigned char G;
	unsigned char B;

	switch (value) {
		case 0: 
			R = PALETTE[0x01][0];
			G = PALETTE[0x01][1];
			B = PALETTE[0x01][2];
			break;
		case 1: 
			R = PALETTE[0x23][0];
			G = PALETTE[0x23][1];
			B = PALETTE[0x23][2];
			break;
		case 2: 
			R = PALETTE[0x27][0];
			G = PALETTE[0x27][1];
			B = PALETTE[0x27][2];
			break;
		case 3: 
			R = PALETTE[0x30][0];
			G = PALETTE[0x30][1];
			B = PALETTE[0x30][2];
			break;
	}

    SDL_SetRenderDrawColor(renderer, R, G, B, 255);
	SDL_RenderDrawPoint(renderer, x, y);

	// TODO: Render using a texture and updating it

	// int start_address = y * 3 * WIDTH + x * 3;
	// if ((start_address + 2) < _DATA_SIZE) {
	// 	_data[start_address] = R;
	// 	_data[start_address + 1] = G;
	// 	_data[start_address + 2] = B;
	// }
}

void show_tile(int bank, int tile_n, int start_x, int start_y) {
	for (int y = 0; y <= 7; y++) {
		unsigned char upper = VRAM[bank + tile_n * 16 + y];
		unsigned char lower = VRAM[bank + tile_n * 16 + y + 8];

		for (int x = 7; x >= 0; x--) {
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
    memcpy(VRAM, rom.chr_rom, 0x1FFF);
	memset(_data, 0, _DATA_SIZE);
	memset(oamdata, 0, OAMSIZE);

	SDL_Texture *texture_chr = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, 128, 256);
	unsigned char framebuffer_chr[256 * 128 * 3];

	int COLORS[] = {
		0x00,
		0x55,
		0xaa,
		0xff
	};

	for (int r = 0; r < 256; r++) {
		for (int col = 0; col < 128; col++) {
			uint16_t adr = (r / 8 * 0x100) + (r % 8) + (col / 8) * 0x10;
			uint8_t pixel = ((rom.chr_rom[adr] >> (7-(col % 8))) & 1) + ((rom.chr_rom[adr + 8] >> (7-(col % 8))) & 1) * 2;
			framebuffer_chr[(r * 128 * 3) + (col * 3)] = COLORS[pixel];
			framebuffer_chr[(r * 128 * 3) + (col * 3) + 1] = COLORS[pixel];
			framebuffer_chr[(r * 128 * 3) + (col * 3) + 2] = COLORS[pixel];
		}
	}

	SDL_UpdateTexture(texture_chr, NULL, framebuffer_chr, 128 * sizeof(unsigned char) * 3);
	SDL_RenderCopy(renderer, texture_chr, NULL, NULL);
	SDL_RenderPresent(renderer);
	ppuctrl= 0x80;
}

unsigned char get_block_id(int x, int y) {
	return ((x % 4) / 2) + ((y % 4) / 2) * 2;
}

unsigned char get_sprite_id(int x, int y, int offset) {
	int tile_id = y * 32 + x;
	int sprite_address = mirror_down_sprite_address(tile_id + offset);
	return VRAM[sprite_address];
}

int mirror_down_sprite_address(int addr) {
	// TODO: Check horizontal mirroring
	if ((addr >= 0x0400 && addr < 0x0800) || (addr >= 0x0c00)) {
		return addr - 0x400;
	}
	return addr;
}

void build_background() {
	// int y = current_line / 8;
	// int bg_counter = 0;

	// for (int x = 0; x < 32 + 1; x++) {
	// 	// Add scroll here
	// 	struct Tile tile = build_tile(x, y, 0);
	// 	background[bg_counter] = tile;
	// 	bg_counter++;
	// }
}

// unsigned char get_attribute(int x, int y, int offset) {
// 	int address = (x / 4) + ((y / 4) * 8) + 0x03c0 + offset;
// 	return VRAM[address];
// }


// func (this *Ppu) readCharacterRAM(addr uint16) byte {
// 	return this.Bus.ReadByPpu(addr)
// }

// func (this *Ppu) writeCharacterRAM(addr uint16, data byte) {
// 	this.Bus.WriteByPpu(addr, data)
// }

// unsigned char **build_sprite(int sprite_id, int offset) {
// 	unsigned char **sprite = (unsigned char **)malloc(64);
// 	for (int i = 0; i < 16; i++) {
// 		for (int j = 0; j < 8; j++) {
// 			int address = sprite_id * 16 + i + offset;
// 			int ram = VRAM[address];
// 			if ((ram & (0x80 >> j)) != 0) {
// 				sprite[i % 8][j] += (0x01 << (i / 8));
// 			}
// 		}
// 	}
// 	return sprite;
// }

// int background_table_offset() {
// 	if ((ppuctrl & 0x10) > 0) {
// 		return 0x1000;
// 	}
// 	return 0x0000;
// }

// struct Tile build_tile(int x, int y, int offset) {
// 	// INFO see. http://hp.vector.co.jp/authors/VA042397/nes/ppu.html
// 	unsigned char block_id = get_block_id(x, y);
// 	unsigned char sprite_id = get_sprite_id(x, y, offset);
// 	unsigned char attr = get_attribute(x, y, offset);
// 	unsigned char palette_id = (attr >> (block_id * 2)) & 0x03;
// 	unsigned char sprite[8][8];

// 	struct Tile returned_tile;

// 	memcpy(&returned_tile.sprite, build_sprite(sprite_id, background_table_offset()), 64);
// 	returned_tile.scroll_x = 0;
// 	returned_tile.scroll_y = 0;
// 	returned_tile.palette_id = palette_id;

// 	return returned_tile;
// }


// func (this *Ppu) buildSprites() {
// 	var offset uint16 = 0x0000
// 	var sprite Sprite

// 	if (this.Registers[0] & 0x08) > 0 {
// 		offset = 0x1000
// 	}

// 	for i := 0 ; i < SPRITES_NUMBER ; i = (i+4)  {
// 		// INFO: Offset sprite Y position, because First and last 8line is not rendered.

// 		y := this.SpriteRam.Read(uint16(i))
// 		if y < 8 {
// 			return
// 		}

// 		spriteId := this.SpriteRam.Read(uint16(i+1))
// 		attr := this.SpriteRam.Read(uint16(i+2))
// 		x := this.SpriteRam.Read(uint16(i+3))
// 		sprite = this.buildSprite(spriteId, offset)
// 		this.Sprites[i/4] = NewStripeWithAttribute(sprite, x, y, attr, spriteId)
// 		//fmt.Println(sprite)
// 	}
// }

