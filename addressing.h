#pragma once

#include "cpu.h"
#include "addressing_modes.h"

// A negative cycles count is used to determine if it needs to check for page crossing and increase by one
// (absolute value is used for computing cycles)

struct addressing_data addressing[(0xFF) + 1] = {
    {&BRK, IMPLICIT, 7}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {&ASL, ZEROPAGE, 5}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
    {&ASL, ACCUMULATOR, 2}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {&ASL, ABSOLUTE, 6}, {0, 0, 0}, {&BPL, RELATIVE, 2}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
    {0, 0, 0}, {0, 0, 0}, {&ASL, ZEROPAGEX, 6}, {0, 0, 0}, {&CLC, IMPLICIT, 2}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
    {&ASL, ABSOLUTEX, 7}, {0, 0, 0}, {&JSR, ABSOLUTE, 6}, {&AND, INDIRECTX, 6}, {0, 0, 0}, {0, 0, 0}, {&BIT, ZEROPAGE, 3}, {&AND, ZEROPAGE, 3}, {0, 0, 0}, {0, 0, 0},
    {0, 0, 0}, {&AND, IMMEDIATE, 2}, {0, 0, 0}, {0, 0, 0}, {&BIT, ABSOLUTE, 4}, {&AND, ABSOLUTE, 4}, {0, 0, 0}, {0, 0, 0}, {&BMI, RELATIVE, 2}, {&AND, INDIRECTY, -5},
    // 50
    {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {&AND, ZEROPAGEX, 4}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {&AND, ABSOLUTEY, -4}, {0, 0, 0}, {0, 0, 0},
    {0, 0, 0}, {&AND, ABSOLUTEX, -4}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {&EOR, INDIRECTX, 6}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {&EOR, ZEROPAGE, 3},
    {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {&EOR, IMMEDIATE, 2}, {0, 0, 0}, {0, 0, 0}, {&JMP, ABSOLUTE, 3}, {&EOR, ABSOLUTE, 4}, {0, 0, 0}, {0, 0, 0},
    {&BVC, RELATIVE, 2}, {&EOR, INDIRECTY, -5}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {&EOR, ZEROPAGEX, 4}, {0, 0, 0}, {0, 0, 0}, {CLI, IMPLICIT, 2}, {&EOR, ABSOLUTEY, -4},
    {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {&EOR, ABSOLUTEX, -4}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {&ADC, INDIRECTX, 6}, {0, 0, 0}, {0, 0, 0},
    // 100
    {0, 0, 0}, {&ADC, ZEROPAGE, 3}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {&ADC, IMMEDIATE, 2}, {0, 0, 0}, {0, 0, 0}, {&JMP, INDIRECT, 5}, {&ADC, ABSOLUTE, 4},
    {0, 0, 0}, {0, 0, 0}, {&BVS, RELATIVE, 2}, {&ADC, INDIRECTY, -5}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {&ADC, ZEROPAGEX, 4}, {0, 0, 0}, {0, 0, 0},
    {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {&ADC, ABSOLUTEX, -4}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
    {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {&DEY, IMPLICIT, 2}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
    {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {&BCC, RELATIVE, 2}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
    // 150
    {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
    {&LDY, IMMEDIATE, 2}, {&LDA, INDIRECTX, 6}, {&LDX, IMMEDIATE, 2}, {0, 0, 0}, {&LDY, ZEROPAGE, 3}, {&LDA, ZEROPAGE, 3}, {&LDX, ZEROPAGE, 3}, {0, 0, 0}, {0, 0, 0}, {&LDA, IMMEDIATE, 2},
    {0, 0, 0}, {&ADC, ABSOLUTEY, -4}, {&LDY, ABSOLUTE, 4}, {&LDA, ABSOLUTE, 4}, {&LDX, ABSOLUTE, 4}, {0, 0, 0}, {&BCS, RELATIVE, 2}, {&LDA, INDIRECTY, -5}, {0, 0, 0}, {0, 0, 0},
    {&LDY, ZEROPAGEX, 4}, {&LDA, ZEROPAGEX, 4}, {&LDX, ZEROPAGEY, 4}, {0, 0, 0}, {&CLV, IMPLICIT, 2}, {&LDA, ABSOLUTEY, -4}, {0, 0, 0}, {0, 0, 0}, {&LDY, ABSOLUTEX, -4}, {&LDA, ABSOLUTEX, -4},
    {&LDX, ABSOLUTEY, -4}, {0, 0, 0}, {&CPY, IMMEDIATE, 2}, {&CMP, INDIRECTX, 6}, {0, 0, 0}, {0, 0, 0}, {&CPY, ZEROPAGE, 3}, {&CMP, ZEROPAGE, 3}, {&DEC, ZEROPAGE, 5}, {0, 0, 0},
    // 200
    {&INY, IMPLICIT, 2}, {&CMP, IMMEDIATE, 2}, {&DEX, IMPLICIT, 2}, {0, 0, 0}, {&CPY, ABSOLUTE, 4}, {&CMP, ABSOLUTE, 4}, {&DEC, ABSOLUTE, 6}, {0, 0, 0}, {&BNE, RELATIVE, 2}, {&CMP, INDIRECTY, -5},
    {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {&CMP, ZEROPAGEX, 4}, {&DEC, ZEROPAGEX, 6}, {0, 0, 0}, {&CLD, IMPLICIT, 2}, {&CMP, ABSOLUTEY, -4}, {0, 0, 0}, {0, 0, 0},
    {0, 0, 0}, {&CMP, ABSOLUTEX, -4}, {&DEC, ABSOLUTEX, 7}, {0, 0, 0}, {&CPX, IMMEDIATE, 2}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {&CPX, ZEROPAGE, 3}, {0, 0, 0},
    {&INC, ZEROPAGE, 5}, {0, 0, 0}, {&INX, IMPLICIT, 2}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {&CMP, ABSOLUTE, 4}, {0, 0, 0}, {&INC, ABSOLUTE, 6}, {0, 0, 0},
    {&BEQ, RELATIVE, 2}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {&INC, ZEROPAGEX, 6}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
    // 250
    {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {&INC, ABSOLUTEX, 7}, {0, 0, 0}
};
