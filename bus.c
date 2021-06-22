#include "bus.h"

uint16_t get_address_from_params(unsigned char first, unsigned char second, unsigned char addr_mode) {
    uint16_t high;
    uint16_t low;
    uint16_t address;

    switch (addr_mode) {
        case ACCUMULATOR:
            return A;
        case IMMEDIATE:
            return first;
        case RELATIVE:
            return first;
        case ZEROPAGE:
            return first % 256;
        case ZEROPAGEX:
            return (first + X) % 256;
        case ZEROPAGEY:
            return (first + Y) % 256;
        case ABSOLUTE:
            address = first;
            address = (second << 8) |address;
            return address;
        case ABSOLUTEX:
            address = first;
            address = (second << 8) |address;
            return address + X;
        case ABSOLUTEY:
            address = first;
            address = (second << 8) |address;
            return address + Y;
        case INDIRECTX:
            return (RAM[(first + X + 1) & 0xFF] << 8) | RAM[(first + X) & 0xFF];
        case INDIRECTY:
            high = RAM[(first + 1) & 0xFF] << 8;
            low = RAM[first & 0xFF];
            return (high | low) + Y;
        case INDIRECT:
            if (first == 0xFF) {
                high = (second << 8);
                low = (second << 8) | 0x00FF;
                address = (RAM[high] << 8) | RAM[low];
            } else {
                high = second;
                high <<= 8;
                low = first;
                address = (RAM[(high | low) + 1] << 8) | RAM[(high | low)];
            }

            return address;
    }
}

unsigned char cpu_read(uint16_t value, unsigned char addr_mode) {
    switch (addr_mode) {
        case ACCUMULATOR:
        case IMMEDIATE:
        case RELATIVE:
            return value;
        case ZEROPAGE:
        case ZEROPAGEX:
        case ZEROPAGEY:
        case ABSOLUTE:
        case ABSOLUTEX:
        case ABSOLUTEY:
        case INDIRECTX:
        case INDIRECTY:
        case INDIRECT:
            return RAM[value];
    }
}

void cpu_write(unsigned char first, unsigned char second, unsigned char addr_mode, unsigned char value) {
    if (ACCUMULATOR == addr_mode) {
        A = value;
    } else {
        RAM[get_address_from_params(first, second, addr_mode)] = value;
    }
}

unsigned char ppu_read(uint16_t address, unsigned char addr_mode) {
    
}

void ppu_write(unsigned char first, unsigned char second, unsigned char addr_mode) {
    
}
