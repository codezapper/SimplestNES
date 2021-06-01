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

int8_t CF = 0;
int8_t ZF = 0;
int8_t ID = 0;
int8_t DM = 0;
int8_t BC = 0;
int8_t OF = 0;
int8_t NF = 0;

int cycles_cnt = 0;
char extra_value = 0;

extern struct addressing_data addressing[0xFF];

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
    }
}

short check_bit(int number, int bit) {
    if (number & (1 << bit)) {
        return 1;
    }

    return 0;
}

void init_ram()
{
    memset(RAM, 0, sizeof(RAM));
}

void LDA(char *address) {
    A = *address;
    if (0 == A) {
        ZF = 1;
    }
    if (check_bit(A, 7)) {
        NF = 1;
    }
    PC += 1;
}

void ADC(char *address) {
    int result = A + *address + CF;

    if (result > 256) {
        CF = 1;
    }
    A = result;
    if (0 == A) {
        ZF = 1;
    }
    if (check_bit(A, 7)) {
        NF = 1;
    }

    OF = 0;
    if ((127 < A) || (-128 > A)) {
        OF = 1;
    }
    PC += 1;
}

void AND(char *address) {
    A &= *address;
    if (0 == A) {
        ZF = 1;
    }
    if (check_bit(A, 7)) {
        NF = 1;
    }

}

void ASL(char *address) {
    *address <<= 1;

    ZF = 0;
    if (0 == A) {
        ZF = 1;
    }

    if (check_bit(*address, 7)) {
        NF = 1;
    }

    CF = 0;
    if (*address > 255) {
        *address &= 0xFF;
        CF = 1;
    }
}

void BCC(char *address) {
    addressing[0x90].cycles = 2;
    if (CF == 0) {
        PC += *address;
        addressing[0x90].cycles = 3;
    }
}

void BCS(char *address) {
    addressing[0xB0].cycles = 2;
    if (CF == 1) {
        PC += *address;
        addressing[0xB0].cycles = 3;
    }
}

void BEQ(char *address) {
    addressing[0xF0].cycles = 2;
    if (ZF == 1) {
        PC += *address;
        addressing[0xF0].cycles = 3;
    }
}
