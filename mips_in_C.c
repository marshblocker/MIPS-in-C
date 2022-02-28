// MIPS in C :p

#include <stdio.h>
#include <stdlib.h>

#define log printf("log\n")

#define DATA_MEM_SIZE 251		// 0x00000000 to 0x000003E8.
#define REG_FILE_SIZE 32
#define INSTR_START_ADDR 0x10000000
#define INT_MIN -2147483648
#define INT_MAX 2147483647

typedef struct mips {
	int N;
	int PC;
	int* instr_mem;
	int* data_mem;
	int* reg_file;
} MIPS;

MIPS* build_MIPS(void);
void get_input(MIPS* M);
void execute_instruction(MIPS* M, const int instruction);
void r_type_instr(MIPS* M, const int instruction);
void i_type_instr(MIPS* M, const int instruction);
void jump(MIPS* M, const int instruction);
void safe_add(int* dest, int a, int b);

int main() 
{
	MIPS* M = build_MIPS();
	get_input(M);

	int index = (M->PC - INSTR_START_ADDR) / 4;
	while (index < M->N) {
		const int instruction = M->instr_mem[index];

		execute_instruction(M, instruction);
		index = (M->PC - INSTR_START_ADDR) / 4;
	}

	return 0;
}

MIPS* build_MIPS(void)
{
	int N; scanf("%d", &N);
	MIPS* M = (MIPS*) malloc(sizeof(MIPS));

	M->N = N;
	M->PC = 0x10000000;

	M->instr_mem = (int*) malloc(N * sizeof(int));
	for (int i = 0; i < N; i++) {
		M->instr_mem[i] = 0;
	}

	M->data_mem = (int*) malloc(DATA_MEM_SIZE * sizeof(int));
	for (int i = 0; i < DATA_MEM_SIZE; i++) {
		M->data_mem[i] = 0;
	}

	M->reg_file = (int*) malloc(REG_FILE_SIZE * sizeof(int));
	for (int i = 0; i < REG_FILE_SIZE; i++) {
		M->reg_file[i] = 0;
	}

	return M;
}

void get_input(MIPS* M)
{
	for (int i = 0; i < M->N; i++) {
		scanf("%d", &M->instr_mem[i]);
	}

	return;
}

void execute_instruction(MIPS* M, const int instruction)
{
	const int opcode = (instruction & 0xFC000000);
	if (opcode == 0x00000000) {
		r_type_instr(M, instruction);
	} else if (opcode != 0x08000000) {
		i_type_instr(M, instruction);
	} else {
		jump(M, instruction);
	}
}

void r_type_instr(MIPS* M, const int instruction)
{
	int* rf = M->reg_file;

	/*
	Extract funct field of instruction.
	*/
	const int funct = instruction & 0x0000003F;

	/*
	Extract registers' addresses.
	*/
	int rs = (instruction & 0x03E00000) >> 21;
	int rt = (instruction & 0x001F0000) >> 16;
	int rd = (instruction & 0x0000F800) >> 11;

	/*
	Destination register is the zero register.
	Not allowed to modify unless the instruction
	is syscall.
	*/
	if (rd == 0 && funct != 0b001100) {
			M->PC += 4;
			return;
	}

	switch(funct)
	{
		// funct == ADD.
		case 0b100000: rf[rd] = rf[rs] + rf[rt]; break;	

		// funct == SUB.
		case 0b100010: rf[rd] = rf[rs] - rf[rt]; break;

		// funct == AND.
		case 0b100100: rf[rd] = rf[rs] & rf[rt]; break;

		// funct == OR.
		case 0b100101: rf[rd] = rf[rs] | rf[rt]; break;

		// funct == SLT.
		case 0b101010: rf[rd] = (rf[rs] < rf[rt])? 1 : 0; break;

		// funct == syscall
		case 0b001100:
		{
			int v0 = rf[2];

			// syscall 34: print int in hex.
			if (v0 == 34) {
				int a0 = rf[4];
				printf("0x%08X\n", a0);
			} else if (v0 == 10) {
				// syscall 10: terminate program.
				exit(0);
			}
			break;
		}

		default: break;
	}

	M->PC += 4;
	return;
}

void i_type_instr(MIPS* M, const int instruction)
{
	int* rf = M->reg_file;

	/*
	Extract instruction's opcode field.
	*/
	const int opcode = (instruction & 0xFC000000) >> 26;

	/*
	Extract register operands' addresses.
	*/
	int rs = (instruction & 0x03E00000) >> 21;
	int rt = (instruction & 0x001F0000) >> 16;

	/*
	Extract instruction's immediate field.
	*/
	int immediate = instruction & 0x0000FFFF;

	/*
	Sign-extended immediate for ADDIU, LW, and
	SW instructions.
	*/
	int sign_imm = ((immediate << 16) < 0)  ?
				   (0xFFFF0000 | immediate) :
				   immediate;

	switch(opcode)
	{
		/*
		opcode = ADDIU.
		*/
		case 0b001001:
		{
			if (rt != 0) {
				rf[rt] = rf[rs] + sign_imm;
			}
			M->PC += 4;
			break;
		}
		
		/*
		opcode = LW.
		*/
		case 0b100011:
		{
			if (rt != 0) {
				const int data_mem_addr = (rf[rs] + sign_imm) / 4;
				rf[rt] = M->data_mem[data_mem_addr];
			}
			M->PC += 4;
			break;
		}

		/*
		opcode = SW.
		*/
		case 0b101011:
		{
			const int data_mem_addr = (rf[rs] + sign_imm) / 4;
			M->data_mem[data_mem_addr] = rf[rt];
			M->PC += 4;
			break;
		}

		/*
		opcode = BEQ.
		*/
		case 0b000100:
		{
			const int branch_addr = sign_imm << 2;
			if (rf[rs] == rf[rt]) {
				M->PC = M->PC + 4 + branch_addr;
			} else {
				M->PC += 4;
			}
			break;
		}

		default: break;
	}

	return;
}

void jump(MIPS* M, const int instruction)
{
	/*
	JumpAddr = {PC+4[31:28], address, 2'b0}
	*/
	int jump_addr = instruction & 0x03FFFFFF;
	int head_bits = (M->PC + 4) & 0xF0000000;
	jump_addr = (head_bits) | (jump_addr << 2);
	M->PC = jump_addr;

	return;
}

void safe_add(int* dest, int a, int b)
{
	if (a >= 0) {
 		if (INT_MAX - a < b) {
   			/* would overflow */
   			return;
  		}
	} else {
  		if (b < INT_MIN - a) {
			/* would overflow */
   			return;
  		}
 	}
	*dest =  a + b;
	return;
}



