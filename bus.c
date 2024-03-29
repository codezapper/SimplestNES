#include "bus.h"
#include "ppu.h"
#include <SDL_scancode.h>
#include <SDL_keyboard.h>

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

unsigned char buttons[8] = {0, 0, 0, 0, 0, 0, 0, 0};
// Order is A, B, SELECT, START, UP, DOWN, LEFT, RIGHT
// TODO: Make buttons customizable
int scancodes[8] = {SDL_SCANCODE_J, SDL_SCANCODE_K, SDL_SCANCODE_Q, SDL_SCANCODE_E, SDL_SCANCODE_W, SDL_SCANCODE_S, SDL_SCANCODE_A, SDL_SCANCODE_D};

uint8_t readController1(uint8_t bit) {
    uint8_t *SDL_keys = (uint8_t*)SDL_GetKeyboardState(0x0);
    for (int i = 0; i < 8; i++) {
        buttons[i] = SDL_keys[scancodes[i]];
    }
    return buttons[bit];
}

unsigned char poll_controller1 = -1;

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
            } else if (value == 0x4016) {
                if (poll_controller1 >= 0) {
                    unsigned char ret = readController1(poll_controller1++);
                    if (poll_controller1 > 7) {
                        poll_controller1 = -1;
                    }
                    return ret | 0x40;
                }
                return 0x40;
            } else {
                // TODO: Do not read RAM directly, but use a separate function to support mappers
                return RAM[value];
            }
    }
}


void cpu_write(unsigned char first, unsigned char second, unsigned char addr_mode, unsigned char value) {
    if (ACCUMULATOR == addr_mode) {
        A = value;
    } else {
        uint16_t address = get_address_from_params(first, second, addr_mode);
        if (((address >= 0x2000) && (address <= 0x3FFF)) || (address == 0x4014)) {
            ppu_write(address, value);
        } else if (address == 0x4016) {
            poll_controller1 = value;
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
    // If it's a PPU register, they're mirrored up to 0x3FFF
    // Maybe 0x2000-0x3FFF ?

    // When reading from an unmapped area (e.g. reading
    // from a write-only register), the last written value
    // is returned. This is called "open bus".
    open_bus = value;

    if (address == OAMDMA) {
        uint16_t cpu_address = value << 8;
        unsigned char start_addr = get_oamaddr();

        int cnt_cpu_ram = 0;
        int cnt_ppu_ram = 0;

        while (cnt_cpu_ram < 256) {
            int ppu_address = (start_addr + cnt_ppu_ram) % 256;
            write_dma(ppu_address, RAM[cpu_address + cnt_cpu_ram]);
            cnt_cpu_ram++;
            cnt_ppu_ram++;
        }
        return;
    }

    uint16_t offset = (address % 8) + 0x2000;
    // The PPU has a warm up time of 2 VBLs
    // if ((write_enabled == 0) && (offset >= 0x2003)) {
    //     return;
    // }
    switch (offset) {
        case PPUCTRL:
            write_ppuctrl(value);
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
    }
}
