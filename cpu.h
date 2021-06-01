void init_ram();
char *get_pointer_to_ram(int16_t opcode, int16_t first, int16_t second);
void LDA(char *address);
void ADC(char *address);
void AND(char *address);
void ASL(char *address);
void BCC(char *address);
void BCS(char *address);
void BEQ(char *address);
