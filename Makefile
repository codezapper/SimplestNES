exe:
	rm -f main.o cpu.o ppu.o rom.o utils.o bus.o yanes
	gcc -g -O0 -c utils.c -o utils.o
	gcc -g -O0 -c cpu.c -o cpu.o
	gcc -g -O0 -I/usr/include/SDL2 -c ppu.c -o ppu.o
	gcc -g -O0 -I/usr/include/SDL2 -c bus.c -o bus.o
	gcc -g -O0 -c rom.c -o rom.o
	gcc -g -O0 -I/usr/include/SDL2 -c main.c -o main.o
	gcc -g -O0 main.o cpu.o ppu.o rom.o utils.o bus.o -lSDL2 -lSDL2_ttf -o yanes

profiler:
	rm -f main.o cpu.o ppu.o rom.o utils.o bus.o yanes
	gcc -p -O3 -c utils.c -o utils.o
	gcc -p -O3 -c cpu.c -o cpu.o
	gcc -p -O3 -I/usr/include/SDL2 -c ppu.c -o ppu.o
	gcc -p -O3 -I/usr/include/SDL2 -c bus.c -o bus.o
	gcc -p -O3 -c rom.c -o rom.o
	gcc -p -O3 -I/usr/include/SDL2 -c main.c -o main.o
	gcc -p -O3 main.o cpu.o ppu.o rom.o utils.o bus.o -lSDL2 -lSDL2_ttf -o yanes

optimized:
	rm -f main.o cpu.o ppu.o rom.o utils.o bus.o yanes
	gcc -O3 -c utils.c -o utils.o
	gcc -O3 -c cpu.c -o cpu.o
	gcc -O3 -I/usr/include/SDL2 -c ppu.c -o ppu.o
	gcc -O3 -I/usr/include/SDL2 -c bus.c -o bus.o
	gcc -O3 -c rom.c -o rom.o
	gcc -O3 -I/usr/include/SDL2 -c main.c -o main.o
	gcc -O3 main.o cpu.o ppu.o rom.o utils.o bus.o -lSDL2 -lSDL2_ttf -o yanes
