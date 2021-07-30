#include <stdint.h>
#include "addressing_modes.h"

extern unsigned char RAM[0xFFFF];
extern uint8_t A;
extern uint8_t X;
extern uint8_t Y;
extern uint8_t PS;
extern unsigned char SP;
extern uint16_t PC;

uint16_t get_address_from_params(unsigned char first, unsigned char second, unsigned char addr_mode);

unsigned char cpu_read(unsigned char first, unsigned char second, unsigned char addr_mode);
void cpu_write(unsigned char first, unsigned char second, unsigned char addr_mode, unsigned char value);

unsigned char ppu_read(uint16_t address);
void ppu_write(uint16_t address, unsigned char value);
