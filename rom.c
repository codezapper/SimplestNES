#include <stdio.h>
#include "rom.h"

struct ROM rom;

char *load_rom(char *filename) {
    FILE *rom_file = fopen(filename, "rb");

    fread(&rom.header, 16, 1, rom_file);
}