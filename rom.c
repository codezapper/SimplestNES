#include <stdio.h>
#include <string.h>

#include "rom.h"
#include "utils.h"

extern unsigned char RAM[0xFFFF];

struct ROM rom;

void load_rom(char *filename) {
    FILE *rom_file = fopen(filename, "rb");

 
     if ( fseek( rom_file, 16, SEEK_SET ) ) {
        int a = 0;
     }

    // fread(&rom.header, 16, 1, rom_file);

    // int mirroring = check_bit(rom.header.flags_6, 0);
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

    unsigned char rom[32 *1024L];

    // if (1 == trainer) {
        // TODO: implement trainer
        // printf("TRAINER\n");
    // } else {
        printf("NO TRAINER\n");
        int i = 0;
        fread(rom, 1, sizeof(rom), rom_file);
        // while (!feof(rom_file)) {
        //     // fread(&RAM[0x200+i], 1, 1, rom_file);
        memcpy(&RAM[0xC000], rom, 16*1024);
        memcpy(&RAM[0x8000], rom, 16*1024);
            // RAM[0x8000+i] = (unsigned char)(getc(rom_file));
        //     i++;
        //     if (0x6210 == i) {
        //         int a = 0;
        //     }
        // }
        // fread(&RAM[0x200], 1, 16384 * rom.header.prg_size, rom_file);
    // }

    fclose(rom_file);
}