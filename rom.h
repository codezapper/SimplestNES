#define TRAINER_START 0x7000
#define TRAINER_END 0x71FF

struct HEADER {
    unsigned char nes[4];
    unsigned char prg_blocks;
    unsigned char chr_blocks;
    unsigned char flags_6;
    unsigned char flags_7;
    unsigned char flags_8;
    unsigned char flags_9;
    unsigned char flags_10;
    unsigned char padding[5];
};

struct ROM {
    unsigned char header[16];
    unsigned char trainer[512];
    unsigned char prg_rom[16384 * 4];
    unsigned char chr_rom[8192 * 8];
    unsigned char instrom[8192];
    unsigned char prom[32];
};

struct HEADER20 {
    unsigned char nes[4];
    unsigned char prg_rom_lsb;
    unsigned char chr_rom_lsb;
    unsigned char flags_6;
    unsigned char flags_7;
    unsigned char mapper;
    unsigned char submapper;
    unsigned char rom_size_msb;
    unsigned char prg_ram_size;
    unsigned char chr_ram_size;
    unsigned char timing;
    unsigned char system_type;
    unsigned char misc_roms;
    unsigned char expansion_device;
};

struct ROM20 {
    unsigned char header[16];
    unsigned char trainer[512];
    unsigned char prg_rom[16384 * 4];
    unsigned char chr_rom[8192 * 8];
    unsigned char misc_rom[16384];
};

void load_rom(char *filename);
