/*
 * Copyright (C) 2024 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

template <u32 instruction>
static constexpr auto GenerateHandlerARM() -> Handler32 {
  const u32 opcode = instruction & 0x0FFFFFFF;

  const bool pre = instruction & (1 << 24);
  const bool add = instruction & (1 << 23);
  const bool wb = instruction & (1 << 21);
  const bool load = instruction & (1 << 20);

  switch(opcode >> 26) {
  case 0b00:
    if(opcode & (1 << 25)) {
      // ARM.8 Data processing and PSR transfer ... immediate
      const bool set_flags = instruction & (1 << 20);
      const int  opcode = (instruction >> 21) & 0xF;

      if(!set_flags && opcode >= 0b1000 && opcode <= 0b1011) {
        const bool use_spsr = instruction & (1 << 22);
        const bool to_status = instruction & (1 << 21);

        return &ARM7TDMI::ARM_StatusTransfer<true, use_spsr, to_status>;
      } else {
        const int field4 = (instruction >> 4) & 0xF;

        return &ARM7TDMI::ARM_DataProcessing<true, static_cast<ARM7TDMI::DataOp>(opcode), set_flags, field4>;
      }
    } else if((opcode & 0xFF000F0) == 0x1200010) {
      // ARM.3 Branch and exchange
      // TODO: Some bad instructions might be falsely detected as BX.
      // How does HW handle this?
      return &ARM7TDMI::ARM_BranchAndExchange;
    } else if((opcode & 0x10000F0) == 0x0000090) {
      // ARM.1 Multiply (accumulate), ARM.2 Multiply (accumulate) long
      const bool accumulate = instruction & (1 << 21);
      const bool set_flags = instruction & (1 << 20);

      if(opcode & (1 << 23)) {
        const bool sign_extend = instruction & (1 << 22);

        return &ARM7TDMI::ARM_MultiplyLong<sign_extend, accumulate, set_flags>;
      } else {
        return &ARM7TDMI::ARM_Multiply<accumulate, set_flags>;
      }
    } else if((opcode & 0x10000F0) == 0x1000090) {
      // ARM.4 Single data swap
      const bool byte = instruction & (1 << 22);

      return &ARM7TDMI::ARM_SingleDataSwap<byte>;
    } else if((opcode & 0xF0) == 0xB0 ||
      (opcode & 0xD0) == 0xD0) {
      // ARM.5 Halfword data transfer, register offset
      // ARM.6 Halfword data transfer, immediate offset
      // ARM.7 Signed data transfer (byte/halfword)
      const bool immediate = instruction & (1 << 22);
      const int opcode = (instruction >> 5) & 3;

      return &ARM7TDMI::ARM_HalfwordSignedTransfer<pre, add, immediate, wb, load, opcode>;
    } else {
      // ARM.8 Data processing and PSR transfer
      const bool set_flags = instruction & (1 << 20);
      const int  opcode = (instruction >> 21) & 0xF;

      if(!set_flags && opcode >= 0b1000 && opcode <= 0b1011) {
        const bool use_spsr = instruction & (1 << 22);
        const bool to_status = instruction & (1 << 21);

        return &ARM7TDMI::ARM_StatusTransfer<false, use_spsr, to_status>;
      } else {
        const int field4 = (instruction >> 4) & 0xF;

        return &ARM7TDMI::ARM_DataProcessing<false, static_cast<ARM7TDMI::DataOp>(opcode), set_flags, field4>;
      }
    }
    break;
  case 0b01:
    // ARM.9 Single data transfer, ARM.10 Undefined
    if((opcode & 0x2000010) == 0x2000010) {
      return &ARM7TDMI::ARM_Undefined;
    } else {
      const bool immediate = ~instruction & (1 << 25);
      const bool byte = instruction & (1 << 22);

      return &ARM7TDMI::ARM_SingleDataTransfer<immediate, pre, add, byte, wb, load>;
    }
    break;
  case 0b10:
    // ARM.11 Block data transfer, ARM.12 Branch
    if(opcode & (1 << 25)) {
      return &ARM7TDMI::ARM_BranchAndLink<(opcode >> 24) & 1>;
    } else {
      const bool user_mode = instruction & (1 << 22);

      return &ARM7TDMI::ARM_BlockDataTransfer<pre, add, user_mode, wb, load>;
    }
    break;
  case 0b11:
    if(opcode & (1 << 25)) {
      if(opcode & (1 << 24)) {
        // ARM.16 Software interrupt
        return &ARM7TDMI::ARM_SWI;
      } else {
        // ARM.14 Coprocessor data operation
        // ARM.15 Coprocessor register transfer
      }
    } else {
      // ARM.13 Coprocessor data transfer
    }
    break;
  }

  return &ARM7TDMI::ARM_Undefined;
}
