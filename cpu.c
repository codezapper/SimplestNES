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

#include "utils.h"
#include "bus.h"
#include "addressing.h"

unsigned char RAM[0xFFFF];

uint16_t PC = 0;
uint16_t SP = 0xFD;
unsigned char A = 0;
unsigned char X = 0;
unsigned char Y = 0;
unsigned char PS = 0b00000100;

int cycles_cnt = 0;
unsigned char extra_value = 0;

extern unsigned char interrupt_occurred;
extern unsigned char interrupt_handled;

extern struct ROM rom;
// extern struct addressing_data addressing[];

extern int total_cycles;
extern int current_line;

int cycles = 0;
int cpu_interrupt_count = 0;

void stack_push(unsigned char value) {
    RAM[SP + 0x100] = value;
    SP--;
}

unsigned char stack_pop() {
    SP++;
    unsigned char value = RAM[SP + 0x100];
    return value;
}

void push_PC() {
    unsigned char high = PC >> 8;
    unsigned char low = (PC << 8) >> 8;

    stack_push(high);
    stack_push(low);
}

void pop_PC() {
    unsigned char low = stack_pop();
    unsigned char high = stack_pop();

    PC = high;
    PC <<= 8;
    PC |= low;
}

void init_ram()
{
    memset(RAM, 0xFF, sizeof(RAM));
}

void set_OF(unsigned char first, unsigned char second, unsigned char carry) {
    unsigned char first_7 = check_bit(first, 7);
    unsigned char second_7 = check_bit(second, 7);

    if (!first_7 && !second_7 && carry) {
        PS = set_bit(PS, OF);
        return;
    }
    if (first_7 && second_7 && !carry) {
        PS = set_bit(PS, OF);
        return;
    }
    PS = clear_bit(PS, OF);
}

void ADC(unsigned char first, unsigned char second, unsigned char addr_mode) {
    // set_OF(first, second, check_bit(PS, CF));

    PS = clear_bit(PS, ZF);
    PS = clear_bit(PS, NF);
    PS = clear_bit(PS, OF);

    unsigned char value = cpu_read(first, second, addr_mode) & 0xFF;
    uint16_t result = A + value + check_bit(PS, CF);

    PS = clear_bit(PS, CF);

    if ((~(A ^ value) & (A ^ result) & 0x80) > 0) {
        PS = set_bit(PS, OF);
    }

    if (0 == (result & 0xFF)) {
        PS = set_bit(PS, ZF);
    }
    if (check_bit(result, 7)) {
        PS = set_bit(PS, NF);
    }

    if (result > 255) {
        PS = set_bit(PS, CF);
    }

    A = (unsigned char)(result & 0xFF);
}

void AND(unsigned char first, unsigned char second, unsigned char addr_mode) {
    PS = clear_bit(PS, ZF);
    PS = clear_bit(PS, NF);

    A &= cpu_read(first, second, addr_mode);
    if (0 == A) {
        PS = set_bit(PS, ZF);
    }
    if (check_bit(A, 7)) {
        PS = set_bit(PS, NF);
    }
}

void ASL(unsigned char first, unsigned char second, unsigned char addr_mode) {
    PS = clear_bit(PS, CF);
    PS = clear_bit(PS, ZF);
    PS = clear_bit(PS, NF);

    unsigned char value = (unsigned char)(cpu_read(first, second, addr_mode) & 0xFF);

    if (check_bit(value, 7) == 1) {
        PS = set_bit(PS, CF);
    }

    value <<= 1;
    
    if (0 == value) {
        PS = set_bit(PS, ZF);
    }
    if (check_bit(value, 7)) {
        PS = set_bit(PS, NF);
    }

    cpu_write(first, second, addr_mode, value);
}

void BCC(unsigned char first, unsigned char second, unsigned char addr_mode) {
    //addressing[0x90].cycles = 2;
    if (check_bit(PS, CF) == 0) {
        PC += (char)get_address_from_params(first, second, addr_mode);
        //addressing[0x90].cycles = 3;
    }
}

void BCS(unsigned char first, unsigned char second, unsigned char addr_mode) {
    //addressing[0xB0].cycles = 2;
    if (check_bit(PS, CF) == 1) {
        PC += (char)get_address_from_params(first, second, addr_mode);
        //addressing[0xB0].cycles = 3;
    }
}

void BEQ(unsigned char first, unsigned char second, unsigned char addr_mode) {
    //addressing[0xF0].cycles = 2;
    if (check_bit(PS, ZF) == 1) {
        PC += (char)get_address_from_params(first, second, addr_mode);
        //addressing[0xF0].cycles = 3;
    }
}

void BIT(unsigned char first, unsigned char second, unsigned char addr_mode) {
    unsigned char value = cpu_read(first, second, addr_mode);
    unsigned char result = A & value;

    PS = clear_bit(PS, ZF);
    PS = clear_bit(PS, NF);
    PS = clear_bit(PS, OF);

    if (0 == result) {
        PS = set_bit(PS, ZF);
    }

    // PS = (PS & 0x3F) | (value & 0xC0);

    if (check_bit(value, 7)) {
        PS = set_bit(PS, NF);
    }

    if (check_bit(value, 6)) {
        PS = set_bit(PS, OF);
    }
}

void BMI(unsigned char first, unsigned char second, unsigned char addr_mode) {
    //addressing[0xF0].cycles = 2;
    if (check_bit(PS, NF) == 1) {
        PC += (char)get_address_from_params(first, second, addr_mode);
        //addressing[0xF0].cycles = 3;
    }
}

void BNE(unsigned char first, unsigned char second, unsigned char addr_mode) {
    //addressing[0xD0].cycles = 2;
    if (check_bit(PS, ZF) == 0) {
        PC += (char)get_address_from_params(first, second, addr_mode);
        //addressing[0xD0].cycles = 3;
    }
}

void BPL(unsigned char first, unsigned char second, unsigned char addr_mode) {
    //addressing[0x10].cycles = 2;
    if (check_bit(PS, NF) == 0) {
        PC += (char)get_address_from_params(first, second, addr_mode);
        //addressing[0x10].cycles = 3;
    }
}

void BRK(unsigned char first, unsigned char second, unsigned char addr_mode) {
    push_PC();
    stack_push(PS);

    PS = set_bit(PS, B4);
    PS = set_bit(PS, B5);
    PS = set_bit(PS, ID);

    PC = (RAM[0xFFFF] << 8) | RAM[0xFFFE];
}

void BVC(unsigned char first, unsigned char second, unsigned char addr_mode) {
    //addressing[0x50].cycles = 2;
    if (check_bit(PS, OF) == 0) {
        PC += (char)get_address_from_params(first, second, addr_mode);
        //addressing[0x50].cycles = 3;
    }
}

void BVS(unsigned char first, unsigned char second, unsigned char addr_mode) {
    //addressing[0x70].cycles = 2;
    if (check_bit(PS, OF) == 1) {
        PC += (char)get_address_from_params(first, second, addr_mode);
        //addressing[0x70].cycles = 3;
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

    unsigned char value = (unsigned char)(cpu_read(first, second, addr_mode) & 0xFF);

    if (A >= value) {
        PS = set_bit(PS, CF);
    }

    if (A == value) {
        PS = set_bit(PS, ZF);
    }

    if (check_bit(A-value, 7) == 1) {
        PS = set_bit(PS, NF);
    }
}

void CPX(unsigned char first, unsigned char second, unsigned char addr_mode) {
    PS = clear_bit(PS, CF);
    PS = clear_bit(PS, ZF);
    PS = clear_bit(PS, NF);

    unsigned char value = (unsigned char)(cpu_read(first, second, addr_mode) & 0xFF);

    if (X >= value) {
        PS = set_bit(PS, CF);
    }

    if (X == value) {
        PS = set_bit(PS, ZF);
    }

    if (check_bit(X-value, 7) == 1) {
        PS = set_bit(PS, NF);
    }
}

void CPY(unsigned char first, unsigned char second, unsigned char addr_mode) {
    PS = clear_bit(PS, CF);
    PS = clear_bit(PS, ZF);
    PS = clear_bit(PS, NF);
    unsigned char value = (unsigned char)(cpu_read(first, second, addr_mode) & 0xFF);

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
    unsigned char value = (unsigned char)(cpu_read(first, second, addr_mode) & 0xFF) - 1;

    cpu_write(first, second, addr_mode, value);

    PS = clear_bit(PS, ZF);
    PS = clear_bit(PS, NF);
    if (0 == cpu_read(first, second, addr_mode)) {
        PS = set_bit(PS, ZF);
    }

    if (check_bit(value, 7) == 1) {
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

    if (check_bit(X, 7) == 1) {
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

    if (check_bit(Y, 7) == 1) {
        PS = set_bit(PS, NF);
    }
}

void EOR(unsigned char first, unsigned char second, unsigned char addr_mode) {
    A ^= cpu_read(first, second, addr_mode);

    PS = clear_bit(PS, ZF);
    PS = clear_bit(PS, NF);
    if (0 == A) {
        PS = set_bit(PS, ZF);
    }

    if (check_bit(A, 7) == 1) {
        PS = set_bit(PS, NF);
    }
}

void INC(unsigned char first, unsigned char second, unsigned char addr_mode) {
    unsigned char value = cpu_read(first, second, addr_mode) + 1;

    cpu_write(first, second, addr_mode, value);

    PS = clear_bit(PS, ZF);
    PS = clear_bit(PS, NF);
    if (0 == value) {
        PS = set_bit(PS, ZF);
    }

    if (check_bit(value, 7) == 1) {
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

    if (check_bit(X, 7) == 1) {
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

    if (check_bit(Y, 7) == 1) {
        PS = set_bit(PS, NF);
    }
}

void JMP(unsigned char first, unsigned char second, unsigned char addr_mode) {
    PC = get_address_from_params(first, second, addr_mode);
}

void JSR(unsigned char first, unsigned char second, unsigned char addr_mode) {
    PC += 2; // Set return point
    push_PC();

    PC = get_address_from_params(first, second, addr_mode);
}

void LDA(unsigned char first, unsigned char second, unsigned char addr_mode) {
    PS = clear_bit(PS, ZF);
    PS = clear_bit(PS, NF);

    A = cpu_read(first, second, addr_mode);
    if (A == 0x7c) {
        int d = 0;
    }
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

    X  = cpu_read(first, second, addr_mode);
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

    Y  = cpu_read(first, second, addr_mode);
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

    unsigned char value = (unsigned char)(cpu_read(first, second, addr_mode) & 0xFF);

    if (check_bit(value, 0) == 1) {
        PS = set_bit(PS, CF);
    }

    value >>= 1;
    
    if (0 == value) {
        PS = set_bit(PS, ZF);
    }
    if (check_bit(value, 7)) {
        PS = set_bit(PS, NF);
    }

    cpu_write(first, second, addr_mode, value);
}

void NOP(unsigned char first, unsigned char second, unsigned char addr_mode) {
}

void ORA(unsigned char first, unsigned char second, unsigned char addr_mode) {
    A |= cpu_read(first, second, addr_mode);

    PS = clear_bit(PS, ZF);
    PS = clear_bit(PS, NF);
    if (0 == A) {
        PS = set_bit(PS, ZF);
    }

    if (check_bit(A, 7) == 1) {
        PS = set_bit(PS, NF);
    }
}

void PHA(unsigned char first, unsigned char second, unsigned char addr_mode) {
    stack_push(A);
}

void PHP(unsigned char first, unsigned char second, unsigned char addr_mode) {
    unsigned char value = set_bit(PS, B4);
    value = set_bit(value, B5);

    stack_push(value);
    // stack_push(PS);
}

void PLA(unsigned char first, unsigned char second, unsigned char addr_mode) {
    PS = clear_bit(PS, ZF);
    PS = clear_bit(PS, NF);

    A = stack_pop();

    if (0 == A) {
        PS = set_bit(PS, ZF);
    }

    if (check_bit(A, 7)) {
        PS = set_bit(PS, NF);
    }
}

void PLP(unsigned char first, unsigned char second, unsigned char addr_mode) {
    unsigned char value = stack_pop();

    // Need to ignore B4 and B5

    PS = clear_bit(PS, 0);
    PS = clear_bit(PS, 1);
    PS = clear_bit(PS, 2);
    PS = clear_bit(PS, 3);
    PS = clear_bit(PS, 6);
    PS = clear_bit(PS, 7);
    if (check_bit(value, 0) == 1) {
        PS = set_bit(PS, 0);
    }
    if (check_bit(value, 1) == 1) {
        PS = set_bit(PS, 1);
    }
    if (check_bit(value, 2) == 1) {
        PS = set_bit(PS, 2);
    }
    if (check_bit(value, 3) == 1) {
        PS = set_bit(PS, 3);
    }
    if (check_bit(value, 6) == 1) {
        PS = set_bit(PS, 6);
    }
    if (check_bit(value, 7) == 1) {
        PS = set_bit(PS, 7);
    }
}

void ROL(unsigned char first, unsigned char second, unsigned char addr_mode) {
    int value = cpu_read(first, second, addr_mode);
    int prev_value = value;

    value <<= 1;

    if (check_bit(PS, CF) == 1) {
        value = set_bit(value, 0);
    } else {
        value = clear_bit(value, 0);
    }

    if (check_bit(prev_value, 7) == 1) {
        PS = set_bit(PS, CF);
    } else {
        PS = clear_bit(PS, CF);
    }

    if (check_bit(value, 7) == 1) {
        PS = set_bit(PS, NF);
    } else {
        PS = clear_bit(PS, NF);
    }

    if (0 == value) {
        PS = set_bit(PS, ZF);
    } else {
        PS = clear_bit(PS, ZF);
    }

    cpu_write(first, second, addr_mode, value);
}

void ROR(unsigned char first, unsigned char second, unsigned char addr_mode) {
    int value = cpu_read(first, second, addr_mode);
    int prev_value = value;

    value >>= 1;

    if (check_bit(PS, CF) == 1) {
        value = set_bit(value, 7);
    } else {
        value = clear_bit(value, 7);
    }

    if (check_bit(prev_value, 0) == 1) {
        PS = set_bit(PS, CF);
    } else {
        PS = clear_bit(PS, CF);
    }

    if (check_bit(value, 7) == 1) {
        PS = set_bit(PS, NF);
    } else {
        PS = clear_bit(PS, NF);
    }

    if (0 == value) {
        PS = set_bit(PS, ZF);
    } else {
        PS = clear_bit(PS, ZF);
    }

    cpu_write(first, second, addr_mode, value);
}

void RTI(unsigned char first, unsigned char second, unsigned char addr_mode) {
    unsigned char value = stack_pop();

    // Need to ignore B4 and B5

    PS = clear_bit(PS, 0);
    PS = clear_bit(PS, 1);
    PS = clear_bit(PS, 2);
    PS = clear_bit(PS, 3);
    PS = clear_bit(PS, 6);
    PS = clear_bit(PS, 7);
    if (check_bit(value, 0) == 1) {
        PS = set_bit(PS, 0);
    }
    if (check_bit(value, 1) == 1) {
        PS = set_bit(PS, 1);
    }
    if (check_bit(value, 2) == 1) {
        PS = set_bit(PS, 2);
    }
    if (check_bit(value, 3) == 1) {
        PS = set_bit(PS, 3);
    }
    if (check_bit(value, 6) == 1) {
        PS = set_bit(PS, 6);
    }
    if (check_bit(value, 7) == 1) {
        PS = set_bit(PS, 7);
    }

    pop_PC();
}

void RTS(unsigned char first, unsigned char second, unsigned char addr_mode) {
    pop_PC();
}

void SBC(unsigned char first, unsigned char second, unsigned char addr_mode) {
    PS = clear_bit(PS, ZF);
    PS = clear_bit(PS, NF);
    PS = clear_bit(PS, OF);

    unsigned char value = ~cpu_read(first, second, addr_mode) & 0xFF;
    uint16_t result = A + value + check_bit(PS, CF);

    if ((~(A ^ value) & (A ^ result) & 0x80) > 0) {
        PS = set_bit(PS, OF);
    }

    if (0 == (result & 0xFF)) {
        PS = set_bit(PS, ZF);
    }
    if (check_bit(result, 7)) {
        PS = set_bit(PS, NF);
    }

    PS = clear_bit(PS, CF);
    if (result > 255) {
        PS = set_bit(PS, CF);
    }

    A = (unsigned char)(result & 0xFF);
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
    cpu_write(first, second, addr_mode, A);
}

void STX(unsigned char first, unsigned char second, unsigned char addr_mode) {
    cpu_write(first, second, addr_mode, X);
}

void STY(unsigned char first, unsigned char second, unsigned char addr_mode) {
    cpu_write(first, second, addr_mode, Y);
}

void TAX(unsigned char first, unsigned char second, unsigned char addr_mode) {
    PS = clear_bit(PS, ZF);
    PS = clear_bit(PS, NF);

    X = A;

    if (0 == X) {
        PS = set_bit(PS, ZF);
    }
    if (check_bit(X, 7)) {
        PS = set_bit(PS, NF);
    }
}

void TAY(unsigned char first, unsigned char second, unsigned char addr_mode) {
    PS = clear_bit(PS, ZF);
    PS = clear_bit(PS, NF);

    Y = A;

    if (0 == Y) {
        PS = set_bit(PS, ZF);
    }
    if (check_bit(Y, 7)) {
        PS = set_bit(PS, NF);
    }
}

void TSX(unsigned char first, unsigned char second, unsigned char addr_mode) {
    PS = clear_bit(PS, ZF);
    PS = clear_bit(PS, NF);

    X = SP;

    if (0 == X) {
        PS = set_bit(PS, ZF);
    }
    if (check_bit(X, 7)) {
        PS = set_bit(PS, NF);
    }
}

void TXA(unsigned char first, unsigned char second, unsigned char addr_mode) {
    PS = clear_bit(PS, ZF);
    PS = clear_bit(PS, NF);

    A = X;

    if (0 == A) {
        PS = set_bit(PS, ZF);
    }
    if (check_bit(A, 7)) {
        PS = set_bit(PS, NF);
    }
}

void TXS(unsigned char first, unsigned char second, unsigned char addr_mode) {
    SP = X;
}

void TYA(unsigned char first, unsigned char second, unsigned char addr_mode) {
    PS = clear_bit(PS, ZF);
    PS = clear_bit(PS, NF);

    A = Y;

    if (0 == A) {
        PS = set_bit(PS, ZF);
    }
    if (check_bit(A, 7)) {
        PS = set_bit(PS, NF);
    }
}

// UNOFFICIAL OPCODES

void ALR(unsigned char first, unsigned char second, unsigned char addr_mode) {
    AND(first, second, addr_mode);
    LSR(first, second, ACCUMULATOR);
}

void ANC(unsigned char first, unsigned char second, unsigned char addr_mode) {
    AND(first, second, addr_mode);
    if (check_bit(PS, NF)) {
        set_bit(PC, CF);
    } else {
        clear_bit(PC, CF);
    }
}

// This looks different, check documentation

void ARR(unsigned char first, unsigned char second, unsigned char addr_mode) {
    AND(first, second, addr_mode);
    int value = cpu_read(first, second, addr_mode);
    int prev_value = value;

    value >>= 1;

    if (check_bit(PS, CF) == 1) {
        value = set_bit(value, 7);
    } else {
        value = clear_bit(value, 7);
    }

    if (check_bit(prev_value, 6) == 1) {
        PS = set_bit(PS, CF);
    } else {
        PS = clear_bit(PS, CF);
    }

    if (check_bit(prev_value, 6) ^ check_bit(prev_value, 5)) {
        PS = set_bit(PS, OF);
    } else {
        PS = clear_bit(PS, OF);
    }

    if (check_bit(value, 7) == 1) {
        PS = set_bit(PS, NF);
    } else {
        PS = clear_bit(PS, NF);
    }

    if (0 == value) {
        PS = set_bit(PS, ZF);
    } else {
        PS = clear_bit(PS, ZF);
    }

    cpu_write(first, second, addr_mode, value);
}

void AXA(unsigned char first, unsigned char second, unsigned char addr_mode) {
    unsigned char value = A & X;

    cpu_write(first, second, addr_mode, value);
}

void AXS(unsigned char first, unsigned char second, unsigned char addr_mode) {
    PS = clear_bit(PS, ZF);
    PS = clear_bit(PS, NF);

    int result = A & X;

    X = result - (cpu_read(first, second, addr_mode) & 0xFF);

    if (0 == result) {
        PS = set_bit(PS, ZF);
    }

    if (check_bit(result, 7)) {
        PS = set_bit(PS, NF);
    }
}

void DCP(unsigned char first, unsigned char second, unsigned char addr_mode) {
    DEC(first, second, addr_mode);
    CMP(first, second, addr_mode);
}

void ISC(unsigned char first, unsigned char second, unsigned char addr_mode) {
    INC(first, second, addr_mode);
    SBC(first, second, addr_mode);
}

void KIL(unsigned char first, unsigned char second, unsigned char addr_mode) {
    printf("***KILL SIGNAL RECEIVED***\n");
    exit(0);
}

void LAS(unsigned char first, unsigned char second, unsigned char addr_mode) {
    PS = clear_bit(PS, NF);
    PS = clear_bit(PS, ZF);

    unsigned char value = SP & cpu_read(first, second, addr_mode);

    A = value;
    X = value;
    SP = value;

    if (0 == value) {
        PS = set_bit(PS, ZF);
    }
    if (check_bit(value, 7)) {
        PS = set_bit(PS, NF);
    }
}

void LAX(unsigned char first, unsigned char second, unsigned char addr_mode) {
    LDA(first, second, addr_mode);
    TAX(first, second, addr_mode);
}

void OAL(unsigned char first, unsigned char second, unsigned char addr_mode) {
    int a = 0;
}

void RLA(unsigned char first, unsigned char second, unsigned char addr_mode) {
    ROL(first, second, addr_mode);
    AND(first, second, addr_mode);
}

void RRA(unsigned char first, unsigned char second, unsigned char addr_mode) {
    ROR(first, second, addr_mode);
    ADC(first, second, addr_mode);
}

void SAX(unsigned char first, unsigned char second, unsigned char addr_mode) {
    cpu_write(first, second, addr_mode, A & X);
}

void SLO(unsigned char first, unsigned char second, unsigned char addr_mode) {
    ASL(first, second, addr_mode);
    ORA(first, second, addr_mode);
}

void SRE(unsigned char first, unsigned char second, unsigned char addr_mode) {
    LSR(first, second, addr_mode);
    EOR(first, second, addr_mode);
}

void SHX(unsigned char first, unsigned char second, unsigned char addr_mode) {
    uint16_t address = get_address_from_params(first, second, addr_mode);
    unsigned char value = ((address >> 8) & 0xFF) + 1;

    unsigned char result = value & X;

    cpu_write(first, second, addr_mode, value);
}

void SHY(unsigned char first, unsigned char second, unsigned char addr_mode) {
    uint16_t address = get_address_from_params(first, second, addr_mode);
    unsigned char value = ((address >> 8) & 0xFF) + 1;

    unsigned char result = value & X;

    cpu_write(first, second, addr_mode, value);
}

void TAS(unsigned char first, unsigned char second, unsigned char addr_mode) {
    uint16_t address = get_address_from_params(first, second, addr_mode);
    unsigned char value = ((address >> 8) & 0xFF) + 1;

    uint16_t result = A & X;
    SP = result & value;
}

void XAA(unsigned char first, unsigned char second, unsigned char addr_mode) {
    TXA(first, second, addr_mode);
    AND(first, second, addr_mode);
}

// INTERRUPTS

void NMI() {
    push_PC();
    stack_push(PS);

    PS = set_bit(PS, B5);
    PS = clear_bit(PS, B4);
    // PS = set_bit(PS, ID);

    PC = (RAM[0xFFFB] << 8) | RAM[0xFFFA];
}

void IRQ() {
    // TODO: Keep track of the generated IRQ(s?) and service them
    if ((check_bit(PS, ID) == 0) && (interrupt_occurred == IRQ_INT)) {
        push_PC();
        stack_push(PS);

        PS = set_bit(PS, B5);
        PS = set_bit(PS, ID);

        PC = (RAM[0xFFFF] << 8) | RAM[0xFFFE];
    }
}

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

void init_cpu() {
    SP = 0xFD;
    A = 0;
    X = 0;
    Y = 0;
    PC = (RAM[0xFFFD] << 8) | RAM[0xFFFC];
}

int cpu_clock() {
    unsigned char opcode = 0;
    unsigned char value = 0;
    unsigned char first;
    unsigned char second;
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
        return 0;
    }
    first = (RAM[PC + 1]);
    second = (RAM[PC + 2]);

    unsigned char fn_name[4];
    memset(fn_name, 0, 4);
    strncpy(fn_name, addressing[opcode].name, 3);

    // log_to_screen(opcode, first, second, fn_name);

    void (*fun_ptr)(unsigned char, unsigned char, unsigned char) = addressing[opcode].opcode_fun;
    (*fun_ptr)(first, second, addressing[opcode].addr_mode);

    if ((is_jump(fn_name) == 0) && (PC > 0))
    {
        PC += addressing[opcode].bytes;
    }

    return addressing[opcode].cycles;
}