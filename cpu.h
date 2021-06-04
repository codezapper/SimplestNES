void init_ram();
char *get_pointer_to_ram(int16_t opcode, int16_t first, int16_t second);
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
void CMP(char *address);
void CPX(char *address);
void CPY(char *address);
void DEC(char *address);
void DEX(char *address);
void DEY(char *address);
void EOR(char *address);
void INC(char *address);
void INX(char *address);
void INY(char *address);
void JMP(char *address);
void JSR(char *address);
void LDA(char *address);
void LDX(char *address);
void LDY(char *address);
void LSR(char *address);
void NOP(char *address);
void ORA(char *address);
void PHA(char *address);
void PHP(char *address);
void PLA(char *address);
void PLP(char *address);
