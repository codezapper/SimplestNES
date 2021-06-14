// Memory addresses (see https://wiki.nesdev.com/w/index.php/CPU_memory_map):
// $0000-$07FF 	$0800 	2KB internal RAM
// $0800-$0FFF 	$0800 	Mirrors of $0000-$07FF
// $1000-$17FF 	$0800
// $1800-$1FFF 	$0800
// $2000-$2007 	$0008 	NES PPU registers
// $2008-$3FFF 	$1FF8 	Mirrors of $2000-2007 (repeats every 8 bytes)
// $4000-$4017 	$0018 	NES APU and I/O registers
// $4018-$401F 	$0008 	APU and I/O functionality that is normally disabled. See CPU Test Mode.
// $4020-$FFFF 	$BFE0 	Cartridge space: PRG ROM, PRG RAM, and mapper registers (See Note) 

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

uint16_t PC = 0;
uint16_t SP = 0x100;
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

uint16_t read_value_from_params(unsigned char first, unsigned char second, unsigned char addr_mode) {
    switch (addr_mode) {
        case ACCUMULATOR:
            return A;
        case IMMEDIATE:
            return first;
        case RELATIVE:
            return RAM[first];
        case ZEROPAGE:
            return first;
        case ZEROPAGEX:
            return (first + X) % 256;
        case ZEROPAGEY:
            return first + Y;
        case ABSOLUTE:
            int address = first;
            address = (second << 8) |address;
            return address;
        case ABSOLUTEX:
            return first + X;
        case ABSOLUTEY:
            return first + Y;
        case INDIRECTX:
            return first + X;
        case INDIRECTY:
            return first + Y;
        case INDIRECT:
            //TODO: Implemented indirect for JMP
            return 0;
    }
}

void write_value_from_params(unsigned char first, unsigned char second, unsigned char value, unsigned char addr_mode) {
    switch (addr_mode) {
        // case ACCUMULATOR:
        //     return A;
        // case IMMEDIATE:
        //     return first;
        // case RELATIVE:
        //     return RAM[first];
        case ZEROPAGE:
            RAM[first] = value;
            break;
        case ZEROPAGEX:
            RAM[first] = (value + X) % 256;
            break;
        case ZEROPAGEY:
            RAM[first] = value + Y;
            break;
        case ABSOLUTE:
            int address = first;
            address = (second << 8) |address;
            RAM[address] = value;
        case ABSOLUTEX:
            RAM[first + X] = value;
            break;
        case ABSOLUTEY:
            RAM[first + Y] = value;
            break;
        case INDIRECTX:
            RAM[first + X] = value;
            break;
        case INDIRECTY:
            RAM[first + Y] = value;
            break;
        case INDIRECT:
            //TODO: Implemented indirect for JMP
            break;
    }
}

void init_ram()
{
    memset(RAM, 0, sizeof(RAM));
}

void ADC(unsigned char first, unsigned char second, unsigned char addr_mode) {
    int result = A + read_value_from_params(first, second, addr_mode) + CF;
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

void AND(unsigned char first, unsigned char second, unsigned char addr_mode) {
    PS = clear_bit(PS, ZF);
    PS = clear_bit(PS, NF);

    A &= read_value_from_params(first, second, addr_mode);
    if (0 == A) {
        PS = set_bit(PS, ZF);
    }
    if (check_bit(A, 7)) {
        PS = set_bit(PS, NF);
    }
}

void ASL(unsigned char first, unsigned char second, unsigned char addr_mode) {
    unsigned char value = read_value_from_params(first, second, addr_mode);
    value <<= 1;

    write_value_from_params(first, second, value, addr_mode);

    PS = clear_bit(PS, ZF);
    PS = clear_bit(PS, NF);
    PS = clear_bit(PS,CF);

    if (0 == A) {
        PS = set_bit(PS, ZF);
    }

    if (check_bit(value, 7)) {
        PS = set_bit(PS, NF);
    }

    if (value > 255) {
        value &= 0xFF;
        write_value_from_params(first, second, value, addr_mode);
        PS = set_bit(PS, CF);
    }
}

void BCC(unsigned char first, unsigned char second, unsigned char addr_mode) {
    addressing[0x90].cycles = 2;
    if (check_bit(PS, CF) == 0) {
        PC += read_value_from_params(first, second, addr_mode);
        addressing[0x90].cycles = 3;
    }
}

void BCS(unsigned char first, unsigned char second, unsigned char addr_mode) {
    addressing[0xB0].cycles = 2;
    if (check_bit(PS, CF) == 1) {
        PC += read_value_from_params(first, second, addr_mode);
        addressing[0xB0].cycles = 3;
    }
}

void BEQ(unsigned char first, unsigned char second, unsigned char addr_mode) {
    addressing[0xF0].cycles = 2;
    if (check_bit(PS, ZF) == 1) {
        PC += read_value_from_params(first, second, addr_mode);
        addressing[0xF0].cycles = 3;
    }
}

void BIT(unsigned char first, unsigned char second, unsigned char addr_mode) {
    int result = A & read_value_from_params(first, second, addr_mode);

    PS = clear_bit(PS, ZF);
    PS = clear_bit(PS, NF);
    PS = clear_bit(PS, OF);

    if (0 == result) {
        PS = set_bit(PS, ZF);
    }

    if (check_bit(read_value_from_params(first, second, addr_mode), 7)) {
        PS = set_bit(PS, NF);
    }

    if (check_bit(read_value_from_params(first, second, addr_mode), 6)) {
        PS = set_bit(PS, OF);
    }
}

void BMI(unsigned char first, unsigned char second, unsigned char addr_mode) {
    addressing[0xF0].cycles = 2;
    if (NF == 1) {
        PC += read_value_from_params(first, second, addr_mode);
        addressing[0xF0].cycles = 3;
    }
}

void BNE(unsigned char first, unsigned char second, unsigned char addr_mode) {
    addressing[0xD0].cycles = 2;
    if (check_bit(PS, ZF) == 0) {
        PC += read_value_from_params(first, second, addr_mode);
        addressing[0xD0].cycles = 3;
    }
}

void BPL(unsigned char first, unsigned char second, unsigned char addr_mode) {
    addressing[0x10].cycles = 2;
    if (check_bit(PS, NF) == 0) {
        PC += read_value_from_params(first, second, addr_mode);
        addressing[0x10].cycles = 3;
    }
}

void BRK(unsigned char first, unsigned char second, unsigned char addr_mode) {
    stack_push(PC >> 8);
    stack_push(PC);
    stack_push(PS);
    PC = (RAM[0xFFFF] << 8) | RAM[0xFFFE];
    PS = set_bit(PS, BC);
}

void BVC(unsigned char first, unsigned char second, unsigned char addr_mode) {
    addressing[0x50].cycles = 2;
    if (check_bit(PS, OF) == 0) {
        PC += read_value_from_params(first, second, addr_mode);
        addressing[0x50].cycles = 3;
    }
}

void BVS(unsigned char first, unsigned char second, unsigned char addr_mode) {
    addressing[0x70].cycles = 2;
    if (check_bit(PS, OF) == 0) {
        PC += read_value_from_params(first, second, addr_mode);
        addressing[0x70].cycles = 3;
    }
}

void CLC(unsigned char first, unsigned char second, unsigned char addr_mode) {
    PS = clear_bit(PS, CF);
}

void CLD(unsigned char first, unsigned char second, unsigned char addr_mode) {
    PS = clear_bit(PS, DM);
}

void CLI(unsigned char first, unsigned char second, unsigned char addr_mode) {
    PS = clear_bit(PS, ID);
}

void CLV(unsigned char first, unsigned char second, unsigned char addr_mode) {
    PS = clear_bit(PS, OF);
}

void CMP(unsigned char first, unsigned char second, unsigned char addr_mode) {
    PS = clear_bit(PS, CF);
    PS = clear_bit(PS, ZF);
    PS = clear_bit(PS, NF);

    if (A >= read_value_from_params(first, second, addr_mode)) {
        PS = set_bit(PS, CF);
    }

    if (A == read_value_from_params(first, second, addr_mode)) {
        PS = set_bit(PS, ZF);
    }

    if (check_bit(A-read_value_from_params(first, second, addr_mode), 7) == 1) {
        PS = set_bit(PS, NF);
    }
}

void CPX(unsigned char first, unsigned char second, unsigned char addr_mode) {
    PS = clear_bit(PS, CF);
    PS = clear_bit(PS, ZF);
    PS = clear_bit(PS, NF);

    if (X >= read_value_from_params(first, second, addr_mode)) {
        PS = set_bit(PS, CF);
    }

    if (X == read_value_from_params(first, second, addr_mode)) {
        PS = set_bit(PS, ZF);
    }

    if (check_bit(A-read_value_from_params(first, second, addr_mode), 7) == 1) {
        PS = set_bit(PS, NF);
    }
}

void CPY(unsigned char first, unsigned char second, unsigned char addr_mode) {
    PS = clear_bit(PS, CF);
    PS = clear_bit(PS, ZF);
    PS = clear_bit(PS, NF);
    unsigned char value = read_value_from_params(first, second, addr_mode);

    if (Y >= value) {
        PS = set_bit(PS, CF);
    }

    if (Y == value) {
        PS = set_bit(PS, ZF);
    }

    if (check_bit(Y-value, 7) == 1) {
        PS = set_bit(PS, NF);
    }
}

void DEC(unsigned char first, unsigned char second, unsigned char addr_mode) {
    unsigned char value = read_value_from_params(first, second, addr_mode);

    write_value_from_params(first, second, value-1, addr_mode);

    PS = clear_bit(PS, ZF);
    PS = clear_bit(PS, NF);
    if (0 == read_value_from_params(first, second, addr_mode)) {
        PS = set_bit(PS, ZF);
    }

    if (check_bit(Y-read_value_from_params(first, second, addr_mode), 7) == 1) {
        PS = set_bit(PS, NF);
    }
}

void DEX(unsigned char first, unsigned char second, unsigned char addr_mode) {
    X -= 1;

    PS = clear_bit(PS, ZF);
    PS = clear_bit(PS, NF);
    if (0 == X) {
        PS = set_bit(PS, ZF);
    }

    if (check_bit(Y-read_value_from_params(first, second, addr_mode), 7) == 1) {
        PS = set_bit(PS, NF);
    }
}

void DEY(unsigned char first, unsigned char second, unsigned char addr_mode) {
    Y -= 1;

    PS = clear_bit(PS, ZF);
    PS = clear_bit(PS, NF);
    if (0 == Y) {
        PS = set_bit(PS, ZF);
    }

    if (check_bit(Y-read_value_from_params(first, second, addr_mode), 7) == 1) {
        PS = set_bit(PS, NF);
    }
}

void EOR(unsigned char first, unsigned char second, unsigned char addr_mode) {
    A ^= read_value_from_params(first, second, addr_mode);

    PS = clear_bit(PS, ZF);
    PS = clear_bit(PS, NF);
    if (0 == A) {
        PS = set_bit(PS, ZF);
    }

    if (check_bit(Y-read_value_from_params(first, second, addr_mode), 7) == 1) {
        PS = set_bit(PS, NF);
    }
}

void INC(unsigned char first, unsigned char second, unsigned char addr_mode) {
    unsigned char value = read_value_from_params(first, second, addr_mode);

    write_value_from_params(first, second, value-1, addr_mode);

    PS = clear_bit(PS, ZF);
    PS = clear_bit(PS, NF);
    if (0 == read_value_from_params(first, second, addr_mode)) {
        PS = set_bit(PS, ZF);
    }

    if (check_bit(Y-read_value_from_params(first, second, addr_mode), 7) == 1) {
        PS = set_bit(PS, NF);
    }
}

void INX(unsigned char first, unsigned char second, unsigned char addr_mode) {
    X += 1;

    PS = clear_bit(PS, ZF);
    PS = clear_bit(PS, NF);
    if (0 == X) {
        PS = set_bit(PS, ZF);
    }

    if (check_bit(Y-read_value_from_params(first, second, addr_mode), 7) == 1) {
        PS = set_bit(PS, NF);
    }
}

void INY(unsigned char first, unsigned char second, unsigned char addr_mode) {
    Y += 1;

    PS = clear_bit(PS, ZF);
    PS = clear_bit(PS, NF);
    if (0 == Y) {
        PS = set_bit(PS, ZF);
    }

    if (check_bit(Y-read_value_from_params(first, second, addr_mode), 7) == 1) {
        PS = set_bit(PS, NF);
    }
}

void JMP(unsigned char first, unsigned char second, unsigned char addr_mode) {
    uint16_t value = read_value_from_params(first, second, addr_mode);
    PC = read_value_from_params(first, second, addr_mode);
}

void JSR(unsigned char first, unsigned char second, unsigned char addr_mode) {
    int return_point = PC + 2;
    stack_push(return_point >> 8);
    stack_push(return_point & 0x00FF);

    PC  = read_value_from_params(first, second, addr_mode);
}

void LDA(unsigned char first, unsigned char second, unsigned char addr_mode) {
    PS = clear_bit(PS, ZF);
    PS = clear_bit(PS, NF);

    A = read_value_from_params(first, second, addr_mode);
    if (0 == A) {
        PS = set_bit(PS, ZF);
    }
    if (check_bit(A, 7)) {
        PS = set_bit(PS, NF);
    }
}

void LDX(unsigned char first, unsigned char second, unsigned char addr_mode) {
    PS = clear_bit(PS, ZF);
    PS = clear_bit(PS, NF);

    X  = read_value_from_params(first, second, addr_mode);
    if (0 == X) {
        PS = set_bit(PS, ZF);
    }
    if (check_bit(X, 7)) {
        PS = set_bit(PS, NF);
    }
}

void LDY(unsigned char first, unsigned char second, unsigned char addr_mode) {
    PS = clear_bit(PS, ZF);
    PS = clear_bit(PS, NF);

    Y  = read_value_from_params(first, second, addr_mode);
    if (0 == Y) {
        PS = set_bit(PS, ZF);
    }
    if (check_bit(Y, 7)) {
        PS = set_bit(PS, NF);
    }
}

void LSR(unsigned char first, unsigned char second, unsigned char addr_mode) {
    PS = clear_bit(PS, CF);
    PS = clear_bit(PS, ZF);
    PS = clear_bit(PS, NF);

    if (check_bit(read_value_from_params(first, second, addr_mode), 0) == 1) {
        PS = set_bit(PS, CF);
    }

    unsigned char value = read_value_from_params(first, second, addr_mode);

    value <<= 1;
    value &= 0x00FF;

    if (0 == value) {
        PS = set_bit(PS, ZF);
    }
    if (check_bit(read_value_from_params(first, second, addr_mode), 7)) {
        PS = set_bit(PS, NF);
    }

    write_value_from_params(first, second, value, addr_mode);
}

void NOP(unsigned char first, unsigned char second, unsigned char addr_mode) {
}

void ORA(unsigned char first, unsigned char second, unsigned char addr_mode) {
    A |= read_value_from_params(first, second, addr_mode);

    PS = clear_bit(PS, ZF);
    PS = clear_bit(PS, NF);
    if (0 == A) {
        PS = set_bit(PS, ZF);
    }

    if (check_bit(Y-read_value_from_params(first, second, addr_mode), 7) == 1) {
        PS = set_bit(PS, NF);
    }
}

void PHA(unsigned char first, unsigned char second, unsigned char addr_mode) {
    stack_push(A);
}

void PHP(unsigned char first, unsigned char second, unsigned char addr_mode) {
    stack_push(PS);
}

void PLA(unsigned char first, unsigned char second, unsigned char addr_mode) {
    A = stack_pop();
}

void PLP(unsigned char first, unsigned char second, unsigned char addr_mode) {
    PS = stack_pop();
}

void ROL(unsigned char first, unsigned char second, unsigned char addr_mode) {
    int value  = read_value_from_params(first, second, addr_mode);
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

    write_value_from_params(first, second, value, addr_mode);
}


void ROR(unsigned char first, unsigned char second, unsigned char addr_mode) {
    int value  = read_value_from_params(first, second, addr_mode);
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

    write_value_from_params(first, second, value, addr_mode);
}

void RTI(unsigned char first, unsigned char second, unsigned char addr_mode) {
    PS = stack_pop();
}

void RTS(unsigned char first, unsigned char second, unsigned char addr_mode) {
    PC = stack_pop();
}

void SBC(unsigned char first, unsigned char second, unsigned char addr_mode) {
    int result = A - read_value_from_params(first, second, addr_mode);

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

void SEC(unsigned char first, unsigned char second, unsigned char addr_mode) {
    PS = set_bit(PS, CF);
}

void SED(unsigned char first, unsigned char second, unsigned char addr_mode) {
    PS = set_bit(PS, DM);
}

void SEI(unsigned char first, unsigned char second, unsigned char addr_mode) {
    PS = set_bit(PS, ID);
}

void STA(unsigned char first, unsigned char second, unsigned char addr_mode) {
    write_value_from_params(first, second, A, addr_mode);
}

void STX(unsigned char first, unsigned char second, unsigned char addr_mode) {
    write_value_from_params(first, second, X, addr_mode);
}

void STY(unsigned char first, unsigned char second, unsigned char addr_mode) {
    write_value_from_params(first, second, Y, addr_mode);
}

void TAX(unsigned char first, unsigned char second, unsigned char addr_mode) {
    X = A;

    if (0 == X) {
        PS = set_bit(PS, ZF);
    }
    if (check_bit(X, 7)) {
        PS = set_bit(PS, NF);
    }
}

void TAY(unsigned char first, unsigned char second, unsigned char addr_mode) {
    Y = A;

    if (0 == Y) {
        PS = set_bit(PS, ZF);
    }
    if (check_bit(Y, 7)) {
        PS = set_bit(PS, NF);
    }
}

void TSX(unsigned char first, unsigned char second, unsigned char addr_mode) {
    X = SP;

    if (0 == X) {
        PS = set_bit(PS, ZF);
    }
    if (check_bit(X, 7)) {
        PS = set_bit(PS, NF);
    }
}

void TXA(unsigned char first, unsigned char second, unsigned char addr_mode) {
    A = X;

    if (0 == A) {
        PS = set_bit(PS, ZF);
    }
    if (check_bit(A, 7)) {
        PS = set_bit(PS, NF);
    }
}

void TXS(unsigned char first, unsigned char second, unsigned char addr_mode) {
    SP = 0;
}

void TYA(unsigned char first, unsigned char second, unsigned char addr_mode) {
    A = Y;

    if (0 == A) {
        PS = set_bit(PS, ZF);
    }
    if (check_bit(A, 7)) {
        PS = set_bit(PS, NF);
    }
}
