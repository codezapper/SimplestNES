#include <stdio.h>
#include <string.h>

#include "rom.h"
#include "utils.h"

extern char RAM[0xFFFF];

struct ROM rom;

void load_rom(char *filename) {
    FILE *rom_file = fopen(filename, "rb");

    fread(&rom.header, 16, 1, rom_file);

    int mirroring = check_bit(rom.header.flags_6, 0);
    int battery_backed = check_bit(rom.header.flags_6, 1);
    int trainer = check_bit(rom.header.flags_6, 2);
    int ignore_mirroring = check_bit(rom.header.flags_6, 3);
    int lower_mapper = (rom.header.flags_6 >> 4) & 0xF;

    int vs_unisys = check_bit(rom.header.flags_7, 0);
    int playchoice10 = check_bit(rom.header.flags_7, 1);
    int flag_format01 = check_bit(rom.header.flags_7, 2);
    int flag_format02 = check_bit(rom.header.flags_7, 3);
    int higher_mapper = rom.header.flags_7 & 0xF0;

    int mapper = higher_mapper | lower_mapper;

    printf("mirroring: %d\n", mirroring);
    printf("battery_backed: %d\n", battery_backed);
    printf("trainer: %d\n", trainer);
    printf("ignore_mirroring: %d\n", ignore_mirroring);
    printf("lower_mapper: %d\n", lower_mapper);
    printf("vs_unisys: %d\n", vs_unisys);
    printf("playchoice10: %d\n", playchoice10);
    printf("flag_format01: %d\n", flag_format01);
    printf("flag_format02: %d\n", flag_format02);
    printf("higher_mapper: %d\n", higher_mapper);
    printf("mapper: %d\n", mapper);

    // if flag_format01 == 0) && (flag_format02 == 1) {
    // flags 8-15 are in NES format
    //}

    if (1 == trainer) {
        // TODO: implement trainer
        printf("TRAINER\n");
    } else {
        printf("NO TRAINER\n");
        fread(&RAM[0x800], 16384 * rom.header.prg_size, 1, rom_file);
    }

    fclose(rom_file);
}