exe:
	gcc -g -c cpu.c -o cpu.o
	gcc -g -c main.c -o main.o
	gcc -g main.o cpu.o -o yanes

