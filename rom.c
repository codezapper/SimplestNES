#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "rom.h"
#include "utils.h"

extern unsigned char RAM[0xFFFF];

struct ROM rom;
struct ROM20 rom20;

unsigned char is_nes_20 = 0;
unsigned char mirroring = 0;

void load_rom(char *filename) {
    FILE *rom_file = fopen(filename, "rb");
    if (rom_file == NULL) {
        printf("Unable to open %s", filename);
        exit(1);
    }

    fread(&rom.header, 16, 1, rom_file);

    // unsigned char mapper1 = rom.header.flags_6 >> 4;
    // unsigned char mapper2 = rom.header.flags_7 >> 4;
    // unsigned char mapper = (mapper1 | mapper2) << 4;

    if ((rom.header[7] & 0x0C) == 0x08) {
        is_nes_20 = 1;
    }

    // int battery_backed = check_bit(rom.header.flags_6, 1);
    // int trainer = check_bit(rom.header.flags_6, 2);
    // int ignore_mirroring = check_bit(rom.header.flags_6, 3);
    // int lower_mapper = (rom.header.flags_6 >> 4) & 0xF;

    // int vs_unisys = check_bit(rom.header.flags_7, 0);
    // int playchoice10 = check_bit(rom.header.flags_7, 1);
    // int flag_format01 = check_bit(rom.header.flags_7, 2);
    // int flag_format02 = check_bit(rom.header.flags_7, 3);
    // int higher_mapper = rom.header.flags_7 & 0xF0;

    // int mapper = higher_mapper | lower_mapper;

    // printf("mirroring: %d\n", mirroring);
    // printf("battery_backed: %d\n", battery_backed);
    // printf("trainer: %d\n", trainer);
    // printf("ignore_mirroring: %d\n", ignore_mirroring);
    // printf("lower_mapper: %d\n", lower_mapper);
    // printf("vs_unisys: %d\n", vs_unisys);
    // printf("playchoice10: %d\n", playchoice10);
    // printf("flag_format01: %d\n", flag_format01);
    // printf("flag_format02: %d\n", flag_format02);
    // printf("higher_mapper: %d\n", higher_mapper);
    // printf("mapper: %d\n", mapper);

    // if flag_format01 == 0) && (flag_format02 == 1) {
    // flags 8-15 are in NES format
    //}

    unsigned char prg_rom[32 *1024L];

    // if (1 == trainer) {
        // TODO: implement trainer
        // printf("TRAINER\n");
    // } else {
        // printf("NO TRAINER\n");
    if (is_nes_20) {
        struct HEADER20 header;
        memcpy(&header, rom.header, 16);
        mirroring = check_bit(header.flags_6, 0);

        int prg_size = ((header.rom_size_msb & 0b00001111) << 8) | header.prg_rom_lsb;
        int chr_size = ((header.rom_size_msb & 0b11110000) << 8) | header.chr_rom_lsb;
        if (prg_size > 1) {
            fread(&RAM[0x8000], 1, 16*1024, rom_file);
            fread(&RAM[0xC000], 1, 16*1024, rom_file);
        } else {
            fread(&rom.prg_rom, 1, 16*1024*prg_size, rom_file);
            memcpy(&RAM[0x8000], &rom.prg_rom, 16*1024);
            memcpy(&RAM[0xC000], &rom.prg_rom, 16*1024);
        }
        fread(&rom.chr_rom, 1, 8*1024*chr_size, rom_file);
    } else {
        struct HEADER header;
        memcpy(&header, rom.header, 16);
        mirroring = check_bit(header.flags_6, 0);
        if (header.prg_blocks > 1) {
            fread(&RAM[0x8000], 1, 16*1024, rom_file);
            fread(&RAM[0xC000], 1, 16*1024, rom_file);
        } else {
            fread(&rom.prg_rom, 1, 16*1024*header.prg_blocks, rom_file);
            memcpy(&RAM[0x8000], &rom.prg_rom, 16*1024);
            memcpy(&RAM[0xC000], &rom.prg_rom, 16*1024);
        }
        fread(&rom.chr_rom, 1, 8*1024*header.chr_blocks, rom_file);
    }
    // }

    fclose(rom_file);

    // FILE *rom_file_chr = fopen(filename, "rb");
    // for (int i = 0; i < (0x10 + (0x4000 * rom.header.prg_blocks)); i++) {
    //     // rom.chr_rom[i] = fgetc(rom_file_chr);
    //     fgetc(rom_file_chr);
    // }
    // for (int j = 0; j < (8192 * rom.header.chr_blocks); j++) {
    //     rom.chr_rom[j] = fgetc(rom_file_chr);
    // }
    // fclose(rom_file_chr);

}