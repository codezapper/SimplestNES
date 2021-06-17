#pragma once

#include "cpu.h"
#include "addressing_modes.h"

// A negative cycles count is used to determine if it needs to check for page crossing and increase by one
// (absolute value is used for computing cycles)

struct addressing_data addressing[(0xFF) + 1] = {
    {&BRK, "BRK", IMPLICIT, 1, 7}, {&ORA, "ORA", INDIRECTX, 2, 6}, {0, "   ", 0, 0, 0}, {0, "   ", 0, 0, 0}, {&NOP, "NOP", ABSOLUTEX, 2, 1}, {&ORA, "ORA", ZEROPAGE, 2, 5}, {&ASL, "ASL", ZEROPAGE, 2, 5}, {0, "   ", 0, 0, 0}, {&PHP, "PHP", IMPLICIT, 1, 3}, {&ORA, "ORA", IMMEDIATE, 2, 2},
    {&ASL, "ASL", ACCUMULATOR, 1, 2}, {0, "   ", 0, 0, 0}, {&NOP, "NOP", INDIRECT, 3, 1}, {&ORA, "ORA", ABSOLUTE, 3, 4}, {&ASL, "ASL", ABSOLUTE, 3, 6}, {0, "   ", 0, 0, 0}, {&BPL, "BPL", RELATIVE, 2, 2}, {&ORA, "ORA", INDIRECTY, 2, -5}, {0, "   ", 0, 0, 0}, {0, "   ", 0, 0, 0},
    {&NOP, "NOP", ABSOLUTEX, 2, 1}, {&ORA, "ORA", ZEROPAGEX, 2, 4}, {&ASL, "ASL", ZEROPAGEX, 2, 6}, {0, "   ", 0, 0, 0}, {&CLC, "CLC", IMPLICIT, 1, 2}, {&ORA, "ORA", ABSOLUTEY, 3, -4}, {&NOP, "NOP", IMPLICIT, 1, 1}, {0, "   ", 0, 0, 0}, {&NOP, "NOP", INDIRECT, 3, 1}, {&ORA, "ORA", ABSOLUTEX, 3, -4},
    {&ASL, "ASL", ABSOLUTEX, 3, 7}, {0, "   ", 0, 0, 0}, {&JSR, "JSR", ABSOLUTE, 3, 6}, {&AND, "AND", INDIRECTX, 2, 6}, {0, "   ", 0, 0, 0}, {0, "   ", 0, 0, 0}, {&BIT, "BIT", ZEROPAGE, 2, 3}, {&AND, "AND", ZEROPAGE, 2, 3}, {&ROL, "ROL", ZEROPAGE, 2, 5}, {0, "   ", 0, 0, 0},
    {&PLP, "PLP", IMPLICIT, 1, 4}, {&AND, "AND", IMMEDIATE, 2, 2}, {&ROL, "ROL", ACCUMULATOR, 1, 2}, {0, "   ", 0, 0, 0}, {&BIT, "BIT", ABSOLUTE, 3, 4}, {&AND, "AND", ABSOLUTE, 3, 4}, {&ROL, "ROL", ABSOLUTE, 3, 6}, {0, "   ", 0, 0, 0}, {&BMI, "BMI", RELATIVE, 2, 2}, {&AND, "AND", INDIRECTY, 2, -5},
    // 50
    {0, "   ", 0, 0, 0}, {0, "   ", 0, 0, 0}, {&NOP, "NOP", ABSOLUTEX, 2, 1}, {&AND, "AND", ZEROPAGEX, 2, 4}, {&ROL, "ROL", ZEROPAGEX, 2, 6}, {0, 0, 0, 0}, {&SEC, "SEC", IMPLICIT, 1, 2}, {&AND, "AND", ABSOLUTEY, 3, -4}, {&NOP, "NOP", IMPLICIT, 1, 1}, {0, 0, 0, 0},
    {&NOP, "NOP", INDIRECT, 3, 1}, {&AND, "AND", ABSOLUTEX, 3, -4}, {&ROL, "ROL", ABSOLUTEX, 3, 7}, {0, 0, 0, 0}, {&RTI, "RTI", IMPLICIT, 1, 6}, {&EOR, "EOR", INDIRECTX, 2, 6}, {0, 0, 0, 0}, {0, 0, 0, 0}, {&NOP, "NOP", ABSOLUTEX, 2, 1}, {&EOR, "EOR", ZEROPAGE, 2, 3},
    {&LSR, "LSR", ZEROPAGE, 2, 5}, {0, 0, 0, 0}, {&PHA, "PHA", IMPLICIT, 1, 3}, {&EOR, "EOR", IMMEDIATE, 2, 2}, {&LSR, "LSR", ACCUMULATOR, 1, 2}, {0, 0, 0, 0}, {&JMP, "JMP", ABSOLUTE, 3, 3}, {&EOR, "EOR", ABSOLUTE, 3, 4}, {&LSR, "LSR", ABSOLUTE, 3, 6}, {0, 0, 0, 0},
    {&BVC, "BVC", RELATIVE, 2, 2}, {&EOR, "EOR", INDIRECTY, 2, -5}, {0, 0, 0, 0}, {0, 0, 0, 0}, {&NOP, "NOP", ABSOLUTEX, 2, 1}, {&EOR, "EOR", ZEROPAGEX, 2, 4}, {&LSR, "LSR", ZEROPAGEX, 2, 6}, {0, 0, 0, 0}, {CLI, IMPLICIT, 1, 2}, {&EOR, "EOR", ABSOLUTEY, 3, -4},
    {&NOP, "NOP", IMPLICIT, 1, 1}, {0, "   ", 0, 0, 0}, {&NOP, "NOP", INDIRECT, 3, 1}, {&EOR, "EOR", ABSOLUTEX, 3, -4}, {&LSR, "LSR", ABSOLUTEX, 3, 7}, {0, "   ", 0, 0, 0}, {&RTS, "RTS", IMPLICIT, 1, 6}, {&ADC, "ADC", INDIRECTX, 2, 6}, {0, "   ", 0, 0, 0}, {0, "   ", 0, 0, 0},
    // 100
    {&NOP, "NOP", ABSOLUTEX, 2, 1}, {&ADC, "ADC", ZEROPAGE, 2, 3}, {&ROR, "ROR", ZEROPAGE, 2, 5}, {0, "   ", 0, 0, 0}, {&PLA, "PLA", IMPLICIT, 1, 4}, {&ADC, "ADC", IMMEDIATE, 2, 2}, {&ROR, "ROR", ACCUMULATOR, 1, 2}, {0, "   ", 0, 0, 0}, {&JMP, "JMP", INDIRECT, 3, 5}, {&ADC, "ADC", ABSOLUTE, 3, 4},
    {&ROR, "ROR", ABSOLUTE, 3, 6}, {0, "   ", 0, 0, 0}, {&BVS, "BVS", RELATIVE, 2, 2}, {&ADC, "ADC", INDIRECTY, 2, -5}, {0, "   ", 0, 0, 0}, {0, "   ", 0, 0, 0}, {&NOP, "NOP", ABSOLUTEX, 2, 1}, {&ADC, "ADC", ZEROPAGEX, 2, 4}, {&ROR, "ROR", ZEROPAGEX, 2, 6}, {0, "   ", 0, 0, 0},
    {&SEI, "SEI", IMPLICIT, 1, 2}, {&ADC, "ADC", ABSOLUTEY, 3, -4}, {&NOP, "NOP", IMPLICIT, 1, 1}, {0, "   ", 0, 0, 0}, {&NOP, "NOP", INDIRECT, 3, 1}, {&ADC, "ADC", ABSOLUTEX, 3, -4}, {&ROR, "ROR", ABSOLUTEX, 3, 7}, {0, "   ", 0, 0, 0}, {&NOP, "NOP", ZEROPAGEX, 2, 1}, {&STA, "STA", INDIRECTX, 2, 6},
    {0, "   ", 0, 0, 0}, {0, "   ", 0, 0, 0}, {&STY, "STY", ZEROPAGE, 2, 3}, {&STA, "STA", ZEROPAGE, 2, 3}, {&STX, "STX", ZEROPAGE, 2, 3}, {0, "   ", 0, 0, 0}, {&DEY, "DEY", IMPLICIT, 1, 2}, {0, "   ", 0, 0, 0}, {&TXA, "TXA", IMPLICIT, 1, 2}, {0, "   ", 0, 0, 0},
    {&STY, "STY", ABSOLUTE, 3, 4}, {&STA, "STA", ABSOLUTE, 3, 4}, {&STX, "STX", ABSOLUTE, 3, 4}, {0, "   ", 0, 0, 0}, {&BCC, "BCC", RELATIVE, 2, 2}, {&STA, "STA", INDIRECTY, 2, 6}, {0, "   ", 0, 0, 0}, {0, "   ", 0, 0, 0}, {&STY, "STY", ZEROPAGEX, 2, 4}, {&STA, "STA", ZEROPAGEX, 2, 4},
    // 150
    {&STX, "STX", ZEROPAGEY, 2, 4}, {0, "   ", 0, 0, 0}, {&TYA, "TYA", IMPLICIT, 1, 2}, {&STA, "STA", ABSOLUTEY, 3, 5}, {&TXS, "TXS", IMPLICIT, 1, 2}, {0, "   ", 0, 0, 0}, {0, "   ", 0, 0, 0}, {&STA, "STA", ABSOLUTEX, 3, 5}, {0, "   ", 0, 0, 0}, {0, "   ", 0, 0, 0},
    {&LDY, "LDY", IMMEDIATE, 2, 2}, {&LDA, "LDA", INDIRECTX, 2, 6}, {&LDX, "LDX", IMMEDIATE, 2, 2}, {0, "   ", 0, 0, 0}, {&LDY, "LDY", ZEROPAGE, 2, 3}, {&LDA, "LDA", ZEROPAGE, 2, 3}, {&LDX, "LDX", ZEROPAGE, 2, 3}, {0, "   ", 0, 0, 0}, {&TAY, "TAY", IMPLICIT, 1, 2}, {&LDA, "LDA", IMMEDIATE, 2, 2},
    {&TAX, "TAX", IMPLICIT, 1, 2}, {0, "   ", 0, 0, 0}, {&LDY, "LDY", ABSOLUTE, 3, 4}, {&LDA, "LDA", ABSOLUTE, 3, 4}, {&LDX, "LDX", ABSOLUTE, 3, 4}, {0, "   ", 0, 0, 0}, {&BCS, "BCS", RELATIVE, 2, 2}, {&LDA, "LDA", INDIRECTY, 2, -5}, {0, "   ", 0, 0, 0}, {0, "   ", 0, 0, 0},
    {&LDY, "LDY", ZEROPAGEX, 2, 4}, {&LDA, "LDA", ZEROPAGEX, 2, 4}, {&LDX, "LDX", ZEROPAGEY, 2, 4}, {0, "   ", 0, 0, 0}, {&CLV, "CLV", IMPLICIT, 1, 2}, {&LDA, "LDA", ABSOLUTEY, 3, -4}, {&TSX, "TSX", IMPLICIT, 1, 2}, {0, "   ", 0, 0, 0}, {&LDY, "LDY", ABSOLUTEX, 3, -4}, {&LDA, "LDA", ABSOLUTEX, 3, -4},
    {&LDX, "LDX", ABSOLUTEY, 3, -4}, {0, "   ", 0, 0, 0}, {&CPY, "CPY", IMMEDIATE, 2, 2}, {&CMP, "CMP", INDIRECTX, 2, 6}, {0, "   ", 0, 0, 0}, {0, "   ", 0, 0, 0}, {&CPY, "CPY", ZEROPAGE, 2, 3}, {&CMP, "CMP", ZEROPAGE, 2, 3}, {&DEC, "DEC", ZEROPAGE, 2, 5}, {0, "   ", 0, 0, 0},
    // 200
    {&INY, "INY", IMPLICIT, 1, 2}, {&CMP, "CMP", IMMEDIATE, 2, 2}, {&DEX, "DEX", IMPLICIT, 1, 2}, {0, "   ", 0, 0, 0}, {&CPY, "CPY", ABSOLUTE, 3, 4}, {&CMP, "CMP", ABSOLUTE, 3, 4}, {&DEC, "DEC", ABSOLUTE, 3, 6}, {0, "   ", 0, 0, 0}, {&BNE, "BNE", RELATIVE, 2, 2}, {&CMP, "CMP", INDIRECTY, 2, -5},
    {0, "   ", 0, 0, 0}, {0, "   ", 0, 0, 0}, {&NOP, "NOP", ABSOLUTEX, 2, 1}, {&CMP, "CMP", ZEROPAGEX, 2, 4}, {&DEC, "DEC", ZEROPAGEX, 2, 6}, {0, "   ", 0, 0, 0}, {&CLD, "CLD", IMPLICIT, 1, 2}, {&CMP, "CMP", ABSOLUTEY, 3, -4}, {&NOP, "NOP", IMPLICIT, 1, 1}, {0, "   ", 0, 0, 0},
    {&NOP, "NOP", INDIRECT, 3, 1}, {&CMP, "CMP", ABSOLUTEX, 3, -4}, {&DEC, "DEC", ABSOLUTEX, 3, 7}, {0, "   ", 0, 0, 0}, {&CPX, "CPX", IMMEDIATE, 2, 2}, {&SBC, "SBC", INDIRECTX, 2, 6}, {0, "   ", 0, 0, 0}, {0, "   ", 0, 0, 0}, {&CPX, "CPX", ZEROPAGE, 2, 3}, {&SBC, "SBC", ZEROPAGE, 2, 3},
    {&INC, "INC", ZEROPAGE, 2, 5}, {0, "   ", 0, 0, 0}, {&INX, "INX", IMPLICIT, 1, 2}, {&SBC, "SBC", IMMEDIATE, 2, 2}, {&NOP, "NOP", IMPLICIT, 1, 2}, {0, "   ", 0, 0, 0}, {&CPX, "CPX", ABSOLUTE, 3, 4}, {&SBC, "SBC", ABSOLUTE, 3, 4}, {&INC, "INC", ABSOLUTE, 3, 6}, {0, "   ", 0, 0, 0},
    {&BEQ, "BEQ", RELATIVE, 2, 2}, {&SBC, "SBC", INDIRECTY, 2, -5}, {0, "   ", 0, 0, 0}, {0, "   ", 0, 0, 0}, {&NOP, "NOP", ABSOLUTEX, 2, 1}, {&SBC, "SBC", ZEROPAGEX, 2, 4}, {&INC, "INC", ZEROPAGEX, 2, 6}, {0, "   ", 0, 0, 0}, {&SED, "SED", IMPLICIT, 1, 2}, {&SBC, "SBC", ABSOLUTEY, 3, -4},
    // 250
    {&NOP, "NOP", IMPLICIT, 1, 1}, {0, "   ", 0, 0, 0}, {&NOP, "NOP", INDIRECT, 3, 1}, {&SBC, "SBC", ABSOLUTEX, 3, -4}, {&INC, "INC", ABSOLUTEX, 3, 7}, {0, "   ", 0, 0, 0}
};
