#include "bus.h"
#include "ppu.h"

extern unsigned char VRAM[0xFFFF];
extern int cycles;

extern unsigned char write_enabled;

unsigned char open_bus;

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

unsigned char cpu_read(unsigned char first, unsigned char second, unsigned char addr_mode) {
    uint16_t value = get_address_from_params(first, second, addr_mode);

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
            if ((value >= 0) && (value <= 0x1FFF)) {
                value &= 0x07FF;
            }

            if ((value >= 0x2000) && (value <= 0x3FFF)) {
                return ppu_read(value);
            } else {
                return RAM[value];
            }
    }
}

void cpu_write(unsigned char first, unsigned char second, unsigned char addr_mode, unsigned char value) {
    if (ACCUMULATOR == addr_mode) {
        A = value;
    } else {
        uint16_t address = get_address_from_params(first, second, addr_mode);
        open_bus = value;
        // If it's a PPU register, they're mirrored up to 0x3FFF
        // Maybe 0x2000-0x3FFF ?
        if ((address >= 0x2000) && (address <= 0x3FFF)) {
            unsigned char offset = address % 8;
            if ((write_enabled == 0) && (offset >= 3)) {
                // return;
            }
            switch (offset) {
                case 0:
                    return write_ppuctrl(value);
                case 1:
                    return write_ppumask(value);
                case 3:
                    return write_oamaddr(value);
                case 5:
                    return write_ppuscroll(value);
                case 6:
                    return write_v(value);
                case 7:
                    return write_ppudata(value);
            }
        } else {
            RAM[address] = value;
        }
    }
}

unsigned char ppu_read(uint16_t address) {
    uint16_t offset = (address % 8) + 0x2000;
    switch (offset) {
        case PPUSTATUS:
            return read_ppustatus();
            break;
        case OAMDATA:
            return read_oamdata();
            break;
        case PPUDATA:
            return read_ppudata();
            break;
    }
    return open_bus;
}

void ppu_write(uint16_t address, unsigned char value) {
    switch (address) {
        case PPUCTRL:
            if (cycles >= 30000) {
                write_ppuctrl(value);
            }
            break;
        case PPUMASK:
            write_ppumask(value);
            break;
        case OAMADDR:
            write_oamaddr(value);
            break;
        case OAMDATA:
            write_oamdata(value);
            break;
        case PPUSCROLL:
            write_ppuscroll(value);
            break;
        case PPUADDR:
            write_v(value);
            break;
        case PPUDATA:
            write_ppudata(value);
            break;
        case OAMDMA:
            uint16_t cpu_address = value << 8;
            for (int i = 0; i < 256; i++) {
                write_dma(i, RAM[cpu_address+i]);
            }
            break;
    }
}
