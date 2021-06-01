void init_ram();
char *get_pointer_to_ram(int16_t opcode, int16_t first, int16_t second);
void LDA(char *address);
void ADC(char *address);
void AND(char *address);
