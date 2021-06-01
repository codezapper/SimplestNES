#pragma once

#include "cpu.h"
#include "addressing_modes.h"

struct addressing_data addressing[0xFF] = {
    {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
    {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
    {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
    {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
    {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
    // 50
    {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
    {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
    {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
    {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
    {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
    // 100
    {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
    {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
    {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
    {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
    {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
    // 150
    {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
    {0, 0, 0}, {&LDA, INDIRECTX, 6}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {&LDA, ZEROPAGE, 3}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {&LDA, IMMEDIATE, 2},
    {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {&LDA, ABSOLUTE, 4}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {&LDA, INDIRECTY, 5}, {0, 0, 0}, {0, 0, 0},
    {0, 0, 0}, {&LDA, ZEROPAGEX, 4}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {&LDA, ABSOLUTEY, 4}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {&LDA, ABSOLUTEX, 4},
    {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
    // 200
    {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
    {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
    {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
    {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
    {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
    // 250
    {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}
};

// void init_addressing() {
    // addressing[0xA9].addr_mode = IMMEDIATE;
    // addressing[0xA9].cycles = 2;
    // addressing[0xA5].addr_mode = ZEROPAGE;
    // addressing[0xA5].cycles = 3;
    // addressing[0xB5].addr_mode = ZEROPAGEX;
    // addressing[0xB5].cycles = 4;
    // addressing[0xAD].addr_mode = ABSOLUTE;
    // addressing[0xAD].cycles = 4;
    // addressing[0xBD].addr_mode = ABSOLUTEX;
    // addressing[0xBD].cycles = 4; // +1
    // addressing[0xB9].addr_mode = ABSOLUTEY;
    // addressing[0xB9].cycles = 4; // +1
    // addressing[0xA1].addr_mode = INDIRECTX;
    // addressing[0xA1].cycles = 6; // +1
    // addressing[0xB1].addr_mode = INDIRECTY;
    // addressing[0xB1].cycles = 5; // +1


    // addressing[0x29].addr_mode = IMMEDIATE;
    // addressing[0x29].cycles = 2;
    // addressing[0x25].addr_mode = ZEROPAGE;
    // addressing[0x25].cycles = 3;
    // addressing[0x35].addr_mode = ZEROPAGEX;
    // addressing[0x35].cycles = 4;
    // addressing[0x2D].addr_mode = ABSOLUTE;
    // addressing[0x2D].cycles = 4;
    // addressing[0x3D].addr_mode = ABSOLUTEX; // +1
    // addressing[0x3D].cycles = 4;
    // addressing[0x39].addr_mode = ABSOLUTEY; // +1
    // addressing[0x39].cycles = 4;
    // addressing[0x21].addr_mode = INDIRECTX; // +1
    // addressing[0x21].addr_mode = 6; // +1
    // addressing[0x31].addr_mode = INDIRECTY;
    // addressing[0x31].cycles = 5;

    // addressing[0x0A].addr_mode = ACCUMULATOR;
    // addressing[0x0A].cycles = 2;
    // addressing[0x06].addr_mode = ZEROPAGE;
    // addressing[0x06].cycles = 5;
    // addressing[0x16].addr_mode = ZEROPAGEX;
    // addressing[0x16].cycles = 6;
    // addressing[0x0E].addr_mode = ABSOLUTE;
    // addressing[0x0E].cycles = 6;
    // addressing[0x1E].addr_mode = ABSOLUTEX; // +1
    // addressing[0x1E].cycles = 7;
    
    // addressing[0x69].addr_mode = IMMEDIATE;
    // addressing[0x69].cycles = 2;
    // addressing[0x65].addr_mode = ZEROPAGE;
    // addressing[0x65].cycles = 3;
    // addressing[0x75].addr_mode = ZEROPAGEX;
    // addressing[0x75].cycles = 4;
    // addressing[0x6D].addr_mode = ABSOLUTE;
    // addressing[0x6D].cycles = 4; // +1
    // addressing[0x7D].addr_mode = ABSOLUTEX;
    // addressing[0x7D].cycles = 4; // +1
    // addressing[0x79].addr_mode = ABSOLUTEY;
    // addressing[0x79].cycles = 4; // +1
    // addressing[0x61].addr_mode = INDIRECTX;
    // addressing[0x61].cycles = 6;
    // addressing[0x71].addr_mode = INDIRECTY;
    // addressing[0x71].cycles = 5; // + 1
// }
