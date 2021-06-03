#define IMPLICIT        0
#define ACCUMULATOR     1
#define IMMEDIATE       2
#define ZEROPAGE        3
#define ZEROPAGEX       4
#define ZEROPAGEY       5
#define RELATIVE        6
#define ABSOLUTE        7
#define ABSOLUTEX       8
#define ABSOLUTEY       9
#define INDEXED         10
#define INDIRECT        11
#define INDIRECTX       12
#define INDIRECTY       13

#define INVALID_IMMEDIATE -32768

struct addressing_data {
    void *opcode_fun;
    int addr_mode;
    int cycles;
};
