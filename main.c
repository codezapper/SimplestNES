#include <stdio.h>
#include <stdint.h>
#include "addressing.h"
#include "rom.h"

extern char RAM[0xFFFF];
extern int8_t A;

extern struct ROM rom;

void main(int argc, char **argv) {
    init_ram();
    RAM[2] = 17;

    // ((addressing[0xA5])(get_pointer_to_ram(0xA5, 2, 20));
    // void (*fun_ptr)(char *) = addressing[0xA5].opcode_fun;
    // (*fun_ptr)(get_pointer_to_ram(0xA5, 2, 20));
    // printf("%d\n", A);
    // void (*fun_ptr)(char *) = addressing[0xA9].opcode_fun;
    // (*fun_ptr)(get_pointer_to_ram(0xA9, 42, 20));
    // printf("%d\n", A);

    load_rom("/home/gabriele/Downloads/cpu_test/cpu_dummy_reads.nes");
    printf("%s %d %d %d %d %d %d %d\n", rom.header.nes, rom.header.prg_size, rom.header.chr_size, rom.header.flags_6, rom.header.flags_7, rom.header.flags_8, rom.header.flags_9, rom.header.flags_10, rom.header.padding[5]);
}