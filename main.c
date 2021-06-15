#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "addressing.h"
#include "rom.h"

extern unsigned char RAM[0xFFFF];
extern uint8_t A;
extern uint8_t X;
extern uint8_t Y;
extern uint8_t PS;
extern uint16_t SP;

extern struct ROM rom;
extern struct addressing_data addressing[(0xFF) + 1];
extern uint16_t PC;

int is_jump_or_branch(char *fn_name) {
    if (
        // (strncmp(fn_name, "BCC", 3) != 0) &&
        // (strncmp(fn_name, "BCS", 3) != 0) &&
        // (strncmp(fn_name, "BEQ", 3) != 0) &&
        // (strncmp(fn_name, "BMI", 3) != 0) &&
        // (strncmp(fn_name, "BNE", 3) != 0) &&
        // (strncmp(fn_name, "BPL", 3) != 0) &&
        (strncmp(fn_name, "BRK", 3) != 0) &&
        // (strncmp(fn_name, "BVC", 3) != 0) &&
        // (strncmp(fn_name, "BVS", 3) != 0) &&
        (strncmp(fn_name, "JMP", 3) != 0) &&
        (strncmp(fn_name, "JSR", 3) != 0)
        // (strncmp(fn_name, "RTS", 3) != 0)
        ) {
        return 0;
    }
    return 1;
}

void main(int argc, char **argv) {
    init_ram();
    // load_rom("/home/gabriele/Downloads/cpu_test/cpu_dummy_writes/cpu_dummy_writes_oam.nes");
    // load_rom("/home/gabriele/Downloads/cpu_test/cpu_dummy_reads.nes");
    // load_rom("/home/gabriele/Downloads/bf.nes");
    load_rom("/home/gabriele/Downloads/cpu_test/nestest.nes");
    // printf("%d\n", rom.header.prg_blocks);
    // printf("%s %d %d %d %d %d %d %d\n", rom.header.nes, rom.header.prg_blocks, rom.header.chr_blocks, rom.header.flags_6, rom.header.flags_7, rom.header.flags_8, rom.header.flags_9, rom.header.flags_10, rom.header.padding[5]);

    unsigned char opcode = 0;
    unsigned char value = 0;
    unsigned char first;
    unsigned char second;
    PC = 0xC000;
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

        void (*fun_ptr)(unsigned char, unsigned char, unsigned char) = addressing[opcode].opcode_fun;
        (*fun_ptr)(first, second, addressing[opcode].addr_mode);

        unsigned char *fn_name = addressing[opcode].name;

        if (is_jump_or_branch(fn_name) == 0) {
            PC++;

            int am = addressing[opcode].addr_mode;
            if ((ZEROPAGEX == am) || (ZEROPAGEY == am) || (ABSOLUTEX == am) || (ABSOLUTEY == am) || (INDIRECTX == am) || (INDIRECTY == am)) {
                PC += 2;
            } else if ((ZEROPAGE == am) || (ABSOLUTE == am) || (RELATIVE == am) || (INDIRECT == am) || (IMMEDIATE == am)) {
                PC++;
            }
        }
    }
    
    printf("0x02: %02x\n", RAM[0x02]);
    printf("0x03: %03x\n", RAM[0x03]);
}