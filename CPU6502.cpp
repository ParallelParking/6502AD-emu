//6502AD microprocessor emulator
//based on designs and information by obelisk, C64 and Dave Poo

#include <stdio.h>
#include <stdlib.h>
#include <cstdint>

using byte = uint8_t; //1 byte, 8 bits
using word = uint16_t; //2 bytes 16 bits
using U32 = uint32_t; //4 bytes 32 bits

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

	void WriteWord(word value, U32 address, U32& cycles) {
		data[address] = value & 0xFF;
		data[address+1] = (value >> 8);
		cycles -= 2;
	} //writes two bytes
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

	byte FetchByte(U32& cycles, Memory& mem) {
		byte data = mem[PC];
		PC++;
		cycles--;
		return data;
	}

	byte ReadByte(U32& cycles, byte address, Memory& mem) {
		byte data = mem[address];
		cycles--;
		return data;
	}//similar to Fetch but doesn't increment program counter.

	byte FetchWord(U32& cycles, Memory& mem) {
		word data = mem[PC];
		PC++;
		data |= (mem[PC] << 8); //Has to do with the 6502 being little endian. Note that a |= b is a = a | b
		PC++;
		cycles -= 2;
		return data;
	}

	//opcodes
	static constexpr byte 
		LDA_IM = 0xA9, //Load accumulator - immediate
		LDA_ZP = 0xA5, //				  - zero page
		LDA_ZPX = 0xB5, //				  - zero page, X
		JSR = 0x20; // Jump to subroutine
	
	void LDASetStatus() {
		ZF = (A == 0);
		NF = (A & 0b1000000) > 0; //dont really get this.
	}

	void Execute(U32 cycles, Memory& mem) {
		while (cycles > 0) {
			byte Instn = FetchByte(cycles, mem); //fetches opcode
			switch (Instn) {
			case LDA_IM: {
				A = FetchByte(cycles, mem); //fetches value
				LDASetStatus();
			} break;
			case LDA_ZP: {
				byte ZeroPageAddress = FetchByte(cycles, mem);
				A = ReadByte(cycles, ZeroPageAddress, mem);
				LDASetStatus();
			} break;
			case LDA_ZPX: {
				byte ZeroPageAddress = FetchByte(cycles, mem);
				ZeroPageAddress += X;
				cycles--;
				A = ReadByte(cycles, ZeroPageAddress, mem);
				LDASetStatus();
			} break;
			case JSR: {
				word SubAddress = FetchWord(cycles, mem);
				mem.WriteWord(PC - 1, SP, cycles);
				SP++;
				PC = SubAddress;
				cycles--;
			}
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
	mem[0xFFFC] = CPU::JSR;
	mem[0xFFFD] = 0x42;
	mem[0xFFFE] = 0x42;
	mem[0x4242] = CPU::LDA_IM;
	mem[0x4243] = 0x84; //inline prog
	cpu.Execute(9, mem);
	return 0;
}
