void init_ram();
char *get_pointer_to_ram(int16_t opcode, int16_t first, int16_t second);
void LDA(char *address);
void ADC(char *address);
void AND(char *address);
void ASL(char *address);
void BCC(char *address);
void BCS(char *address);
void BEQ(char *address);
void BIT(char *address);
void BMI(char *address);
void BNE(char *address);
void BPL(char *address);
void BRK(char *address);
void BVC(char *address);
void BVS(char *address);
void CLC(char *address);
void CLD(char *address);
void CLI(char *address);
void CLV(char *address);
