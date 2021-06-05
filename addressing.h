#pragma once

#include "cpu.h"
#include "addressing_modes.h"

// A negative cycles count is used to determine if it needs to check for page crossing and increase by one
// (absolute value is used for computing cycles)

struct addressing_data addressing[(0xFF) + 1] = {
    {&BRK, IMPLICIT, 7}, {&ORA, INDIRECTX, 6}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {&ORA, ZEROPAGE, 5}, {&ASL, ZEROPAGE, 5}, {0, 0, 0}, {&PHP, IMPLICIT, 3}, {&ORA, IMMEDIATE, 2},
    {&ASL, ACCUMULATOR, 2}, {0, 0, 0}, {0, 0, 0}, {&ORA, ABSOLUTE, 4}, {&ASL, ABSOLUTE, 6}, {0, 0, 0}, {&BPL, RELATIVE, 2}, {&ORA, INDIRECTY, -5}, {0, 0, 0}, {0, 0, 0},
    {0, 0, 0}, {&ORA, ZEROPAGEX, 4}, {&ASL, ZEROPAGEX, 6}, {0, 0, 0}, {&CLC, IMPLICIT, 2}, {&ORA, ABSOLUTEY, -4}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {&ORA, ABSOLUTEX, -4},
    {&ASL, ABSOLUTEX, 7}, {0, 0, 0}, {&JSR, ABSOLUTE, 6}, {&AND, INDIRECTX, 6}, {0, 0, 0}, {0, 0, 0}, {&BIT, ZEROPAGE, 3}, {&AND, ZEROPAGE, 3}, {&ROL, ZEROPAGE, 5}, {0, 0, 0},
    {&PLP, IMPLICIT, 4}, {&AND, IMMEDIATE, 2}, {&ROL, ACCUMULATOR, 2}, {0, 0, 0}, {&BIT, ABSOLUTE, 4}, {&AND, ABSOLUTE, 4}, {&ROL, ABSOLUTE, 6}, {0, 0, 0}, {&BMI, RELATIVE, 2}, {&AND, INDIRECTY, -5},
    // 50
    {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {&AND, ZEROPAGEX, 4}, {&ROL, ZEROPAGEX, 6}, {0, 0, 0}, {&SEC, IMPLICIT, 2}, {&AND, ABSOLUTEY, -4}, {0, 0, 0}, {0, 0, 0},
    {0, 0, 0}, {&AND, ABSOLUTEX, -4}, {&ROL, ABSOLUTEX, 7}, {0, 0, 0}, {&RTI, IMPLICIT, 6}, {&EOR, INDIRECTX, 6}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {&EOR, ZEROPAGE, 3},
    {&LSR, ZEROPAGE, 5}, {0, 0, 0}, {&PHA, IMPLICIT, 3}, {&EOR, IMMEDIATE, 2}, {&LSR, ACCUMULATOR, 2}, {0, 0, 0}, {&JMP, ABSOLUTE, 3}, {&EOR, ABSOLUTE, 4}, {&LSR, ABSOLUTE, 6}, {0, 0, 0},
    {&BVC, RELATIVE, 2}, {&EOR, INDIRECTY, -5}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {&EOR, ZEROPAGEX, 4}, {&LSR, ZEROPAGEX, 6}, {0, 0, 0}, {CLI, IMPLICIT, 2}, {&EOR, ABSOLUTEY, -4},
    {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {&EOR, ABSOLUTEX, -4}, {&LSR, ABSOLUTEX, 7}, {0, 0, 0}, {&RTS, IMPLICIT, 6}, {&ADC, INDIRECTX, 6}, {0, 0, 0}, {0, 0, 0},
    // 100
    {0, 0, 0}, {&ADC, ZEROPAGE, 3}, {&ROR, ZEROPAGE, 5}, {0, 0, 0}, {&PLA, IMPLICIT, 4}, {&ADC, IMMEDIATE, 2}, {&ROR, ACCUMULATOR, 2}, {0, 0, 0}, {&JMP, INDIRECT, 5}, {&ADC, ABSOLUTE, 4},
    {&ROR, ABSOLUTE, 6}, {0, 0, 0}, {&BVS, RELATIVE, 2}, {&ADC, INDIRECTY, -5}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {&ADC, ZEROPAGEX, 4}, {&ROR, ZEROPAGEX, 6}, {0, 0, 0},
    {&SEI, IMPLICIT, 2}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {&ADC, ABSOLUTEX, -4}, {&ROR, ABSOLUTEX, 7}, {0, 0, 0}, {0, 0, 0}, {&STA, INDIRECTX, 6},
    {0, 0, 0}, {0, 0, 0}, {&STY, ZEROPAGE, 3}, {&STA, ZEROPAGE, 3}, {&STX, ZEROPAGE, 3}, {0, 0, 0}, {&DEY, IMPLICIT, 2}, {0, 0, 0}, {&TXA, IMPLICIT, 2}, {0, 0, 0},
    {&STY, ABSOLUTE, 4}, {&STA, ABSOLUTE, 4}, {&STX, ABSOLUTE, 4}, {0, 0, 0}, {&BCC, RELATIVE, 2}, {&STA, INDIRECTY, 6}, {0, 0, 0}, {0, 0, 0}, {&STY, ZEROPAGEX, 4}, {&STA, ZEROPAGEX, 4},
    // 150
    {&STX, ZEROPAGEY, 4}, {0, 0, 0}, {&TYA, IMPLICIT, 2}, {&STA, ABSOLUTEY, 5}, {&TXS, IMPLICIT, 2}, {0, 0, 0}, {0, 0, 0}, {&STA, ABSOLUTEX, 5}, {0, 0, 0}, {0, 0, 0},
    {&LDY, IMMEDIATE, 2}, {&LDA, INDIRECTX, 6}, {&LDX, IMMEDIATE, 2}, {0, 0, 0}, {&LDY, ZEROPAGE, 3}, {&LDA, ZEROPAGE, 3}, {&LDX, ZEROPAGE, 3}, {0, 0, 0}, {&TAY, IMPLICIT, 2}, {&LDA, IMMEDIATE, 2},
    {&TAX, IMPLICIT, 2}, {&ADC, ABSOLUTEY, -4}, {&LDY, ABSOLUTE, 4}, {&LDA, ABSOLUTE, 4}, {&LDX, ABSOLUTE, 4}, {0, 0, 0}, {&BCS, RELATIVE, 2}, {&LDA, INDIRECTY, -5}, {0, 0, 0}, {0, 0, 0},
    {&LDY, ZEROPAGEX, 4}, {&LDA, ZEROPAGEX, 4}, {&LDX, ZEROPAGEY, 4}, {0, 0, 0}, {&CLV, IMPLICIT, 2}, {&LDA, ABSOLUTEY, -4}, {&TSX, IMPLICIT, 2}, {0, 0, 0}, {&LDY, ABSOLUTEX, -4}, {&LDA, ABSOLUTEX, -4},
    {&LDX, ABSOLUTEY, -4}, {0, 0, 0}, {&CPY, IMMEDIATE, 2}, {&CMP, INDIRECTX, 6}, {0, 0, 0}, {0, 0, 0}, {&CPY, ZEROPAGE, 3}, {&CMP, ZEROPAGE, 3}, {&DEC, ZEROPAGE, 5}, {0, 0, 0},
    // 200
    {&INY, IMPLICIT, 2}, {&CMP, IMMEDIATE, 2}, {&DEX, IMPLICIT, 2}, {0, 0, 0}, {&CPY, ABSOLUTE, 4}, {&CMP, ABSOLUTE, 4}, {&DEC, ABSOLUTE, 6}, {0, 0, 0}, {&BNE, RELATIVE, 2}, {&CMP, INDIRECTY, -5},
    {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {&CMP, ZEROPAGEX, 4}, {&DEC, ZEROPAGEX, 6}, {0, 0, 0}, {&CLD, IMPLICIT, 2}, {&CMP, ABSOLUTEY, -4}, {0, 0, 0}, {0, 0, 0},
    {0, 0, 0}, {&CMP, ABSOLUTEX, -4}, {&DEC, ABSOLUTEX, 7}, {0, 0, 0}, {&CPX, IMMEDIATE, 2}, {&SBC, INDIRECTX, 6}, {0, 0, 0}, {0, 0, 0}, {&CPX, ZEROPAGE, 3}, {&SBC, ZEROPAGE, 3},
    {&INC, ZEROPAGE, 5}, {0, 0, 0}, {&INX, IMPLICIT, 2}, {&SBC, IMMEDIATE, 2}, {&NOP, IMPLICIT, 2}, {0, 0, 0}, {&CMP, ABSOLUTE, 4}, {&SBC, ABSOLUTE, 4}, {&INC, ABSOLUTE, 6}, {0, 0, 0},
    {&BEQ, RELATIVE, 2}, {&SBC, INDIRECTY, -5}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {&SBC, ZEROPAGEX, 4}, {&INC, ZEROPAGEX, 6}, {0, 0, 0}, {&SED, IMPLICIT, 2}, {&SBC, ABSOLUTEY, -4},
    // 250
    {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {&SBC, ABSOLUTEX, -4}, {&INC, ABSOLUTEX, 7}, {0, 0, 0}
};
