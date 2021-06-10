#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "addressing.h"
#include "rom.h"

extern unsigned char RAM[0xFFFF];
extern int8_t A;
extern int8_t X;
extern int8_t Y;
extern int8_t PS;
extern int8_t SP;

extern struct ROM rom;
extern int16_t PC;

void main(int argc, char **argv) {
    init_ram();
    // RAM[2] = 17;
    // ((addressing[0xA5])(get_pointer_to_ram(0xA5, 2, 20));
    // void (*fun_ptr)(char *) = addressing[0xA5].opcode_fun;
    // (*fun_ptr)(get_pointer_to_ram(0xA5, 2, 20));
    // printf("%d\n", A);
    // void (*fun_ptr)(char *) = addressing[0xA9].opcode_fun;
    // (*fun_ptr)(get_pointer_to_ram(0xA9, 42, 20));
    // printf("%d\n", A);

    load_rom("/home/gabriele/Downloads/cpu_test/cpu_dummy_writes/cpu_dummy_writes_oam.nes");
    printf("%d\n", rom.header.prg_size);
    // printf("%s %d %d %d %d %d %d %d\n", rom.header.nes, rom.header.prg_size, rom.header.chr_size, rom.header.flags_6, rom.header.flags_7, rom.header.flags_8, rom.header.flags_9, rom.header.flags_10, rom.header.padding[5]);

    unsigned char opcode = 0;
    unsigned char value = 0;
    unsigned char first;
    unsigned char second;
    PC = 0xC00;
    while (PC > 0) {
        opcode = RAM[PC];
        if (addressing[opcode].cycles == 0) {
            PC++;
            continue;
        }
        first = (RAM[PC + 1]);
        second = (RAM[PC + 2]);

        printf("%x, %x %x %x %s  A:%x X:%x Y:%x P:%x SP:%x CYCLE:%d\n",
        PC, opcode, first, second, addressing[opcode].name, A, X, Y, PS, SP, 0); 

        void (*fun_ptr)(unsigned char *) = addressing[opcode].opcode_fun;
        (*fun_ptr)(get_pointer_to_ram(opcode, first, second));

        unsigned char *fn_name = addressing[opcode].name;

        if ((strcmp(fn_name, "BCC") != 0) &&
            (strcmp(fn_name, "BCS") != 0) &&
            (strcmp(fn_name, "BEQ") != 0) &&
            (strcmp(fn_name, "BMI") != 0) &&
            (strcmp(fn_name, "BNE") != 0) &&
            (strcmp(fn_name, "BPL") != 0) &&
            (strcmp(fn_name, "BRK") != 0) &&
            (strcmp(fn_name, "BVC") != 0) &&
            (strcmp(fn_name, "BVS") != 0) &&
            (strcmp(fn_name, "JMP") != 0) &&
            (strcmp(fn_name, "JSR") != 0) &&
            (strcmp(fn_name, "RTS") != 0)
            ) {

            PC++;

            int am = addressing[opcode].addr_mode;
            if ((ZEROPAGEX == am) || (ZEROPAGEY == am) || (ABSOLUTEX == am) || (ABSOLUTEY == am) || (INDIRECTX == am) || (INDIRECTY == am)) {
                PC += 2;
            } else if ((ZEROPAGE == am) || (ABSOLUTE == am) || (RELATIVE == am) || (INDIRECT == am)) {
                PC += 1;
            }
        }
    }
    
    printf("0x02: %02x\n", RAM[0x02]);
    printf("0x03: %03x\n", RAM[0x03]);
}