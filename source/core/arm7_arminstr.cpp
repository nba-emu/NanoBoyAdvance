/*
 * Copyright (C) 2015 Frederic Meyer
 *
 * This file is part of nanoboyadvance.
 *
 * nanoboyadvance is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * nanoboyadvance is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with nanoboyadvance. If not, see <http://www.gnu.org/licenses/>.
 */

#include "arm7.h"

constexpr auto ARM_ERR = 0;
constexpr auto ARM_1 = 1;
constexpr auto ARM_2 = 2;
constexpr auto ARM_3 = 3;
constexpr auto ARM_4 = 4;
constexpr auto ARM_5 = 5;
constexpr auto ARM_6 = 6;
constexpr auto ARM_7 = 7;
constexpr auto ARM_8 = 8;
constexpr auto ARM_9 = 9;
constexpr auto ARM_10 = 10;
constexpr auto ARM_11 = 11;
constexpr auto ARM_12 = 12;
constexpr auto ARM_13 = 13;
constexpr auto ARM_14 = 14;
constexpr auto ARM_15 = 15;
constexpr auto ARM_16 = 16;

using namespace std;

namespace NanoboyAdvance
{
int ARM7::ARMDecode(uint instruction)
{
	uint opcode = instruction & 0x0FFFFFFF;
	int section = ARM_ERR;
	switch (opcode >> 26)
	{
	case 0b00:
		if (opcode & (1 << 25))
		{
			// ARM.8 Data processing and PSR transfer ... immediate
			section = ARM_8;
		}
		// TODO: Check if we really must check all that bits
		else if ((opcode & 0xFFFFFF0) == 0x12FFF10)
		{
			// ARM.3 Branch and exchange
			section = ARM_3;
		}
		else if ((opcode & 0x10000F0) == 0x90)
		{
			// ARM.1 Multiply (accumulate), ARM.2 Multiply (accumulate) long
			section = opcode & (1 << 23) ? ARM_2 : ARM_1;
		}
		else if ((opcode & 0x10000F0) == 0x1000090)
		{
			// ARM.4 Single data swap
			section = ARM_4;
		}
		else if ((opcode & 0x4000F0) == 0xB0)
		{
			// ARM.5 Halfword data transfer, register offset
			section = ARM_5;
		}
		else if ((opcode & 0x4000F0) == 0x4000B0)
		{
			// ARM.6 Halfword data transfer, immediate offset
			section = ARM_6;
		}
		else if ((opcode & 0xD0) == 0xD0)
		{
			// ARM.7 Signed data transfer (byte/halfword)
			section = ARM_7;
		}
		else
		{
			// ARM.8 Data processing and PSR transfer
			section = ARM_8;
		}
		break;
	case 0b01:
		// ARM.9 Single data transfer, ARM.10 Undefined
		section = (opcode & 0x2000010) == 0x2000010 ? ARM_10 : ARM_9;
		break;
	case 0b10:
		// ARM.11 Block data transfer, ARM.12 Branch
		section = opcode & (1 << 25) ? ARM_12 : ARM_11;
		break;
	case 0b11:
		// TODO: Optimize with a switch?
		if (opcode & (1 << 25))
		{
			if (opcode & (1 << 24))
			{
				// ARM.16 Software interrupt
				section = ARM_16;
			}
			else
			{
				// ARM.14 Coprocessor data operation, ARM.15 Coprocessor register transfer
				section = opcode & 0x10 ? ARM_15 : ARM_14;
			}
		}
		else
		{
			// ARM.13 Coprocessor data transfer
			section = ARM_13;
		}
		break;
	}
	return section;
}

// TODO: If there is a cause for Ebola, Aids and Cancer, this is it (in other words, rewrite)
//       Implement coprocessor operation instructions,
//       Put string arrays in class definition and make them static
string ARM7::ARMDisasm(uint base, uint instruction)
{
	int type = ARMDecode(instruction);
	bool immediate { (instruction & (1 << 25)) == (1 << 25) };
	bool preindexed { (instruction & (1 << 24)) == (1 << 24) };
	bool addtobase { (instruction & (1 << 23)) == (1 << 23) };
	bool writeback { (instruction & (1 << 21)) == (1 << 21) };
	bool load { (instruction & (1 << 20)) == (1 << 20) };
	stringstream mmemonic { };
	string conditions[] = {
			"eq", "ne", "cs", "cc", "mi", "pl", "vs", "vc",
			"hi", "ls", "ge", "lt", "gt", "le", "",   "nv"
	};
	string shifts[] = { "lsl", "lsr", "asr", "ror" };
	string condition = conditions[instruction >> 28];
	switch (type)
	{
	case ARM_ERR:
		mmemonic << "<err>";
		break;
	case ARM_1:
	{
		// ARM.1 Multiply (accumulate)
		int rm = instruction & 0xF;
		int rs = (instruction >> 8) & 0xF;
		int rn = (instruction >> 12) & 0xF;
		int rd = (instruction >> 16) & 0xF;
		if (instruction & (1 << 21))
		{
			mmemonic << "mla" << condition << (instruction & (1 << 20) ? "s " : " ") << "r" << rd << ", r" << rm << ", r" << rs << ", r" << rn;
		}
		else
		{
			mmemonic << "mul" << condition << (instruction & (1 << 20) ? "s " : " ") << "r" << rd << ", r" << rm << ", r" << rs;
		}
		break;
	}
	case ARM_2:
	{
		// ARM.2 Multiply (accumulate) long
		int rm = instruction & 0xF;
		int rs = (instruction >> 8) & 0xF;
		int rd_lsw = (instruction >> 12) & 0xF;
		int rd_msw = (instruction >> 16) & 0xF;
		mmemonic << (instruction & (1 << 22) ? "s" : "u") << (instruction & (1 << 21) ? "mlal" : "mull") << condition << (instruction & (1 << 20) ? "s " : " ") <<
				"r" << rd_lsw << ", r" << rd_msw << ", r" << rm << ", r" << rs;
		break;
	}
	case ARM_3:
		// ARM.3 Branch and exchange
		mmemonic << "bx" << condition << " r" << (instruction & 0xF);
		break;
	case ARM_4:
	{
		// ARM.4 Single data swap
		int rm = instruction & 0xF;
		int rd = (instruction >> 12) & 0xF;
		int rn = (instruction >> 16) & 0xF;
		mmemonic << "swp" << condition << (instruction & (1 << 22) ? "b " : " ") << "r" << rd << ", r" << rm << ", [r" << rn << "]";
		break;
	}
	// TODO: ARM.5 to ARM.7 reduce code redundancy
	case ARM_5:
	{
		// ARM.5 Halfword data transfer, register offset
		int rm = instruction & 0xF;
		int rd = (instruction >> 12) & 0xF;
		int rn = (instruction >> 16) & 0xF;
		mmemonic << (load ? "ldr" : "str") << condition << "h r" << rd << ", [r" << rn;
		if (preindexed)
		{
			mmemonic << ", " << (addtobase ? "r" : "-r") << rm << "]" << (writeback ? "!" : "");
		}
		else
		{
			mmemonic << "]" << ", " << (addtobase ? "r" : "-r") << rm << (writeback ? "!" : "");
		}
		break;
	}
	case ARM_6:
	{
		// ARM.6 Halfword data transfer, immediate offset
		int offset = (instruction & 0xF) | ((instruction >> 4) & 0xF0);
		int rd = (instruction >> 12) & 0xF;
		int rn = (instruction >> 16) & 0xF;
		mmemonic << (load ? "ldr" : "str") << condition << "h r" << rd << ", [r" << rn;
		if (preindexed)
		{
			mmemonic << ", " << (addtobase ? "#0x" : "-#0x") << hex << offset << dec << "]" << (writeback ? "!" : "");
		}
		else
		{
			mmemonic << "]" << ", " << (addtobase ? "#0x" : "-#0x") << hex << offset << dec << (writeback ? "!" : "");
		}
		break;
	}
	case ARM_7:
	{
		// ARM.7 Signed data transfer
		int rd = (instruction >> 12) & 0xF;
		int rn = (instruction >> 16) & 0xF;
		bool immediate2 { (instruction & (1 << 22)) == (1 << 22) };
		stringstream displacement { };
		mmemonic << (load ? "ldr" : "str") << condition << (instruction & (1 << 5) ? "sh r" : "sb r") << rd << ", [r" << rn;
		if (immediate2)
		{
			int offset = (instruction & 0xF) | ((instruction >> 4) & 0xF0);
			displacement << (addtobase ? "#0x" : "-#0x") << hex << offset << dec;
		}
		else
		{
			int rm = instruction & 0xF;
			displacement << (addtobase ? "r" : "-r") << rm;
		}
		if (preindexed)
		{
			mmemonic << ", " << displacement.str() << "]";
		}
		else
		{
			mmemonic << "], " << displacement.str();
		}
		mmemonic << (writeback ? "!" : "");
		break;
	}
	case ARM_8:
	{
		// ARM.8 Data processing and PSR transfer
		int opcode = (instruction >> 21) & 0xF;
		bool set_flags { (instruction & (1 << 20)) == (1 << 20) };
		if (!set_flags && opcode >= 0b1000 && opcode <= 0b1011)
		{
			// PSR transfer
			switch ((instruction >> 16) & 0x3F)
			{
			// MRS (transfer PSR contents to a register)
			case 0b001111:
			{
				int rd = (instruction >> 12) & 0xF;
				mmemonic << "mrs" << condition << " r" << rd << (instruction & (1 << 22) ? ", spsr_fsxc" : ", cpsr"); // is the _fsxc really correct? (vba appends this)
				break;
			}
			// MSR (transfer register contents to PSR)
			case 0b101001:
			{
				int rm = instruction & 0xF;
				mmemonic << "msr" << condition << (instruction & (1 << 22) ? " spsr_fc, r" : " cpsr_fc, r") << rm; // is the _fc really correct? (vba appends this)
				break;
			}
			// MSR (transfer register contents or immediate value to PSR flag bits only)
			case 0b101000:
			{
				mmemonic << "msr" << condition << (instruction & (1 << 22) ? " spsr_f, " : " cpsr_f, "); // is the _f really correct? (vba appends this)
				if (immediate)
				{
					int imm = instruction & 0xFF;
					int ror = ((instruction >> 8) & 0xF) * 2;
					uint result = (imm >> ror) | (imm << (32 - ror));
					mmemonic << "#0x" << hex << result << dec;
				}
				else
				{
					mmemonic << "r" << (instruction & 0xF);
				}
				break;
			}
			// Undefined operation
			default:
				mmemonic << "<brokenpsr>";
				break;
			}
		}
		else
		{
			// Data processing
			int rd = (instruction >> 12) & 0xF;
			int rn = (instruction >> 16) & 0xF;
			string opcodes[] = {
					"and", "eor", "sub", "rsb", "add", "adc", "sbc", "rsc",
					"tst", "teq", "cmp", "cmn", "orr", "mov", "bic", "mvn"
			};
			mmemonic << opcodes[opcode] << condition << (set_flags ? "s " : " ");
			if (opcode >= 0b1000 && opcode <= 0b1011)
			{
				mmemonic << "r" << rn;
			}
			else if (opcode == 0b1101 || opcode == 0b1111)
			{
				mmemonic << "r" << rd;
			}
			else
			{
				mmemonic << "r" << rd << ", r" << rn;
			}
			// Handle operand2
			if (immediate)
			{
				int imm = instruction & 0xFF;
				int ror = ((instruction >> 8) & 0xF) * 2;
				uint result = (imm >> ror) | (imm << (32 - ror));
				mmemonic << ", #0x" << hex << result << dec;
			}
			else
			{
				int rm = instruction & 0xF;
				mmemonic << ", r" << rm << ", " << shifts[(instruction >> 5) & 3];
				if (instruction & (1 << 4))
				{
					mmemonic << " r" << ((instruction >> 8) & 0xF);
				}
				else
				{
					mmemonic << " #0x" << hex << ((instruction >> 7) & 0x1F) << dec;
				}
			}
		}
		break;
	}
	case ARM_9:
	{
		// ARM.9 Load/store register/unsigned byte
		stringstream offset { };
		int rd = (instruction >> 12) & 0xF;
		int rn = (instruction >> 16) & 0xF;
		mmemonic << (load ? "ldr" : "str") << condition << (instruction & (1 << 22) ? "b r" : " r") << rd << ", [r" << rn;
		// is it an immediate instruction?
		if (immediate)
		{
			int rm = instruction & 0xF;
			offset << (addtobase ? "r" : "-r") << rm << ", " << shifts[(instruction >> 5) & 3];
			// is operand2 of the shift immediate or register?
			if (instruction & (1 << 4))
			{
				int rs = (instruction >> 8) & 0xF;
				offset << " r" << rs;
			}
			else
			{
				offset << " #0x" << hex << ((instruction >> 7) & 0x1F) << dec;
			}
		}
		else
		{
			offset << (addtobase ? "#0x" : "-#0x") << hex << (instruction & 0xFFF) << dec;
		}
		// is the instruction pre-indexed?
		if (preindexed)
		{
			mmemonic << ", " << offset.str() << "]" << (writeback ? "!" : "");
		}
		else
		{
			mmemonic << "], " << offset.str() << (writeback ? "!" : "");
		}
		break;
	}
	case ARM_10:
		// ARM.10 Undefined
		mmemonic << "<undef>";
		break;
	case ARM_11:
	{
		// ARM.11 Block data transfer
		int rn = (instruction >> 16) & 0xF;
		bool forceuser { (instruction & (1 << 22)) == (1 << 22) };
		bool listing { false };
		bool requirecomma { false };
		if (load && preindexed && addtobase)
		{
			mmemonic << "ldm" << condition << (rn == 13 ? "ed" : "ib");
		}
		else if (load && !preindexed && addtobase)
		{
			mmemonic << "ldm" << condition << (rn == 13 ? "fd" : "ia");
		}
		else if (load && preindexed && !addtobase)
		{
			mmemonic << "ldm" << condition << (rn == 13 ? "ea" : "db");
		}
		else if (load && !preindexed && !addtobase)
		{
			mmemonic << "ldm" << condition << (rn == 13 ? "fa" : "da");
		}
		else if (!load && preindexed && addtobase)
		{
			mmemonic << "stm" << condition << (rn == 13 ? "fa" : "ib");
		}
		else if (!load && !preindexed && addtobase)
		{
			mmemonic << "stm" << condition << (rn == 13 ? "ea" : "ia");
		}
		else if (!load && preindexed && !addtobase)
		{
			mmemonic << "stm" << condition << (rn == 13 ? "fd" : "db");
		}
		else
		{
			mmemonic << "stm" << condition << (rn == 13 ? "ed" : "da");
		}
		mmemonic << " r" << rn << (writeback ? "!, {" : ", {");
		for (int i = 0; i < 16; i++)
		{
			if (instruction & (1 << i))
			{
				bool neighborset { false };
				if (i != 15 && (instruction & (1 << (i + 1))))
				{
					neighborset = true;
				}
				if (neighborset && !listing)
				{
					listing = true;
					mmemonic << (requirecomma ? ",r" : "r") << i << "-";
				}
				else if (!neighborset && listing)
				{
					listing = false;
					mmemonic << "r" << i;
				}
				else if (!neighborset && !listing)
				{
					mmemonic << (requirecomma ? ",r" : "r") << i;
				}
				requirecomma = true;
			}
		}
		mmemonic << (forceuser ? "}^" : "}");
		break;
	}
	case ARM_12:
	{
		// ARM.12 Branch
		int offset = instruction & 0xFFFFFF;
		if (offset & 0x800000)
		{
			offset |= 0xFF000000;
		}
		offset <<= 2;
		mmemonic << (instruction & (1 << 24) ? "bl" : "b") << condition << " 0x" << hex << (base + 8 + offset) << dec;
		break;
	}
	case ARM_13:
		// ARM.13 Coprocessor data transfer
		mmemonic << "<coproc>";
		break;
	case ARM_14:
		// ARM.14 Coprocessor data operation
		mmemonic << "<coproc2>";
		break;
	case ARM_15:
		// ARM.15 Coprocessor register transfer
		mmemonic << "<coproc3>";
		break;
	case ARM_16:
		// ARM.16 Software interrupt
		mmemonic << "swi" << condition << " #0x" << hex << (instruction & 0xFFFFFF) << dec;
		break;
	}
	return mmemonic.str();
}

void ARM7::ARMExecute(uint instruction, int type)
{
	int condition = instruction >> 28;
	bool execute { false };

#ifdef CPU_LOG
	// Log our status for debug reasons
	LOG(LOG_INFO, "Executing %s, r15=0x%x", ARMDisasm(r15 - 8, instruction).c_str(), r15);
	for (int i = 0; i < 16; i++)
	{
		if (i == 15)
		{
			cout << "r" << i << " = 0x" << std::hex << reg(i) << " (0x" << (reg(i) - 8) << ")" << std::dec << endl;
		}
		else
		{
			cout << "r" << i << " = 0x" << std::hex << reg(i) << std::dec << endl;
		}
	}
	cout << "cpsr = 0x" << std::hex << cpsr << std::dec << endl;
	cout << "spsr = 0x" << std::hex << *pspsr << std::dec << endl;
	cout << "mode = ";
	switch (cpsr & 0x1F)
	{
	case User: cout << "User" << endl; break;
	case System: cout << "System" << endl; break;
	case IRQ: cout << "IRQ" << endl; break;
	case SVC: cout << "SVC" << endl; break;
	default: cout << "n.n." << endl; break;
	}
#endif

	// Check if the instruction will be executed
	switch (condition)
	{
	case 0x0: execute = (cpsr & ZeroFlag) == ZeroFlag; break;
	case 0x1: execute = (cpsr & ZeroFlag) != ZeroFlag; break;
	case 0x2: execute = (cpsr & CarryFlag) == CarryFlag; break;
	case 0x3: execute = (cpsr & CarryFlag) != CarryFlag; break;
	case 0x4: execute = (cpsr & SignFlag) == SignFlag; break;
	case 0x5: execute = (cpsr & SignFlag) != SignFlag; break;
	case 0x6: execute = (cpsr & OverflowFlag) == OverflowFlag; break;
	case 0x7: execute = (cpsr & OverflowFlag) != OverflowFlag; break;
	case 0x8: execute = ((cpsr & CarryFlag) == CarryFlag) & ((cpsr & ZeroFlag) != ZeroFlag); break;
	case 0x9: execute = ((cpsr & CarryFlag) != CarryFlag) || ((cpsr & ZeroFlag) == ZeroFlag); break;
	case 0xA: execute = ((cpsr & SignFlag) == SignFlag) == ((cpsr & OverflowFlag) == OverflowFlag); break;
	case 0xB: execute = ((cpsr & SignFlag) == SignFlag) != ((cpsr & OverflowFlag) == OverflowFlag); break;
	case 0xC: execute = ((cpsr & ZeroFlag) != ZeroFlag) && (((cpsr & SignFlag) == SignFlag) == ((cpsr & OverflowFlag) == OverflowFlag)); break;
	case 0xD: execute = ((cpsr & ZeroFlag) == ZeroFlag) || (((cpsr & SignFlag) == SignFlag) != ((cpsr & OverflowFlag) == OverflowFlag)); break;
	case 0xE: execute = true; break;
	case 0xF: execute = false; break;
	}

	// If it will not be executed return now
	if (!execute)
	{
		return;
	}

	// Perform the actual execution
	switch (type)
	{
	case ARM_1:
	{
		// ARM.1 Multiply (accumulate)
		int reg_operand1 = instruction & 0xF;
		int reg_operand2 = (instruction >> 8) & 0xF;
		int reg_operand3 = (instruction >> 12) & 0xF;
		int reg_dest = (instruction >> 16) & 0xF;
		bool set_flags { (instruction & (1 << 20)) == (1 << 20) };
		bool accumulate { (instruction & (1 << 21)) == (1 << 21) };
		reg(reg_dest) = reg(reg_operand1) * reg(reg_operand2);
		if (accumulate)
		{
			reg(reg_dest) += reg(reg_operand3);
		}
		calculate_sign(reg(reg_dest));
		calculate_zero(reg(reg_dest));
		break;
	}
	case ARM_2:
	{
		// ARM.2 Multiply (accumulate) long
		int reg_operand1 = instruction & 0xF;
		int reg_operand2 = (instruction >> 8) & 0xF;
		int reg_dest_low = (instruction >> 12) & 0xF;
		int reg_dest_high = (instruction >> 16) & 0xF;
		bool set_flags { (instruction & (1 << 20)) == (1 << 20) };
		bool accumulate { (instruction & (1 << 21)) == (1 << 21) };
		bool sign_extend { (instruction & (1 << 22)) == (1 << 22) };
		slong result { };
		if (sign_extend)
		{
			slong operand1 = reg(reg_operand1);
			slong operand2 = reg(reg_operand2);
			operand1 |= operand1 & 0x80000000 ? 0xFFFFFFFF00000000 : 0;
			operand2 |= operand2 & 0x80000000 ? 0xFFFFFFFF00000000 : 0;
			result = operand1 * operand2;
		}
		else
		{
			ulong uresult = (ulong)reg(reg_operand1) * (ulong)reg(reg_operand2);
			result = uresult;
		}
		if (accumulate)
		{
			slong value = reg(reg_dest_high);
			value <<= 16;
			value <<= 16;
			value |= reg(reg_dest_low);
			result += value;
		}
		reg(reg_dest_low) = result & 0xFFFFFFFF;
		reg(reg_dest_high) = result >> 32;
		calculate_sign(reg(reg_dest_high));
		calculate_zero(result);
		break;
	}
	case ARM_3:
	{
		// ARM.3 Branch and exchange
		int reg_address = instruction & 0xF;
		if (reg(reg_address) & 1)
		{
			r15 = reg(reg_address) & ~(1);
			cpsr |= Thumb;
		}
		else
		{
			r15 = reg(reg_address) & ~(3);
		}
		flush_pipe = true;
		break;
	}
	case ARM_4:
	{
		// ARM.4 Single data swap
		int reg_source = instruction & 0xF;
		int reg_dest = (instruction >> 12) & 0xF;
		int reg_base = (instruction >> 16) & 0xF;
		bool swap_byte { (instruction & (1 << 22)) == (1 << 22) };
		uint memory_value { };
		ASSERT(reg_source == 15 || reg_dest == 15 || reg_base == 15, LOG_WARN, "Single Data Transfer, thou shall not use r15, r15=0x%x", r15);
		if (swap_byte)
		{
			memory_value = ReadByte(reg(reg_base)) & 0xFF;
			WriteByte(reg(reg_base), reg(reg_source) & 0xFF);
			reg(reg_dest) = memory_value;
		}
		else
		{
			memory_value = ReadWordRotated(reg(reg_base));
			WriteWord(reg(reg_base), reg(reg_source));
			reg(reg_dest) = memory_value;
		}
		break;
	}
	// TODO: Recheck for correctness and look for possabilities to optimize this bunch of code
	case ARM_5:
	case ARM_6:
	case ARM_7:
	{
		// ARM.5 Halfword data transfer, register offset
		// ARM.6 Halfword data transfer, immediate offset
		// ARM.7 Signed data transfer (byte/halfword)
		// TODO: Proper alignment handling
		uint offset { };
		int reg_dest = (instruction >> 12) & 0xF;
		int reg_base = (instruction >> 16) & 0xF;
		bool load { (instruction & (1 << 20)) == (1 << 20) };
		bool write_back { (instruction & (1 << 21)) == (1 << 21) };
		bool immediate { (instruction & (1 << 22)) == (1 << 22) };
		bool add_to_base { (instruction & (1 << 23)) == (1 << 23) };
		bool pre_indexed { (instruction & (1 << 24)) == (1 << 24) };
		uint address = reg(reg_base);

		// Instructions neither write back if base register is r15 nor should they have the write-back bit set when being post-indexed (post-indexing automatically writes back the address)
		ASSERT(reg_base == 15 && write_back, LOG_WARN, "Halfword and Signed Data Transfer, thou shall not writeback to r15, r15=0x%x", r15);
		ASSERT(write_back && !pre_indexed, LOG_WARN, "Halfword and Signed Data Transfer, thou shall not have write-back bit if being post-indexed, r15=0x%x", r15);

		// Load-bit shall not be unset when signed transfer is selected
		ASSERT(type == ARM_7 && !load, LOG_WARN, "Halfword and Signed Data Transfer, thou shall not store in signed mode, r15=0x%x", r15);

		// If the instruction is immediate take an 8-bit immediate value as offset, otherwise take the contents of a register as offset
		if (immediate)
		{
			offset = (instruction & 0xF) | ((instruction >> 4) & 0xF0);
		}
		else
		{
			int reg_offset = instruction & 0xF;
			ASSERT(reg_offset == 15, LOG_WARN, "Halfword and Signed Data Transfer, thou shall not take r15 as offset, r15=0x%x", r15);
			offset = reg(reg_offset);
		}

		// If the instruction is pre-indexed we must add/subtract the offset beforehand
		if (pre_indexed)
		{
			if (add_to_base)
			{
				address += offset;
			}
			else
			{
				address -= offset;
			}
		}

		// Perform the actual load / store operation
		if (load)
		{
			// TODO: Check if pipeline is flushed when reg_dest is r15
			if (type == ARM_7)
			{
				bool halfword { (instruction & (1 << 5)) == (1 << 5) };
				uint value { };
				if (halfword)
				{
					value = ReadHWord(address);
					if (value & 0x8000)
					{
						value |= 0xFFFF0000;
					}
				}
				else
				{
					value = ReadByte(address);
					if (value & 0x80)
					{
						value |= 0xFFFFFF00;
					}
				}
				reg(reg_dest) = value;
			}
			else
			{
				reg(reg_dest) = ReadHWord(address);
			}
		}
		else
		{
			if (reg_dest == 15)
			{
				WriteHWord(address, r15 + 4);
			}
			else
			{
				WriteHWord(address, reg(reg_dest));
			}
		}

		// When the instruction either is pre-indexed and has the write-back bit or it's post-indexed we must writeback the calculated address
		if ((write_back || !pre_indexed) && reg_base != reg_dest)
		{
			if (!pre_indexed)
			{
				if (add_to_base)
				{
					address += offset;
				}
				else
				{
					address -= offset;
				}
			}
			reg(reg_base) = address;
		}
		break;
	}
	case ARM_8:
	{
		// ARM.8 Data processing and PSR transfer
		bool set_flags { (instruction & (1 << 20)) == (1 << 20) };
		int opcode = (instruction >> 21) & 0xF;

		// Determine wether the instruction is data processing or psr transfer
		if (!set_flags && opcode >= 0b1000 && opcode <= 0b1011)
		{
			// PSR transfer
			bool use_spsr { (instruction & (1 << 22)) == (1 << 22) };
			switch ((instruction >> 16) & 0x3F)
			{
			case 0b001111:
			{
				// MRS (transfer PSR contents to a register)
				int reg_dest = (instruction >> 12) & 0xF;
				reg(reg_dest) = use_spsr ? *pspsr : cpsr;
				break;
			}
			case 0b101001:
			{
				// MSR (transfer register contents to PSR)
				int reg_source = instruction & 0xF;
				if (use_spsr)
				{
					*pspsr = reg(reg_source);
				}
				else
				{
					if ((cpsr & 0x1F) == User)
					{
						cpsr = (cpsr & 0x0FFFFFFF) | (reg(reg_source) & 0xF0000000);
					}
					else
					{
						cpsr = reg(reg_source);
						RemapRegisters();
					}
				}
				break;
			}
			case 0b101000:
			{
				// MSR (transfer register contents or immediate value to PSR flag bits only)
				bool immediate { (instruction & (1 << 25)) == (1 << 25) };
				uint operand { };
				if (immediate)
				{
					int immediate_value = instruction & 0xFF;
					int amount = ((instruction >> 8) & 0xF) << 1;
					operand = (immediate_value >> amount) | (immediate_value << (32 - amount));
				}
				else
				{
					int reg_source = instruction & 0xF;
					operand = reg(reg_source);
				}
				if (use_spsr)
				{
					*pspsr = (*pspsr & 0x0FFFFFFF) | (operand & 0xF0000000);
				}
				else
				{
					cpsr = (cpsr & 0x0FFFFFFF) | (operand & 0xF0000000);
				}
				break;
			}
			default:
				LOG(LOG_ERROR, "Malformed PSR transfer instruction, r15=0x%x", r15);
				break;
			}
		}
		else
		{
			// Data processing
			int reg_dest = (instruction >> 12) & 0xF;
			int reg_operand1 = (instruction >> 16) & 0xF;
			bool immediate { (instruction & (1 << 25)) == (1 << 25) };
			uint operand1 = reg(reg_operand1);
			uint operand2 { };
			bool carry { (cpsr & CarryFlag) == CarryFlag };

			// Operand 2 can either be an 8 bit immediate value rotated right by 4 bit value or the value of a register shifted by a specific amount
			if (immediate)
			{
				int immediate_value = instruction & 0xFF;
				int amount = ((instruction >> 8) & 0xF) << 1;
				operand2 = (immediate_value >> amount) | (immediate_value << (32 - amount));
				if (amount != 0)
				{
					carry = (immediate_value >> (amount - 1)) & 1 ? true : false;
				}
			}
			else
			{
				int reg_operand2 = instruction & 0xF;
				uint amount { };
				operand2 = reg(reg_operand2);

				// When r15 is used as operand and operand2 is not immediate its value will be 12 bytes ahead instead of 8 bytes
				if (reg_operand1 == 15)
				{
					operand1 += 4;
				}
				if (reg_operand2 == 15)
				{
					operand2 += 4;
				}

				// The amount is either the value of a register or a 5 bit immediate
				if (instruction & (1 << 4))
				{
					int reg_shift = (instruction >> 8) & 0xF;
					amount = reg(reg_shift);

					// When r15 is used as operand and operand2 is not immediate its value will be 12 bytes ahead instead of 8 bytes
					// TODO: Check if this is actually done in this case
					if (reg_shift == 15)
					{
						amount += 4;
					}
				}
				else
				{
					amount = (instruction >> 7) & 0x1F;
				}

				// Perform the actual shift/rotate
				switch ((instruction >> 5) & 3)
				{
				case 0b00:
					// Logical Shift Left
					if (amount != 0)
					{
						uint result = amount >= 32 ? 0 : operand2 << amount;
						carry = (operand2 << (amount - 1)) & 0x80000000 ? true : false;
						operand2 = result;
					}
					break;
				case 0b01:
				{
					// Logical Shift Right
					uint result { };
					if (amount == 0)
					{
						amount = 32;
					}
					result = amount >= 32 ? 0 : operand2 >> amount;
					carry = (operand2 >> (amount - 1)) & 1 ? true : false;
					operand2 = result;
					break;
				}
				case 0b10:
				{
					// Arithmetic Shift Right
					sint result { };
					sint extended = (operand2 & 0x80000000) == 0x80000000 ? 0xFFFFFFFF : 0;
					if (amount == 0)
					{
						amount = 32;
					}
					result = amount >= 32 ? extended : (sint)operand2 >> (sint)amount;
					carry = (operand2 >> (amount - 1)) & 1 ? true : false;
					operand2 = result;
					break;
				}
				case 0b11:
					// Rotate Right
					if (amount != 0)
					{
						for (int i = 1; i <= amount; i++)
						{
							uint high_bit = (operand2 & 1) << 31;
							operand2 = (operand2 >> 1) | high_bit;
							if (i == amount)
							{
								carry = high_bit == 0x80000000;
							}
						}
					}
					else
					{
						bool old_carry { carry };
						carry = (operand2 & 1) == 1;
						operand2 = (operand2 >> 1) | old_carry ? 0x80000000 : 0;
					}
					break;
				}
			}

			// When destination register is r15 and s bit is set rather than updating the flags restore cpsr
			// This is allows for restoring r15 and cpsr at the same time
			if (reg_dest == 15 && set_flags)
			{
				set_flags = false;
				cpsr = *pspsr;
				RemapRegisters();
			}

			// Perform the actual operation
			switch (opcode)
			{
			case 0b0000: // AND
			{
				uint result = operand1 & operand2;
				if (set_flags)
				{
					calculate_sign(result);
					calculate_zero(result);
					assert_carry(carry);
				}
				reg(reg_dest) = result;
				break;
			}
			case 0b0001: // EOR
			{
				uint result = operand1 ^ operand2;
				if (set_flags)
				{
					calculate_sign(result);
					calculate_zero(result);
					assert_carry(carry);
				}
				reg(reg_dest) = result;
				break;
			}
			case 0b0010: // SUB
			{
				uint result = operand1 - operand2;
				if (set_flags)
				{
					assert_carry(operand1 >= operand2);
					calculate_overflow_sub(result, operand1, operand2);
					calculate_sign(result);
					calculate_zero(result);
				}
				reg(reg_dest) = result;
				break;
			}
			case 0b0011: // RSB
			{
				uint result = operand2 - operand1;
				if (set_flags)
				{
					assert_carry(operand2 >= operand1);
					calculate_overflow_sub(result, operand2, operand1);
					calculate_sign(result);
					calculate_zero(result);
				}
				reg(reg_dest) = result;
				break;
			}
			case 0b0100: // ADD
			{
				uint result = operand1 + operand2;
				if (set_flags)
				{
					ulong result_long = (ulong)operand1 + (ulong)operand2;
					assert_carry(result_long & 0x100000000);
					calculate_overflow_add(result, operand1, operand2);
					calculate_sign(result);
					calculate_zero(result);
				}
				reg(reg_dest) = result;
				break;
			}
			case 0b0101: // ADC
			{
				int carry2 { (cpsr >> 29) & 1 };
				uint result = operand1 + operand2 + carry2;
				if (set_flags)
				{
					ulong result_long = (ulong)operand1 + (ulong)operand2 + (ulong)carry2;
					assert_carry(result_long & 0x100000000);
					calculate_overflow_add(result, operand1, operand2 + carry2);
					calculate_sign(result);
					calculate_zero(result);
				}
				reg(reg_dest) = result;
				break;
			}
			case 0b0110: // SBC
			{
				int carry2 { (cpsr >> 29) & 1 };
				uint result = operand1 - operand2 + carry2 - 1;
				if (set_flags)
				{
					assert_carry(operand1 >= (operand2 + carry2 - 1));
					calculate_overflow_sub(result, operand1, (operand2 + carry2 - 1));
					calculate_sign(result);
					calculate_zero(result);
				}
				reg(reg_dest) = result;
				break;
			}
			case 0b0111: // RSC
			{
				int carry2 { (cpsr >> 29) & 1 };
				uint result = operand2 - operand1 + carry2 - 1;
				if (set_flags)
				{
					assert_carry(operand2 >= (operand1 + carry2 - 1));
					calculate_overflow_sub(result, operand2, (operand1 + carry2 - 1));
					calculate_sign(result);
					calculate_zero(result);
				}
				reg(reg_dest) = result;
				break;
			}
			case 0b1000: // TST
			{
				uint result = operand1 & operand2;
				calculate_sign(result);
				calculate_zero(result);
				assert_carry(carry);
				break;
			}
			case 0b1001: // TEQ
			{
				uint result = operand1 ^ operand2;
				calculate_sign(result);
				calculate_zero(result);
				assert_carry(carry);
				break;
			}
			case 0b1010: // CMP
			{
				uint result = operand1 - operand2;
				assert_carry(operand1 >= operand2);
				calculate_overflow_sub(result, operand1, operand2);
				calculate_sign(result);
				calculate_zero(result);
				break;
			}
			case 0b1011: // CMN
			{
				uint result = operand1 + operand2;
				ulong result_long = (ulong)operand1 + (ulong)operand2;
				assert_carry(result_long & 0x100000000);
				calculate_overflow_add(result, operand1, operand2);
				calculate_sign(result);
				calculate_zero(result);
				break;
			}
			case 0b1100: // ORR
			{
				uint result = operand1 | operand2;
				if (set_flags)
				{
					calculate_sign(result);
					calculate_zero(result);
					assert_carry(carry);
				}
				reg(reg_dest) = result;
				break;
			}
			case 0b1101: // MOV
			{
				if (set_flags)
				{
					calculate_sign(operand2);
					calculate_zero(operand2);
					assert_carry(carry);
				}
				reg(reg_dest) = operand2;
				break;
			}
			case 0b1110: // BIC
			{
				uint result = operand1 & ~operand2;
				if (set_flags)
				{
					calculate_sign(result);
					calculate_zero(result);
					assert_carry(carry);
				}
				reg(reg_dest) = result;
				break;
			}
			case 0b1111: // MVN
			{
				uint not_operand2 = ~operand2;
				if (set_flags)
				{
					calculate_sign(not_operand2);
					calculate_zero(not_operand2);
					assert_carry(carry);
				}
				reg(reg_dest) = not_operand2;
				break;
			}
			}

			// When writing to r15 initiate pipeline flush
			if (reg_dest == 15)
			{
				flush_pipe = true;
			}
		}
		break;
	}
	case ARM_9:
	{
		// ARM.9 Load/store register/unsigned byte (Single Data Transfer)
		// TODO: Force user mode when instruction is post-indexed and has writeback bit
		uint offset { };
		int reg_dest = (instruction >> 12) & 0xF;
		int reg_base = (instruction >> 16) & 0xF;
		bool load { (instruction & (1 << 20)) == (1 << 20) };
		bool write_back { (instruction & (1 << 21)) == (1 << 21) };
		bool transfer_byte { (instruction & (1 << 22)) == (1 << 22) };
		bool add_to_base { (instruction & (1 << 23)) == (1 << 23) };
		bool pre_indexed { (instruction & (1 << 24)) == (1 << 24) };
		bool immediate { (instruction & (1 << 25)) == 0 };
		uint address = reg(reg_base);

		// Instructions neither write back if base register is r15 nor should they have the write-back bit set when being post-indexed (post-indexing automatically writes back the address)
		ASSERT(reg_base == 15 && write_back, LOG_WARN, "Single Data Transfer, thou shall not writeback to r15, r15=0x%x", r15);
		ASSERT(write_back && !pre_indexed, LOG_WARN, "Single Data Transfer, thou shall not have write-back bit if being post-indexed, r15=0x%x", r15);

		// The offset added to the base address can either be an 12 bit immediate value or a register shifted by 5 bit immediate value
		if (immediate)
		{
			offset = instruction & 0xFFF;
		}
		else
		{
			int reg_offset = instruction & 0xF;
			uint shift_operand1 = reg(reg_offset);
			uint shift_operand2 = (instruction >> 7) & 0x1F;
			int shift = (instruction >> 5) & 3;
			ASSERT(reg_offset == 15, LOG_WARN, "Single Data Transfer, thou shall not use r15 as offset, r15=0x%x", r15);

			// We can safe some work by ensuring that we do not shift by zero bits
			switch (shift)
			{
			case 0b00:
			{
				// Logical Shift Left
				offset = shift_operand2 >= 32 ? 0 : shift_operand1 << shift_operand2;
				break;
			}
			case 0b01:
			{
				// Logical Shift Right
				if (shift_operand2 == 0)
				{
					shift_operand2 = 32;
				}
				offset = shift_operand2 >= 32 ? 0 : shift_operand1 >> shift_operand2;
				break;
			}
			case 0b10:
			{
				// Arithmetic Shift Right
				sint result { };
				sint extended = (shift_operand1 & 0x80000000) == 0x80000000 ? 0xFFFFFFFF : 0;
				if (shift_operand2 == 0)
				{
					shift_operand2 = 32;
				}
				result = shift_operand2 >= 32 ? extended : (sint)shift_operand1 >> (sint)shift_operand2;
				offset = (uint)result;
				break;
			}
			case 0b11:
			{
				// Rotate Right
				// TODO: RRX
				offset = shift_operand1;
				for (int i = 1; i <= shift_operand2; i++)
				{
					uint high_bit = (offset & 1) << 31;
					offset = (offset >> 1) | high_bit;
				}
				break;
			}
			}
		}

		// If the instruction is pre-indexed we must add/subtract the offset beforehand
		if (pre_indexed)
		{
			if (add_to_base)
			{
				address += offset;
			}
			else
			{
				address -= offset;
			}
		}

		// Perform the actual load / store operation
		if (load)
		{
			if (transfer_byte)
			{
				reg(reg_dest) = ReadByte(address);
			}
			else
			{
				reg(reg_dest) = ReadWordRotated(address);
			}
			if (reg_dest == 15)
			{
				flush_pipe = true;
			}
		}
		else
		{
			uint value = reg(reg_dest);
			if (reg_dest == 15)
			{
				value += 4;
			}
			if (transfer_byte)
			{
				WriteByte(address, value & 0xFF);
			}
			else
			{
				WriteWord(address, value);
			}
		}

		// When the instruction either is pre-indexed and has the write-back bit or it's post-indexed we must writeback the calculated address
		if ((write_back || !pre_indexed) && reg_base != reg_dest)
		{
			if (!pre_indexed)
			{
				if (add_to_base)
				{
					address += offset;
				}
				else
				{
					address -= offset;
				}
			}
			reg(reg_base) = address;
		}
		break;
	}
	case ARM_10:
		// ARM.10 Undefined
		LOG(LOG_ERROR, "Undefined instruction, r15=0x%x", r15);
		break;
	case ARM_11:
	{
		// ARM.11 Block Data Transfer
		bool pc_in_list { (instruction & (1 << 15)) == (1 << 15) };
		int reg_base = (instruction >> 16) & 0xF;
		bool load { (instruction & (1 << 20)) == (1 << 20) };
		bool write_back { (instruction & (1 << 21)) == (1 << 21) };
		bool s_bit { (instruction & (1 << 22)) == (1 << 22) }; // TODO: Give this a meaningful name
		bool add_to_base { (instruction & (1 << 23)) == (1 << 23) };
		bool pre_indexed { (instruction & (1 << 24)) == (1 << 24) };
		uint address = reg(reg_base);
		uint old_address = address;
		bool switched_mode { false };
		int old_mode { };
		int first_register { };

		// Base register must not be r15
		ASSERT(reg_base == 15, LOG_WARN, "Block Data Tranfser, thou shall not take r15 as base register, r15=0x%x", r15);

		// If the s bit is set and the instruction is either a store or r15 is not in the list switch to user mode
		if (s_bit && (!load || !pc_in_list))
		{
			// Writeback must not be activated in this case
			ASSERT(write_back, LOG_WARN, "Block Data Transfer, thou shall not do user bank transfer with writeback, r15=0x%x", r15);

			// Save current mode and enter user mode
			old_mode = cpsr & 0x1F;
			cpsr = (cpsr & ~0x1F) | User;
			RemapRegisters();

			// Mark that we switched to user mode
			switched_mode = true;
		}

		// Find the first register
		for (int i = 0; i < 16; i++)
		{
			if (instruction & (1 << i))
			{
				first_register = i;
				break;
			}
		}

		// Walk through the register list
		// TODO: Start with the first register (?)
		//       Remove code redundancy
		if (add_to_base)
		{
			for (int i = 0; i < 16; i++)
			{
				// Determine if the current register will be loaded/saved
				if (instruction & (1 << i))
				{
					// If instruction is pre-indexed we must update address beforehand
					if (pre_indexed)
					{
						address += 4;
					}

					// Perform the actual load / store operation
					if (load)
					{
						// Loading the base disables writeback
						// TODO: Check if it only disables writeback if the base is the first register in the list
						if (i == reg_base)
						{
							write_back = false;
						}

						// Load the register
						reg(i) = ReadWord(address);

						// If r15 is overwritten, the pipeline must be flushed
						if (i == 15)
						{
							// NOTICE: Here the bad return happens
							//         Check why this is happening
							// If the s bit is set a mode switch is performed
							if (s_bit)
							{
								// spsr_<mode> must not be copied to cpsr in user mode because user mode has not such a register
								ASSERT((cpsr & 0x1F) == User, LOG_ERROR, "Block Data Transfer is about to copy spsr_<mode> to cpsr, however we are in user mode, r15=0x%x", r15);

								cpsr = *pspsr;
								RemapRegisters();
							}
							flush_pipe = true;
						}
					}
					else
					{
						// When the base register is the first register in the list its original value is written
						if (i == first_register && i == reg_base)
						{
							WriteWord(address, old_address);
						}
						else
						{
							WriteWord(address, reg(i));
						}
					}

					// If instruction is not pre-indexed we must update address afterwards
					if (!pre_indexed)
					{
						address += 4;
					}

					// If the writeback is specified the base register must be updated after each register
					if (write_back)
					{
						reg(reg_base) = address;
					}
				}
			}
		}
		else
		{
			for (int i = 15; i >= 0; i--)
			{
				// Determine if the current register will be loaded/saved
				if (instruction & (1 << i))
				{
					// If instruction is pre-indexed we must update address beforehand
					if (pre_indexed)
					{
						address -= 4;
					}

					// Perform the actual load / store operation
					if (load)
					{
						// Loading the base disables writeback
						// TODO: Check if it only disables writeback if the base is the first register in the list
						if (i == reg_base)
						{
							write_back = false;
						}

						// Load the register
						reg(i) = ReadWord(address);

						// If r15 is overwritten, the pipeline must be flushed
						if (i == 15)
						{
							// If the s bit is set a mode switch is performed
							if (s_bit)
							{
								// spsr_<mode> must not be copied to cpsr in user mode because user mode has not such a register
								ASSERT((cpsr & 0x1F) == User, LOG_ERROR, "Block Data Transfer is about to copy spsr_<mode> to cpsr, however we are in user mode, r15=0x%x", r15);

								cpsr = *pspsr;
								RemapRegisters();
							}
							flush_pipe = true;
						}
					}
					else
					{
						// When the base register is the first register in the list its original value is written
						if (i == first_register && i == reg_base)
						{
							WriteWord(address, old_address);
						}
						else
						{
							WriteWord(address, reg(i));
						}
					}

					// If instruction is not pre-indexed we must update address afterwards
					if (!pre_indexed)
					{
						address -= 4;
					}

					// If the writeback is specified the base register must be updated after each register
					if (write_back)
					{
						reg(reg_base) = address;
					}
				}
			}
		}

		// If we switched mode it's time now to restore the previous mode
		if (switched_mode)
		{
			cpsr = (cpsr & ~0x1F) | old_mode;
			RemapRegisters();
		}
		break;
	}
	case ARM_12:
	{
		// ARM.12 Branch
		bool link { (instruction & (1 << 24)) == (1 << 24) };
		uint offset = instruction & 0xFFFFFF;
		if (offset & 0x800000)
		{
			offset |= 0xFF000000;
		}
		if (link)
		{
			reg(14) = r15 - 4;
		}
		r15 += offset << 2;
		flush_pipe = true;
		break;
	}
	case ARM_13:
		// ARM.13 Coprocessor data transfer
		LOG(LOG_ERROR, "Unimplemented coprocessor data transfer, r15=0x%x", r15);
		break;
	case ARM_14:
		// ARM.14 Coprocessor data operation
		LOG(LOG_ERROR, "Unimplemented coprocessor data operation, r15=0x%x", r15);
		break;
	case ARM_15:
		// ARM.15 Coprocessor register transfer
		LOG(LOG_ERROR, "Unimplemented coprocessor register transfer, r15=0x%x", r15);
		break;
		// TODO: Check wether swi is still executed when IRQ is disabled
		//       Implement HLE version
	case ARM_16:
		// ARM.16 Software interrupt
		if ((cpsr & IRQDisable) == 0)
		{
			r14_svc = r15 - 4;
			r15 = 0x8;
			flush_pipe = true;
			spsr_svc = cpsr;
			cpsr = (cpsr & ~0x1F) | SVC | IRQDisable;
			RemapRegisters();
		}
		break;
	}
#ifdef CPU_LOG
	cin.get();
#endif
}
}
