#include <stdio.h>
#include <stdint.h>
#include "addressing.h"

extern char RAM[0xFFFF];
extern int8_t A;

void main(int argc, char **argv) {
    init_ram();
    RAM[2] = 17;

    // ((addressing[0xA5])(get_pointer_to_ram(0xA5, 2, 20));
    // void (*fun_ptr)(char *) = addressing[0xA5].opcode_fun;
    // (*fun_ptr)(get_pointer_to_ram(0xA5, 2, 20));
    // printf("%d\n", A);
    void (*fun_ptr)(char *) = addressing[0xA9].opcode_fun;
    (*fun_ptr)(get_pointer_to_ram(0xA9, 42, 20));
    printf("%d\n", A);
}