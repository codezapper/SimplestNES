#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "addressing.h"
#include "cpu.h"
#include "ppu.h"
#include "rom.h"
#include "utils.h"

extern unsigned char RAM[0xFFFF];
extern uint8_t A;
extern uint8_t X;
extern uint8_t Y;
extern uint8_t PS;
extern uint16_t SP;

extern struct ROM rom;
extern uint16_t PC;
extern unsigned char interrupt_occurred;

void log_to_screen(unsigned char opcode, unsigned char first, unsigned char second, char *fn_name) {
    int am = addressing[opcode].addr_mode;

    char log_line[1024];
    if (addressing[opcode].bytes == 3) {
        sprintf(log_line, "%04x %02x %02x %02x %s\tA:%02x X:%02x Y:%02x P:%02x SP:%02x CYCLE:%d\n", PC, opcode, first, second, fn_name, A, X, Y, PS, SP, 0); 
    } else if (addressing[opcode].bytes == 2) {
        sprintf(log_line, "%04x %02x %02x    %s\tA:%02x X:%02x Y:%02x P:%02x SP:%02x CYCLE:%d\n", PC, opcode, first, fn_name, A, X, Y, PS, SP, 0); 
    } else {
        sprintf(log_line, "%04x %02x       %s\tA:%02x X:%02x Y:%02x P:%02x SP:%02x CYCLE:%d\n", PC, opcode, fn_name, A, X, Y, PS, SP, 0); 
    }

    for (int i = 0; log_line[i]!='\0'; i++) {
        if(log_line[i] >= 'a' && log_line[i] <= 'z') {
            log_line[i] = log_line[i] -32;
        }
    }

    printf(log_line);
}

int is_jump(unsigned char *fn_name) {
    if (
        (strncmp(fn_name, "BRK", 3) != 0) &&
        (strncmp(fn_name, "JMP", 3) != 0) &&
        (strncmp(fn_name, "JSR", 3) != 0) &&
        (strncmp(fn_name, "RTI", 3) != 0)
        ) {
        return 0;
    }
    return 1;
}

int cpu_interrupt_count = 0;

int must_handle_interrupt() {
    if ((interrupt_occurred == NMI_INT) || ((interrupt_occurred == IRQ_INT) && (check_bit(PS, ID) == 0))){
        if (cpu_interrupt_count >= 6) {
            cpu_interrupt_count = 0;
        } else {
            cpu_interrupt_count++;
        }
    }
    return 0;
}

void main(int argc, char **argv) {
    init_ram();
    init_ppu();
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

    PC = 0xC000; // Test mode, use log compare
    // JMP(0xFC, 0xFF, INDIRECT);
    // PC = (RAM[0xFFFD] << 8) | RAM[0xFFFC];
    while (PC > 0) {
        if (must_handle_interrupt(interrupt_occurred)) {
            if (interrupt_occurred == NMI_INT) {
                NMI();
            } else {
                IRQ();
            }
        }
        opcode = RAM[PC];
        if (addressing[opcode].cycles == 0) {
            PC++;
            continue;
        }
        first = (RAM[PC + 1]);
        second = (RAM[PC + 2]);

        unsigned char fn_name[4];
        memset(fn_name, 0, 4);
        strncpy(fn_name, addressing[opcode].name, 3);

        log_to_screen(opcode, first, second, fn_name);

        void (*fun_ptr)(unsigned char, unsigned char, unsigned char) = addressing[opcode].opcode_fun;
        (*fun_ptr)(first, second, addressing[opcode].addr_mode);

        if ((is_jump(fn_name) == 0) && (PC > 0)) {
            PC += addressing[opcode].bytes;
        }

        ppu_clock(addressing[opcode].cycles * 3);
    }
}
