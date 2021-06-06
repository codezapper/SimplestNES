struct HEADER {
    char nes[4];
    char prg_size;
    char chr_size;
    char flags_6;
    char flags_7;
    char flags_8;
    char flags_9;
    char flags_10;
    char padding[5];
};

struct ROM {
    struct HEADER header;
    char trainer[512];
    char prg_data[16384 * 10];
    char chr_data[8192 * 10];
    char instrom[8192];
    char prom[32];
};

char *load_rom(char *filename);
