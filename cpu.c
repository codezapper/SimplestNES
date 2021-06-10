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
#include "utils.h"

unsigned char RAM[0xFFFF];

int16_t PC = 0;
int16_t SP = 0x100;
unsigned char A = 0;
unsigned char X = 0;
unsigned char Y = 0;
unsigned char PS = 0;

unsigned char CF = 0;
unsigned char ZF = 1;
unsigned char ID = 2;
unsigned char DM = 3;
unsigned char BC = 4;
unsigned char OF = 5;
unsigned char NF = 6;

int cycles_cnt = 0;
unsigned char extra_value = 0;

extern struct addressing_data addressing[0xFF];

void stack_push(unsigned char value) {
    RAM[SP + 0x100] = value;
    SP++;
}

unsigned char stack_pop() {
    unsigned char value = RAM[SP + 0x100];
    SP--;
    return value;
}

unsigned char *get_pointer_to_ram(unsigned char opcode, unsigned char first, unsigned char second) {
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

void init_ram()
{
    memset(RAM, 0, sizeof(RAM));
}

void ADC(unsigned char *address) {
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
}

void AND(unsigned char *address) {
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

void ASL(unsigned char *address) {
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

void BCC(unsigned char *address) {
    addressing[0x90].cycles = 2;
    if (check_bit(PS, CF) == 0) {
        PC += *address;
        addressing[0x90].cycles = 3;
    }
}

void BCS(unsigned char *address) {
    addressing[0xB0].cycles = 2;
    if (check_bit(PS, CF) == 1) {
        PC += *address;
        addressing[0xB0].cycles = 3;
    }
}

void BEQ(unsigned char *address) {
    addressing[0xF0].cycles = 2;
    if (check_bit(PS, ZF) == 1) {
        PC += *address;
        addressing[0xF0].cycles = 3;
    }
}

void BIT(unsigned char *address) {
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

void BMI(unsigned char *address) {
    addressing[0xF0].cycles = 2;
    if (NF == 1) {
        PC += *address;
        addressing[0xF0].cycles = 3;
    }
}

void BNE(unsigned char *address) {
    addressing[0xD0].cycles = 2;
    if (check_bit(PS, ZF) == 0) {
        PC += *address;
        addressing[0xD0].cycles = 3;
    }
}

void BPL(unsigned char *address) {
    addressing[0x10].cycles = 2;
    if (check_bit(PS, NF) == 0) {
        PC += *address;
        addressing[0x10].cycles = 3;
    }
}

void BRK(unsigned char *address) {
    stack_push(PC >> 8);
    stack_push(PC);
    stack_push(PS);
    PC = (RAM[0xFFFF] << 8) | RAM[0xFFFE];
    PS = set_bit(PS, BC);
}

void BVC(unsigned char *address) {
    addressing[0x50].cycles = 2;
    if (check_bit(PS, OF) == 0) {
        PC += *address;
        addressing[0x50].cycles = 3;
    }
}

void BVS(unsigned char *address) {
    addressing[0x70].cycles = 2;
    if (check_bit(PS, OF) == 0) {
        PC += *address;
        addressing[0x70].cycles = 3;
    }
}

void CLC(unsigned char *address) {
    PS = clear_bit(PS, CF);
}

void CLD(unsigned char *address) {
    PS = clear_bit(PS, DM);
}

void CLI(unsigned char *address) {
    PS = clear_bit(PS, ID);
}

void CLV(unsigned char *address) {
    PS = clear_bit(PS, OF);
}

void CMP(unsigned char *address) {
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

void CPX(unsigned char *address) {
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

void CPY(unsigned char *address) {
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

void DEC(unsigned char *address) {
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

void DEX(unsigned char *address) {
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

void DEY(unsigned char *address) {
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

void EOR(unsigned char *address) {
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

void INC(unsigned char *address) {
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

void INX(unsigned char *address) {
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

void INY(unsigned char *address) {
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

void JMP(unsigned char *address) {
    PC = *address;
}

void JSR(unsigned char *address) {
    int return_point = PC + 2;
    stack_push(return_point >> 8);
    stack_push(return_point & 0x00FF);

    PC = *address;
}

void LDA(unsigned char *address) {
    PS = clear_bit(PS, ZF);
    PS = clear_bit(PS, NF);

    A = *address;
    if (0 == A) {
        PS = set_bit(PS, ZF);
    }
    if (check_bit(A, 7)) {
        PS = set_bit(PS, NF);
    }
}

void LDX(unsigned char *address) {
    PS = clear_bit(PS, ZF);
    PS = clear_bit(PS, NF);

    X = *address;
    if (0 == X) {
        PS = set_bit(PS, ZF);
    }
    if (check_bit(X, 7)) {
        PS = set_bit(PS, NF);
    }
}

void LDY(unsigned char *address) {
    PS = clear_bit(PS, ZF);
    PS = clear_bit(PS, NF);

    Y = *address;
    if (0 == Y) {
        PS = set_bit(PS, ZF);
    }
    if (check_bit(Y, 7)) {
        PS = set_bit(PS, NF);
    }
}

void LSR(unsigned char *address) {
    PS = clear_bit(PS, CF);
    PS = clear_bit(PS, ZF);
    PS = clear_bit(PS, NF);

    if (check_bit(*address, 0) == 1) {
        PS = set_bit(PS, CF);
    }

    *address <<= 1;
    *address &= 0x00FF;

    if (0 == *address) {
        PS = set_bit(PS, ZF);
    }
    if (check_bit(*address, 7)) {
        PS = set_bit(PS, NF);
    }

}

void NOP(unsigned char *address) {
}

void ORA(unsigned char *address) {
    A |= *address;

    PS = clear_bit(PS, ZF);
    PS = clear_bit(PS, NF);
    if (0 == A) {
        PS = set_bit(PS, ZF);
    }

    if (check_bit(Y-*address, 7) == 1) {
        PS = set_bit(PS, NF);
    }
}

void PHA(unsigned char *address) {
    stack_push(A);
}

void PHP(unsigned char *address) {
    stack_push(PS);
}

void PLA(unsigned char *address) {
    A = stack_pop();
}

void PLP(unsigned char *address) {
    PS = stack_pop();
}

void ROL(unsigned char *address) {
    int value = *address;
    if (check_bit(PS, CF) == 1) {
        value = set_bit(value, 0);
    } else {
        value = clear_bit(value, 0);
    }

    if (check_bit(value, 7) == 1) {
        PS = set_bit(PS, CF);
    } else {
        PS = clear_bit(PS, CF);
    }

    value <<= 1;

    if (check_bit(value, 7) == 1) {
        PS = set_bit(PS, NF);
    } else {
        PS = clear_bit(PS, NF);
    }

    if (0 == value) {
        PS = set_bit(PS, ZF);
    } else {
        PS = set_bit(PS, ZF);
    }

    *address = value;
}


void ROR(unsigned char *address) {
    int value = *address;
    if (check_bit(PS, CF) == 1) {
        value = set_bit(value, 7);
    } else {
        value = clear_bit(value, 7);
    }

    if (check_bit(value, 0) == 1) {
        PS = set_bit(PS, CF);
    } else {
        PS = clear_bit(PS, CF);
    }

    value <<= 1;

    if (check_bit(value, 7) == 1) {
        PS = set_bit(PS, NF);
    } else {
        PS = clear_bit(PS, NF);
    }

    if (0 == value) {
        PS = set_bit(PS, ZF);
    } else {
        PS = set_bit(PS, ZF);
    }

    *address = value;
}

void RTI(unsigned char *address) {
    PS = stack_pop();
}

void RTS(unsigned char *address) {
    PC = stack_pop();
}

void SBC(unsigned char *address) {
    int result = A - *address;

    if (check_bit(PS, CF) == 0) {
        result -= 1;
    }

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
}

void SEC(unsigned char *address) {
    PS = set_bit(PS, CF);
}

void SED(unsigned char *address) {
    PS = set_bit(PS, DM);
}

void SEI(unsigned char *address) {
    PS = set_bit(PS, ID);
}

void STA(unsigned char *address) {
    *address = A;
}

void STX(unsigned char *address) {
    *address = X;
}

void STY(unsigned char *address) {
    *address = Y;
}

void TAX(unsigned char *address) {
    X = A;

    if (0 == X) {
        PS = set_bit(PS, ZF);
    }
    if (check_bit(X, 7)) {
        PS = set_bit(PS, NF);
    }
}

void TAY(unsigned char *address) {
    Y = A;

    if (0 == Y) {
        PS = set_bit(PS, ZF);
    }
    if (check_bit(Y, 7)) {
        PS = set_bit(PS, NF);
    }
}

void TSX(unsigned char *address) {
    X = SP;

    if (0 == X) {
        PS = set_bit(PS, ZF);
    }
    if (check_bit(X, 7)) {
        PS = set_bit(PS, NF);
    }
}

void TXA(unsigned char *address) {
    A = X;

    if (0 == A) {
        PS = set_bit(PS, ZF);
    }
    if (check_bit(A, 7)) {
        PS = set_bit(PS, NF);
    }
}

void TXS(unsigned char *address) {
    SP = 0;
}

void TYA(unsigned char *address) {
    A = Y;

    if (0 == A) {
        PS = set_bit(PS, ZF);
    }
    if (check_bit(A, 7)) {
        PS = set_bit(PS, NF);
    }
}

