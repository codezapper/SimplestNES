exe:
	gcc -g -c cpu.c -o cpu.o
	gcc -g -c rom.c -o rom.o
	gcc -g -c main.c -o main.o
	gcc -g main.o cpu.o rom.o -o yanes

