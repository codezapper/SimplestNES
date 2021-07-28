#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <SDL.h>

#include "cpu.h"
#include "ppu.h"
#include "rom.h"
#include "utils.h"

extern uint16_t PC;

void main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Usage: %s filepath\n", argv[0]);
        exit(1);
    }

    init_ram();
    load_rom(argv[1]);
    init_cpu();
    init_ppu();

    while (PC > 0)
    {
        SDL_Event e;
		SDL_PollEvent(&e);
		// TODO: Exit gracefully
		if (e.type == SDL_QUIT) {
			exit(1);
		}
		if (e.type == SDL_KEYDOWN) {
			if (e.key.keysym.sym == SDLK_ESCAPE) {
				exit(1);
			}
		}
        int cpu_cycles = cpu_clock();
        ppu_clock(cpu_cycles);
    }
}
