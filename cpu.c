// Memory addresses:
// 0000-00ff -> Zero page
// 0100-01ff -> reserved for system stack
// 0200-0800 -> RAM
// fffa-fffb -> non-maskable interrupt handler
// fffc-fffd -> power on reset location
// fffe-ffff -> BRK/interrupt request handler

// Registers:
// PC -> the program counter, 16 bit in size
// SP -> the stack pointer, has the lowest 8 bit of the address of the stack, 8 bit in size
// A -> the accumulator, used for math and logic operations, 8 bit in size
// X -> a register, can be filled with arbitrary values, special function: can be used to get a copy of SP or change its value. 8 bit in size
// Y -> a register, can be filled with arbitrary values, 8 bit in size

// Processor status:
// This is a special register, in which flags or mode are set or read by setting or reading its bits
// CF -> Carry Flag, it's set when an operation overflow bit 7 or underflows bit 0. It can be explicitly set or cleared with operations SEC and CLC
// ZF -> Zero Flag, it's set if the result of the last operation was zero
// ID -> Interrupt Disable, when set the processor will not respond to interrupts from devices. It can be set and cleared using SEI and CLI
// DM -> Decimal Mode, when set, the processor will use BCD during addition and subtraction.  It can be set and cleared using SED and CLD
// BC -> Break Command, set when a BRK instruction has been executed and an interrupt has been generated to process it.
// OF -> Overflow Flag, set during arithmetic operations if the result has yielded an invalid 2's complement result (e.g. adding to positive numbers and ending up with a negative result: 64 + 64 => -128). It is determined by looking at the carry between bits 6 and 7 and between bit 7 and the carry flag.
// NF -> Negative Flag, set if the result of the last operation had bit 7 set to a one.

// Special:
// cycles_cnt -> A counter of how many cycles an operation will take. This is needed to sync CPU and PPU (and possibly later APU)

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "addressing_modes.h"

char RAM[0xFFFF];
int16_t PC = 0;
int8_t SP = 0;
int8_t A = 0;
int8_t X = 0;
int8_t Y = 0;
int8_t PS = 0;

int8_t CF = 0;
int8_t ZF = 1;
int8_t ID = 2;
int8_t DM = 3;
int8_t BC = 4;
int8_t OF = 5;
int8_t NF = 6;

int cycles_cnt = 0;
char extra_value = 0;

extern struct addressing_data addressing[0xFF];

void stack_push(char value) {
    RAM[SP + 0x100] = value;
    SP--;
}

char stack_pop() {
    char value = RAM[SP + 0x100];
    SP++;
    return value;
}

char *get_pointer_to_ram(int16_t opcode, int16_t first, int16_t second) {
    switch (addressing[opcode].addr_mode) {
        case ACCUMULATOR:
            return &A;
        case IMMEDIATE:
        case RELATIVE:
            extra_value = first;
            return &extra_value;
        case ZEROPAGE:
            return &RAM[first];
        case ZEROPAGEX:
            return &RAM[(first + X) % 256];
        case ZEROPAGEY:
            return &RAM[first + Y];
        case ABSOLUTE:
            return &RAM[first];
        case ABSOLUTEX:
            return &RAM[first + X];
        case ABSOLUTEY:
            return &RAM[first + X];
        case INDIRECTX:
            return &RAM[first + X];
        case INDIRECTY:
            return &RAM[first + Y];
        case INDIRECT:
            //TODO: Implemented indirect for JMP
            return 0;
    }
}

short check_bit(int number, int bit) {
    if (number & (1 << bit)) {
        return 1;
    }

    return 0;
}

char set_bit(char value, int bit) {
    return value | (1 << bit);
}

char clear_bit(char value, int bit) {
    return value | (1 << bit);
}


void init_ram()
{
    memset(RAM, 0, sizeof(RAM));
}

void ADC(char *address) {
    int result = A + *address + CF;
    PS = clear_bit(PS, CF);
    PS = clear_bit(PS, ZF);
    PS = clear_bit(PS, NF);
    PS = clear_bit(PS, OF);

    if (result > 256) {
        PS = set_bit(PS, CF);
    }
    A = result;
    if (0 == A) {
        PS = set_bit(PS, ZF);
    }
    if (check_bit(A, 7)) {
        PS = set_bit(PS, NF);
    }

    if ((127 < A) || (-128 > A)) {
        PS = set_bit(PS, OF);
    }
    PC += 1;
}

void AND(char *address) {
    PS = clear_bit(PS, ZF);
    PS = clear_bit(PS, NF);

    A &= *address;
    if (0 == A) {
        PS = set_bit(PS, ZF);
    }
    if (check_bit(A, 7)) {
        PS = set_bit(PS, NF);
    }
}

void ASL(char *address) {
    *address <<= 1;

    PS = clear_bit(PS, ZF);
    PS = clear_bit(PS, NF);
    PS = clear_bit(PS,CF);

    if (0 == A) {
        PS = set_bit(PS, ZF);
    }

    if (check_bit(*address, 7)) {
        PS = set_bit(PS, NF);
    }

    if (*address > 255) {
        *address &= 0xFF;
        PS = set_bit(PS, CF);
    }
}

void BCC(char *address) {
    addressing[0x90].cycles = 2;
    if (check_bit(PS, CF) == 0) {
        PC += *address;
        addressing[0x90].cycles = 3;
    }
}

void BCS(char *address) {
    addressing[0xB0].cycles = 2;
    if (check_bit(PS, CF) == 1) {
        PC += *address;
        addressing[0xB0].cycles = 3;
    }
}

void BEQ(char *address) {
    addressing[0xF0].cycles = 2;
    if (check_bit(PS, ZF) == 1) {
        PC += *address;
        addressing[0xF0].cycles = 3;
    }
}

void BIT(char *address) {
    int result = A & *address;

    PS = clear_bit(PS, ZF);
    PS = clear_bit(PS, NF);
    PS = clear_bit(PS, OF);

    if (0 == result) {
        PS = set_bit(PS, ZF);
    }

    if (check_bit(*address, 7)) {
        PS = set_bit(PS, NF);
    }

    if (check_bit(*address, 6)) {
        PS = set_bit(PS, OF);
    }
}

void BMI(char *address) {
    addressing[0xF0].cycles = 2;
    if (NF == 1) {
        PC += *address;
        addressing[0xF0].cycles = 3;
    }
}

void BNE(char *address) {
    addressing[0xD0].cycles = 2;
    if (check_bit(PS, ZF) == 0) {
        PC += *address;
        addressing[0xD0].cycles = 3;
    }
}

void BPL(char *address) {
    addressing[0x10].cycles = 2;
    if (check_bit(PS, NF) == 0) {
        PC += *address;
        addressing[0x10].cycles = 3;
    }
}

void BRK(char *address) {
    stack_push(PC >> 8);
    stack_push(PC);
    stack_push(PS);
    PC = (RAM[0xFFFF] << 8) | RAM[0xFFFE];
    PS = set_bit(PS, BC);
}

void BVC(char *address) {
    addressing[0x50].cycles = 2;
    if (check_bit(PS, OF) == 0) {
        PC += *address;
        addressing[0x50].cycles = 3;
    }
}

void BVS(char *address) {
    addressing[0x70].cycles = 2;
    if (check_bit(PS, OF) == 0) {
        PC += *address;
        addressing[0x70].cycles = 3;
    }
}

void CLC(char *address) {
    PS = clear_bit(PS, CF);
}

void CLD(char *address) {
    PS = clear_bit(PS, DM);
}

void CLI(char *address) {
    PS = clear_bit(PS, ID);
}

void CLV(char *address) {
    PS = clear_bit(PS, OF);
}

void CMP(char *address) {
    PS = clear_bit(PS, CF);
    PS = clear_bit(PS, ZF);
    PS = clear_bit(PS, NF);

    if (A >= *address) {
        PS = set_bit(PS, CF);
    }

    if (A == *address) {
        PS = set_bit(PS, ZF);
    }

    if (check_bit(A-*address, 7) == 1) {
        PS = set_bit(PS, NF);
    }
}

void CPX(char *address) {
    PS = clear_bit(PS, CF);
    PS = clear_bit(PS, ZF);
    PS = clear_bit(PS, NF);

    if (X >= *address) {
        PS = set_bit(PS, CF);
    }

    if (X == *address) {
        PS = set_bit(PS, ZF);
    }

    if (check_bit(A-*address, 7) == 1) {
        PS = set_bit(PS, NF);
    }
}

void CPY(char *address) {
    PS = clear_bit(PS, CF);
    PS = clear_bit(PS, ZF);
    PS = clear_bit(PS, NF);
    if (Y >= *address) {
        PS = set_bit(PS, CF);
    }

    if (Y == *address) {
        PS = set_bit(PS, ZF);
    }

    if (check_bit(Y-*address, 7) == 1) {
        PS = set_bit(PS, NF);
    }
}

void DEC(char *address) {
    *address -= 1;

    PS = clear_bit(PS, ZF);
    PS = clear_bit(PS, NF);
    if (0 == *address) {
        PS = set_bit(PS, ZF);
    }

    if (check_bit(Y-*address, 7) == 1) {
        PS = set_bit(PS, NF);
    }
}

void DEX(char *address) {
    X -= 1;

    PS = clear_bit(PS, ZF);
    PS = clear_bit(PS, NF);
    if (0 == X) {
        PS = set_bit(PS, ZF);
    }

    if (check_bit(Y-*address, 7) == 1) {
        PS = set_bit(PS, NF);
    }
}

void DEY(char *address) {
    Y -= 1;

    PS = clear_bit(PS, ZF);
    PS = clear_bit(PS, NF);
    if (0 == Y) {
        PS = set_bit(PS, ZF);
    }

    if (check_bit(Y-*address, 7) == 1) {
        PS = set_bit(PS, NF);
    }
}

void EOR(char *address) {
    A ^= *address;

    PS = clear_bit(PS, ZF);
    PS = clear_bit(PS, NF);
    if (0 == A) {
        PS = set_bit(PS, ZF);
    }

    if (check_bit(Y-*address, 7) == 1) {
        PS = set_bit(PS, NF);
    }
}

void INC(char *address) {
    *address += 1;

    PS = clear_bit(PS, ZF);
    PS = clear_bit(PS, NF);
    if (0 == *address) {
        PS = set_bit(PS, ZF);
    }

    if (check_bit(Y-*address, 7) == 1) {
        PS = set_bit(PS, NF);
    }
}

void INX(char *address) {
    X += 1;

    PS = clear_bit(PS, ZF);
    PS = clear_bit(PS, NF);
    if (0 == X) {
        PS = set_bit(PS, ZF);
    }

    if (check_bit(Y-*address, 7) == 1) {
        PS = set_bit(PS, NF);
    }
}

void INY(char *address) {
    Y += 1;

    PS = clear_bit(PS, ZF);
    PS = clear_bit(PS, NF);
    if (0 == Y) {
        PS = set_bit(PS, ZF);
    }

    if (check_bit(Y-*address, 7) == 1) {
        PS = set_bit(PS, NF);
    }
}

void JMP(char *address) {
    PC = *address;
}

void JSR(char *address) {
    int return_point = PC + 2;
    stack_push(return_point >> 8);
    stack_push(return_point & 0x00FF);

    PC = *address;
}

void LDA(char *address) {
    PS = clear_bit(PS, ZF);
    PS = clear_bit(PS, NF);

    A = *address;
    if (0 == A) {
        PS = set_bit(PS, ZF);
    }
    if (check_bit(A, 7)) {
        PS = set_bit(PS, NF);
    }
    PC += 1;
}

void LDX(char *address) {
    PS = clear_bit(PS, ZF);
    PS = clear_bit(PS, NF);

    X = *address;
    if (0 == X) {
        PS = set_bit(PS, ZF);
    }
    if (check_bit(X, 7)) {
        PS = set_bit(PS, NF);
    }
    PC += 1;
}

void LDY(char *address) {
    PS = clear_bit(PS, ZF);
    PS = clear_bit(PS, NF);

    Y = *address;
    if (0 == Y) {
        PS = set_bit(PS, ZF);
    }
    if (check_bit(Y, 7)) {
        PS = set_bit(PS, NF);
    }
    PC += 1;
}
