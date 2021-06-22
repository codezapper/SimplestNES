exe:
	gcc -g -c utils.c -o utils.o
	gcc -g -c cpu.c -o cpu.o
	gcc -g -c bus.c -o bus.o
	gcc -g -c rom.c -o rom.o
	gcc -g -c main.c -o main.o
	gcc -g main.o cpu.o rom.o utils.o bus.o -o yanes

