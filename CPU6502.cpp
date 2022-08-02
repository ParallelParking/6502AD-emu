//6502AD microprocessor emulator
//based on designs and information by obelisk, C64 and Dave Poo

#include <stdio.h>
#include <stdlib.h>

using byte = unsigned char; //1 byte, 8 bits
using word = unsigned short; //2 bytes 16 bits
using U32 = unsigned int; //4 bytes 32 bits

struct Memory {
	static constexpr U32 MAX_MEM = 1024 * 64; //number of memory slots
	byte data[MAX_MEM];

	void Initialise() {
		for (U32 i = 0; i < MAX_MEM; i++) {
			data[i] = 0;
		} //resets all data slots to zero
	}

	byte operator[](U32 address) const {
		//make sure that address < MAX_MEM
		return data[address];
	} //read one byte

	byte& operator[](U32 address) {
		//make sure that address < MAX_MEM
		return data[address];
	} //writes one byte
};

struct CPU {

	word PC; //Program counter
	byte SP; //Stack pointer

	byte A; //accumulator register
	byte X, Y; //registers
	
	byte CF : 1; //carry flag (C)
	byte ZF : 1; //zero flag (Z)
	byte ID : 1; //interrupt disable (I)
	byte DM : 1; //decimal mode (D)
	byte BC : 1; //break command (V)
	byte OF : 1; //overflow flag (B)
	byte NF : 1; //negative flag (N)
				 //collectively status flags

	void Reset(Memory& mem) {
		PC = 0xFFFC;
		SP = 0x0100; //resetting counter and stack pointer

		CF = 0;
		ZF = 0;
		ID = 0;
		DM = 0;
		BC = 0;
		OF = 0;
		NF = 0; //resetting status flags

		A = 0;
		X = 0;
		Y = 0; //resetting memory

		mem.Initialise();
	}

	byte Fetch(U32& cycles, Memory& mem) {
		byte Data = mem[PC];
		PC++;
		cycles--;
		return Data;
	}

	//opcodes
	static constexpr byte 
		LDA_IM = 0xa9; //Load accumulator - immediate

	void Execute(U32 cycles, Memory& mem) {
		while (cycles > 0) {
			byte Instn = Fetch(cycles, mem); //fetches opcode
			switch (Instn) {
			case LDA_IM: {
				byte val = Fetch(cycles, mem); //fetches value
				A = val;
				ZF = (A == 0);
				NF = (A & 0b1000000) > 0; //dont really get this.
			} break;
			default: {
				printf("Instruction not handled %d", Instn);
			} break;
			}
		}
	}
};

int main() {
	CPU cpu;
	Memory mem;
	cpu.Reset(mem);
	mem[0xFFFC] = CPU::LDA_IM;
	mem[0xFFFD] = 0x36; //tiny inline program
	cpu.Execute(2, mem);
	return 0;
}