#define TRAINER_START 0x7000
#define TRAINER_END 0x71FF

struct HEADER {
    unsigned char nes[4];
    unsigned char prg_size;
    unsigned char chr_size;
    unsigned char flags_6;
    unsigned char flags_7;
    unsigned char flags_8;
    unsigned char flags_9;
    unsigned char flags_10;
    unsigned char padding[5];
};

struct ROM {
    struct HEADER header;
    unsigned char trainer[512];
    unsigned char prg_data[16384 * 4];
    unsigned char chr_data[8192 * 10];
    unsigned char instrom[8192];
    unsigned char prom[32];
};

void load_rom(char *filename);
