#define NMI_INT     1
#define IRQ_INT     2

#define CF  0
#define ZF  1
#define ID  2
#define DM  3
#define B4  4
#define B5  5
#define OF  6
#define NF  7

void init_ram();
void init_cpu();
int cpu_clock();

uint16_t read_value_from_params(unsigned char first, unsigned char second, unsigned char addr_mode);
void write_value_from_params(unsigned char first, unsigned char second, unsigned char value, unsigned char addr_mode);
void ADC(unsigned char first, unsigned char second, unsigned char addr_mode);
void AND(unsigned char first, unsigned char second, unsigned char addr_mode);
void ASL(unsigned char first, unsigned char second, unsigned char addr_mode);
void BCC(unsigned char first, unsigned char second, unsigned char addr_mode);
void BCS(unsigned char first, unsigned char second, unsigned char addr_mode);
void BEQ(unsigned char first, unsigned char second, unsigned char addr_mode);
void BIT(unsigned char first, unsigned char second, unsigned char addr_mode);
void BMI(unsigned char first, unsigned char second, unsigned char addr_mode);
void BNE(unsigned char first, unsigned char second, unsigned char addr_mode);
void BPL(unsigned char first, unsigned char second, unsigned char addr_mode);
void BRK(unsigned char first, unsigned char second, unsigned char addr_mode);
void BVC(unsigned char first, unsigned char second, unsigned char addr_mode);
void BVS(unsigned char first, unsigned char second, unsigned char addr_mode);
void CLC(unsigned char first, unsigned char second, unsigned char addr_mode);
void CLD(unsigned char first, unsigned char second, unsigned char addr_mode);
void CLI(unsigned char first, unsigned char second, unsigned char addr_mode);
void CLV(unsigned char first, unsigned char second, unsigned char addr_mode);
void CMP(unsigned char first, unsigned char second, unsigned char addr_mode);
void CPX(unsigned char first, unsigned char second, unsigned char addr_mode);
void CPY(unsigned char first, unsigned char second, unsigned char addr_mode);
void DEC(unsigned char first, unsigned char second, unsigned char addr_mode);
void DEX(unsigned char first, unsigned char second, unsigned char addr_mode);
void DEY(unsigned char first, unsigned char second, unsigned char addr_mode);
void EOR(unsigned char first, unsigned char second, unsigned char addr_mode);
void INC(unsigned char first, unsigned char second, unsigned char addr_mode);
void INX(unsigned char first, unsigned char second, unsigned char addr_mode);
void INY(unsigned char first, unsigned char second, unsigned char addr_mode);
void JMP(unsigned char first, unsigned char second, unsigned char addr_mode);
void JSR(unsigned char first, unsigned char second, unsigned char addr_mode);
void LDA(unsigned char first, unsigned char second, unsigned char addr_mode);
void LDX(unsigned char first, unsigned char second, unsigned char addr_mode);
void LDY(unsigned char first, unsigned char second, unsigned char addr_mode);
void LSR(unsigned char first, unsigned char second, unsigned char addr_mode);
void NOP(unsigned char first, unsigned char second, unsigned char addr_mode);
void ORA(unsigned char first, unsigned char second, unsigned char addr_mode);
void PHA(unsigned char first, unsigned char second, unsigned char addr_mode);
void PHP(unsigned char first, unsigned char second, unsigned char addr_mode);
void PLA(unsigned char first, unsigned char second, unsigned char addr_mode);
void PLP(unsigned char first, unsigned char second, unsigned char addr_mode);
void ROL(unsigned char first, unsigned char second, unsigned char addr_mode);
void ROR(unsigned char first, unsigned char second, unsigned char addr_mode);
void RTI(unsigned char first, unsigned char second, unsigned char addr_mode);
void RTS(unsigned char first, unsigned char second, unsigned char addr_mode);
void SBC(unsigned char first, unsigned char second, unsigned char addr_mode);
void SEC(unsigned char first, unsigned char second, unsigned char addr_mode);
void SED(unsigned char first, unsigned char second, unsigned char addr_mode);
void SEI(unsigned char first, unsigned char second, unsigned char addr_mode);
void STA(unsigned char first, unsigned char second, unsigned char addr_mode);
void STX(unsigned char first, unsigned char second, unsigned char addr_mode);
void STY(unsigned char first, unsigned char second, unsigned char addr_mode);
void TAX(unsigned char first, unsigned char second, unsigned char addr_mode);
void TAY(unsigned char first, unsigned char second, unsigned char addr_mode);
void TSX(unsigned char first, unsigned char second, unsigned char addr_mode);
void TXA(unsigned char first, unsigned char second, unsigned char addr_mode);
void TXS(unsigned char first, unsigned char second, unsigned char addr_mode);
void TYA(unsigned char first, unsigned char second, unsigned char addr_mode);

// UNOFFICIAL OPCODES
void ALR(unsigned char first, unsigned char second, unsigned char addr_mode);
void ANC(unsigned char first, unsigned char second, unsigned char addr_mode);
void ARR(unsigned char first, unsigned char second, unsigned char addr_mode);
void ASL(unsigned char first, unsigned char second, unsigned char addr_mode);
void AXA(unsigned char first, unsigned char second, unsigned char addr_mode);
void AXS(unsigned char first, unsigned char second, unsigned char addr_mode);
void DCP(unsigned char first, unsigned char second, unsigned char addr_mode);
void ISC(unsigned char first, unsigned char second, unsigned char addr_mode);
void KIL(unsigned char first, unsigned char second, unsigned char addr_mode);
void LAS(unsigned char first, unsigned char second, unsigned char addr_mode);
void LAX(unsigned char first, unsigned char second, unsigned char addr_mode);
void OAL(unsigned char first, unsigned char second, unsigned char addr_mode);
void RLA(unsigned char first, unsigned char second, unsigned char addr_mode);
void RRA(unsigned char first, unsigned char second, unsigned char addr_mode);
void SAX(unsigned char first, unsigned char second, unsigned char addr_mode);
void SLO(unsigned char first, unsigned char second, unsigned char addr_mode);
void SRE(unsigned char first, unsigned char second, unsigned char addr_mode);
void SHX(unsigned char first, unsigned char second, unsigned char addr_mode);
void SHY(unsigned char first, unsigned char second, unsigned char addr_mode);
void TAS(unsigned char first, unsigned char second, unsigned char addr_mode);
void XAA(unsigned char first, unsigned char second, unsigned char addr_mode);

// INTERRUPT

void NMI();
void IRQ();
