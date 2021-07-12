#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <SDL.h>

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
extern unsigned char interrupt_handled;

int cycles = 0;
extern int total_cycles;
extern int current_line;

void log_to_screen(unsigned char opcode, unsigned char first, unsigned char second, char *fn_name)
{
    int am = addressing[opcode].addr_mode;

    char log_line[1024];
    memset(log_line, 0, sizeof(log_line));

    char status_string[9];
    memset(status_string, 0, sizeof(status_string));
    status_string[0] = check_bit(PS, 7) ? 'N' : 'n';
    status_string[1] = check_bit(PS, 6) ? 'V' : 'v';
    status_string[2] = check_bit(PS, 5) ? 'U' : 'u';
    status_string[3] = check_bit(PS, 4) ? 'B' : 'b';
    status_string[4] = check_bit(PS, 3) ? 'D' : 'd';
    status_string[5] = check_bit(PS, 2) ? 'I' : 'i';
    status_string[6] = check_bit(PS, 1) ? 'Z' : 'z';
    status_string[7] = check_bit(PS, 0) ? 'C' : 'c';
    if (addressing[opcode].bytes == 3)
    {
        sprintf(log_line, "%04X %02X %02X %02X %s\tA:%02X X:%02X Y:%02X P:%02X SP:%02X LINE: %03d %s\n", PC, opcode, first, second, fn_name, A, X, Y, PS, SP, current_line, status_string);
    }
    else if (addressing[opcode].bytes == 2)
    {
        sprintf(log_line, "%04X %02X %02X    %s\tA:%02X X:%02X Y:%02X P:%02X SP:%02X LINE: %03d %s\n", PC, opcode, first, fn_name, A, X, Y, PS, SP, current_line, status_string);
    }
    else
    {
        sprintf(log_line, "%04X %02X       %s\tA:%02X X:%02X Y:%02X P:%02X SP:%02X LINE: %03d %s\n", PC, opcode, fn_name, A, X, Y, PS, SP, current_line, status_string);
    }

    // for (int i = 0; log_line[i] != '\0'; i++)
    // {
    //     if (log_line[i] >= 'a' && log_line[i] <= 'z')
    //     {
    //         log_line[i] = log_line[i] - 32;
    //     }
    // }

    printf(log_line);
}

int is_jump(unsigned char *fn_name)
{
    if (
        (strncmp(fn_name, "BRK", 3) != 0) &&
        (strncmp(fn_name, "JMP", 3) != 0) &&
        (strncmp(fn_name, "JSR", 3) != 0) &&
        (strncmp(fn_name, "RTI", 3) != 0))
    {
        return 0;
    }
    return 1;
}

int cpu_interrupt_count = 0;

int must_handle_interrupt()
{
    if (interrupt_handled == 1)
    {
        return 0;
    }
    if ((interrupt_occurred == NMI_INT) || ((interrupt_occurred == IRQ_INT) && (check_bit(PS, ID) == 0)))
    {
        if (cpu_interrupt_count >= 6) {
            cpu_interrupt_count = 0;
            return 1;
        } else {
            cpu_interrupt_count++;
        }
    }
    return 0;
}

void dump_ram()
{
    FILE *dump_file = fopen("dump.txt", "w");
    for (int i = 0; i < sizeof(RAM); i++)
    {
        if ((i % 8) == 0)
        {
            fprintf(dump_file, "\n%04X ", i);
        }
        fprintf(dump_file, "%02X ", RAM[i]);
    }
}

void main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Usage: %s filepath\n", argv[0]);
    }

    init_ram();
    load_rom(argv[1]);
    int cnt = 0;
    // load_rom("/home/gabriele/Downloads/cpu_test/cpu_dummy_writes/cpu_dummy_writes_oam.nes");
    // load_rom("/home/gabriele/Downloads/cpu_test/cpu_dummy_reads.nes");
    // load_rom("/home/gabriele/Downloads/dk.nes");
    // load_rom("/home/gabriele/Downloads/bf.nes");
    // load_rom("/home/gabriele/Downloads/p/palette_pal.nes");
    // load_rom("/home/gabriele/Downloads/color_test.nes");
    // load_rom("/home/gabriele/Downloads/cpu_test/nestest.nes");
    // load_rom("/home/gabriele/Downloads/cpu_test/cpu_dummy_reads.nes");
    // load_rom("/home/gabriele/Downloads/ppu_read_buffer/test_ppu_read_buffer.nes");
    // load_rom("/home/gabriele/Downloads/scanline.nes");
    // load_rom("/home/gabriele/Downloads/pm.nes");
    // load_rom("/home/gabriele/Downloads/branch_timing_tests/1.Branch_Basics.nes");
    // printf("%d\n", rom.header.prg_blocks);
    // printf("%s %d %d %d %d %d %d %d\n", rom.header.nes, rom.header.prg_blocks, rom.header.chr_blocks, rom.header.flags_6, rom.header.flags_7, rom.header.flags_8, rom.header.flags_9, rom.header.flags_10, rom.header.padding[5]);

    unsigned char opcode = 0;
    unsigned char value = 0;
    unsigned char first;
    unsigned char second;

    init_ppu();

    SP = 0xFD;
    A = 0;
    X = 0;
    Y = 0;

    // PC = 0xC000; // Test mode, use log compare
    // JMP(0xFC, 0xFF, INDIRECT);
    PC = (RAM[0xFFFD] << 8) | RAM[0xFFFC];
    // PC = 34187;
    while (PC > 0)
    {
        SDL_Event e;
		SDL_PollEvent(&e);
		// TODO: Exit gracefully
		if (e.type == SDL_QUIT) {
			exit(1);
		}
		if (e.type == SDL_KEYDOWN) {
			if (e.key.keysym.sym == 0x1B) {
				exit(1);
			}
		}

        if (must_handle_interrupt(interrupt_occurred))
        {
            if (interrupt_occurred == NMI_INT)
            {
                NMI();
            }
            else
            {
                IRQ();
            }
            interrupt_handled = 1;
        }
        opcode = RAM[PC];
        if (addressing[opcode].cycles == 0)
        {
            PC++;
            continue;
        }
        first = (RAM[PC + 1]);
        second = (RAM[PC + 2]);

        unsigned char fn_name[4];
        memset(fn_name, 0, 4);
        strncpy(fn_name, addressing[opcode].name, 3);

        // log_to_screen(opcode, first, second, fn_name);

        cnt++;
        if (PC == 0xe944) {
            int d = 0;
        }

        void (*fun_ptr)(unsigned char, unsigned char, unsigned char) = addressing[opcode].opcode_fun;
        (*fun_ptr)(first, second, addressing[opcode].addr_mode);

        // if (opcode == 166) {
        //      dump_ram();
        //      exit(1);
        // }

        if ((is_jump(fn_name) == 0) && (PC > 0))
        {
            PC += addressing[opcode].bytes;
        }

        cycles += addressing[opcode].cycles;
        ppu_clock(addressing[opcode].cycles);
    }
}
