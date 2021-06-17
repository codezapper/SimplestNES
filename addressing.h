#pragma once

#include "cpu.h"
#include "addressing_modes.h"

// A negative cycles count is used to determine if it needs to check for page crossing and increase by one
// (absolute value is used for computing cycles)

struct addressing_data addressing[(0xFF) + 1] = {
    {&BRK, "BRK", IMPLICIT, 7}, {&ORA, "ORA", INDIRECTX, 6}, {0, "   ", 0, 0}, {0, "   ", 0, 0}, {&NOP, "NOP", IMPLICIT, 1}, {&ORA, "ORA", ZEROPAGE, 5}, {&ASL, "ASL", ZEROPAGE, 5}, {0, "   ", 0, 0}, {&PHP, "PHP", IMPLICIT, 3}, {&ORA, "ORA", IMMEDIATE, 2},
    {&ASL, "ASL", ACCUMULATOR, 2}, {0, "   ", 0, 0}, {0, "   ", 0, 0}, {&ORA, "ORA", ABSOLUTE, 4}, {&ASL, "ASL", ABSOLUTE, 6}, {0, "   ", 0, 0}, {&BPL, "BPL", RELATIVE, 2}, {&ORA, "ORA", INDIRECTY, -5}, {0, "   ", 0, 0}, {0, "   ", 0, 0},
    {0, "   ", 0, 0}, {&ORA, "ORA", ZEROPAGEX, 4}, {&ASL, "ASL", ZEROPAGEX, 6}, {0, "   ", 0, 0}, {&CLC, "CLC", IMPLICIT, 2}, {&ORA, "ORA", ABSOLUTEY, -4}, {0, "   ", 0, 0}, {0, "   ", 0, 0}, {0, "   ", 0, 0}, {&ORA, "ORA", ABSOLUTEX, -4},
    {&ASL, "ASL", ABSOLUTEX, 7}, {0, "   ", 0, 0}, {&JSR, "JSR", ABSOLUTE, 6}, {&AND, "AND", INDIRECTX, 6}, {0, "   ", 0, 0}, {0, "   ", 0, 0}, {&BIT, "BIT", ZEROPAGE, 3}, {&AND, "AND", ZEROPAGE, 3}, {&ROL, "ROL", ZEROPAGE, 5}, {0, "   ", 0, 0},
    {&PLP, "PLP", IMPLICIT, 4}, {&AND, "AND", IMMEDIATE, 2}, {&ROL, "ROL", ACCUMULATOR, 2}, {0, "   ", 0, 0}, {&BIT, "BIT", ABSOLUTE, 4}, {&AND, "AND", ABSOLUTE, 4}, {&ROL, "ROL", ABSOLUTE, 6}, {0, "   ", 0, 0}, {&BMI, "BMI", RELATIVE, 2}, {&AND, "AND", INDIRECTY, -5},
    // 50
    {0, "   ", 0, 0}, {0, "   ", 0, 0}, {0, 0, 0}, {&AND, "AND", ZEROPAGEX, 4}, {&ROL, "ROL", ZEROPAGEX, 6}, {0, 0, 0}, {&SEC, "SEC", IMPLICIT, 2}, {&AND, "AND", ABSOLUTEY, -4}, {0, 0, 0}, {0, 0, 0},
    {0, 0, 0}, {&AND, "AND", ABSOLUTEX, -4}, {&ROL, "ROL", ABSOLUTEX, 7}, {0, 0, 0}, {&RTI, "RTI", IMPLICIT, 6}, {&EOR, "EOR", INDIRECTX, 6}, {0, 0, 0}, {0, 0, 0}, {&NOP, "NOP", IMPLICIT, 1}, {&EOR, "EOR", ZEROPAGE, 3},
    {&LSR, "LSR", ZEROPAGE, 5}, {0, 0, 0}, {&PHA, "PHA", IMPLICIT, 3}, {&EOR, "EOR", IMMEDIATE, 2}, {&LSR, "LSR", ACCUMULATOR, 2}, {0, 0, 0}, {&JMP, "JMP", ABSOLUTE, 3}, {&EOR, "EOR", ABSOLUTE, 4}, {&LSR, "LSR", ABSOLUTE, 6}, {0, 0, 0},
    {&BVC, "BVC", RELATIVE, 2}, {&EOR, "EOR", INDIRECTY, -5}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {&EOR, "EOR", ZEROPAGEX, 4}, {&LSR, "LSR", ZEROPAGEX, 6}, {0, 0, 0}, {CLI, IMPLICIT, 2}, {&EOR, "EOR", ABSOLUTEY, -4},
    {0, "   ", 0, 0}, {0, "   ", 0, 0}, {0, "   ", 0, 0}, {&EOR, "EOR", ABSOLUTEX, -4}, {&LSR, "LSR", ABSOLUTEX, 7}, {0, "   ", 0, 0}, {&RTS, "RTS", IMPLICIT, 6}, {&ADC, "ADC", INDIRECTX, 6}, {0, "   ", 0, 0}, {0, "   ", 0, 0},
    // 100
    {&NOP, "NOP", IMPLICIT, 1}, {&ADC, "ADC", ZEROPAGE, 3}, {&ROR, "ROR", ZEROPAGE, 5}, {0, "   ", 0, 0}, {&PLA, "PLA", IMPLICIT, 4}, {&ADC, "ADC", IMMEDIATE, 2}, {&ROR, "ROR", ACCUMULATOR, 2}, {0, "   ", 0, 0}, {&JMP, "JMP", INDIRECT, 5}, {&ADC, "ADC", ABSOLUTE, 4},
    {&ROR, "ROR", ABSOLUTE, 6}, {0, "   ", 0, 0}, {&BVS, "BVS", RELATIVE, 2}, {&ADC, "ADC", INDIRECTY, -5}, {0, "   ", 0, 0}, {0, "   ", 0, 0}, {0, "   ", 0, 0}, {&ADC, "ADC", ZEROPAGEX, 4}, {&ROR, "ROR", ZEROPAGEX, 6}, {0, "   ", 0, 0},
    {&SEI, "SEI", IMPLICIT, 2}, {&ADC, "ADC", ABSOLUTEY, -4}, {0, "   ", 0, 0}, {0, "   ", 0, 0}, {0, "   ", 0, 0}, {&ADC, "ADC", ABSOLUTEX, -4}, {&ROR, "ROR", ABSOLUTEX, 7}, {0, "   ", 0, 0}, {0, "   ", 0, 0}, {&STA, "STA", INDIRECTX, 6},
    {0, "   ", 0, 0}, {0, "   ", 0, 0}, {&STY, "STY", ZEROPAGE, 3}, {&STA, "STA", ZEROPAGE, 3}, {&STX, "STX", ZEROPAGE, 3}, {0, "   ", 0, 0}, {&DEY, "DEY", IMPLICIT, 2}, {0, "   ", 0, 0}, {&TXA, "TXA", IMPLICIT, 2}, {0, "   ", 0, 0},
    {&STY, "STY", ABSOLUTE, 4}, {&STA, "STA", ABSOLUTE, 4}, {&STX, "STX", ABSOLUTE, 4}, {0, "   ", 0, 0}, {&BCC, "BCC", RELATIVE, 2}, {&STA, "STA", INDIRECTY, 6}, {0, "   ", 0, 0}, {0, "   ", 0, 0}, {&STY, "STY", ZEROPAGEX, 4}, {&STA, "STA", ZEROPAGEX, 4},
    // 150
    {&STX, "STX", ZEROPAGEY, 4}, {0, "   ", 0, 0}, {&TYA, "TYA", IMPLICIT, 2}, {&STA, "STA", ABSOLUTEY, 5}, {&TXS, "TXS", IMPLICIT, 2}, {0, "   ", 0, 0}, {0, "   ", 0, 0}, {&STA, "STA", ABSOLUTEX, 5}, {0, "   ", 0, 0}, {0, "   ", 0, 0},
    {&LDY, "LDY", IMMEDIATE, 2}, {&LDA, "LDA", INDIRECTX, 6}, {&LDX, "LDX", IMMEDIATE, 2}, {0, "   ", 0, 0}, {&LDY, "LDY", ZEROPAGE, 3}, {&LDA, "LDA", ZEROPAGE, 3}, {&LDX, "LDX", ZEROPAGE, 3}, {0, "   ", 0, 0}, {&TAY, "TAY", IMPLICIT, 2}, {&LDA, "LDA", IMMEDIATE, 2},
    {&TAX, "TAX", IMPLICIT, 2}, {0, "   ", 0, 0}, {&LDY, "LDY", ABSOLUTE, 4}, {&LDA, "LDA", ABSOLUTE, 4}, {&LDX, "LDX", ABSOLUTE, 4}, {0, "   ", 0, 0}, {&BCS, "BCS", RELATIVE, 2}, {&LDA, "LDA", INDIRECTY, -5}, {0, "   ", 0, 0}, {0, "   ", 0, 0},
    {&LDY, "LDY", ZEROPAGEX, 4}, {&LDA, "LDA", ZEROPAGEX, 4}, {&LDX, "LDX", ZEROPAGEY, 4}, {0, "   ", 0, 0}, {&CLV, "CLV", IMPLICIT, 2}, {&LDA, "LDA", ABSOLUTEY, -4}, {&TSX, "TSX", IMPLICIT, 2}, {0, "   ", 0, 0}, {&LDY, "LDY", ABSOLUTEX, -4}, {&LDA, "LDA", ABSOLUTEX, -4},
    {&LDX, "LDX", ABSOLUTEY, -4}, {0, "   ", 0, 0}, {&CPY, "CPY", IMMEDIATE, 2}, {&CMP, "CMP", INDIRECTX, 6}, {0, "   ", 0, 0}, {0, "   ", 0, 0}, {&CPY, "CPY", ZEROPAGE, 3}, {&CMP, "CMP", ZEROPAGE, 3}, {&DEC, "DEC", ZEROPAGE, 5}, {0, "   ", 0, 0},
    // 200
    {&INY, "INY", IMPLICIT, 2}, {&CMP, "CMP", IMMEDIATE, 2}, {&DEX, "DEX", IMPLICIT, 2}, {0, "   ", 0, 0}, {&CPY, "CPY", ABSOLUTE, 4}, {&CMP, "CMP", ABSOLUTE, 4}, {&DEC, "DEC", ABSOLUTE, 6}, {0, "   ", 0, 0}, {&BNE, "BNE", RELATIVE, 2}, {&CMP, "CMP", INDIRECTY, -5},
    {0, "   ", 0, 0}, {0, "   ", 0, 0}, {0, "   ", 0, 0}, {&CMP, "CMP", ZEROPAGEX, 4}, {&DEC, "DEC", ZEROPAGEX, 6}, {0, "   ", 0, 0}, {&CLD, "CLD", IMPLICIT, 2}, {&CMP, "CMP", ABSOLUTEY, -4}, {0, "   ", 0, 0}, {0, "   ", 0, 0},
    {0, "   ", 0, 0}, {&CMP, "CMP", ABSOLUTEX, -4}, {&DEC, "DEC", ABSOLUTEX, 7}, {0, "   ", 0, 0}, {&CPX, "CPX", IMMEDIATE, 2}, {&SBC, "SBC", INDIRECTX, 6}, {0, "   ", 0, 0}, {0, "   ", 0, 0}, {&CPX, "CPX", ZEROPAGE, 3}, {&SBC, "SBC", ZEROPAGE, 3},
    {&INC, "INC", ZEROPAGE, 5}, {0, "   ", 0, 0}, {&INX, "INX", IMPLICIT, 2}, {&SBC, "SBC", IMMEDIATE, 2}, {&NOP, "NOP", IMPLICIT, 2}, {0, "   ", 0, 0}, {&CPX, "CPX", ABSOLUTE, 4}, {&SBC, "SBC", ABSOLUTE, 4}, {&INC, "INC", ABSOLUTE, 6}, {0, "   ", 0, 0},
    {&BEQ, "BEQ", RELATIVE, 2}, {&SBC, "SBC", INDIRECTY, -5}, {0, "   ", 0, 0}, {0, "   ", 0, 0}, {0, "   ", 0, 0}, {&SBC, "SBC", ZEROPAGEX, 4}, {&INC, "INC", ZEROPAGEX, 6}, {0, "   ", 0, 0}, {&SED, "SED", IMPLICIT, 2}, {&SBC, "SBC", ABSOLUTEY, -4},
    // 250
    {0, "   ", 0, 0}, {0, "   ", 0, 0}, {0, "   ", 0, 0}, {&SBC, "SBC", ABSOLUTEX, -4}, {&INC, "INC", ABSOLUTEX, 7}, {0, "   ", 0, 0}
};
