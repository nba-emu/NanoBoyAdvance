/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#ifndef ARM_INCLUDE_GUARD
#error "This file cannot be included regularely!"
#endif

template <typename T, T Begin,  class Func, T ...Is>
static constexpr void static_for_impl( Func &&f, std::integer_sequence<T, Is...> ) {
  ( f( std::integral_constant<T, Begin + Is>{ } ),... );
}

template <typename T, T Begin, T End, class Func >
static constexpr void static_for( Func &&f ) {
  static_for_impl<T, Begin>( std::forward<Func>(f), std::make_integer_sequence<T, End - Begin>{ } );
}

static constexpr OpcodeTable16 EmitAll16() {
  ARM7TDMI::OpcodeTable16 lut = {};
  
  static_for<std::size_t, 0, 1024>([&](auto i) {
    lut[i] = EmitHandler16<i<<6>();
  });
  return lut;
}

static constexpr OpcodeTable32 EmitAll32() {
  ARM7TDMI::OpcodeTable32 lut = {};

  static_for<std::size_t, 0, 4096>([&](auto i) {
    lut[i] = EmitHandler32<((i & 0xFF0) << 16) | ((i & 0xF) << 4)>();
  });
  return lut;
}

template <std::uint16_t instruction>
static constexpr Instruction16 EmitHandler16() {
  // THUMB.1 Move shifted register
  if ((instruction & 0xF800) < 0x1800) {
    const auto opcode  = (instruction >> 11) & 3;
    const auto offset5 = (instruction >>  6) & 0x1F;
    
    return &ARM7TDMI::Thumb_MoveShiftedRegister<opcode, offset5>;
  }
  
  // THUMB.2 Add/subtract
  if ((instruction & 0xF800) == 0x1800) {
    const bool immediate = (instruction >> 10) & 1;
    const bool subtract  = (instruction >>  9) & 1;
    const auto field3 = (instruction >>  6) & 7;
    
    return &ARM7TDMI::Thumb_AddSub<immediate, subtract, field3>;
  }
  
  // THUMB.3 Move/compare/add/subtract immediate
  if ((instruction & 0xE000) == 0x2000) {
    const auto opcode = (instruction >> 11) & 3;
    const auto rD = (instruction >>  8) & 7;
    
    return &ARM7TDMI::Thumb_Op3<opcode, rD>;
  }
  
  // THUMB.4 ALU operations
  if ((instruction & 0xFC00) == 0x4000) {
    const auto opcode = (instruction >> 6) & 0xF;
    
    return &ARM7TDMI::Thumb_ALU<opcode>;
  }
  
  // THUMB.5 Hi register operations/branch exchange
  if ((instruction & 0xFC00) == 0x4400) {
    const auto opcode = (instruction >> 8) & 3;
    const bool high1  = (instruction >> 7) & 1;
    const bool high2  = (instruction >> 6) & 1;
    
    return &ARM7TDMI::Thumb_HighRegisterOps_BX<opcode, high1, high2>;
  }
  
  // THUMB.6 PC-relative load
  if ((instruction & 0xF800) == 0x4800) {
    const auto rD = (instruction >>  8) & 7;
    
    return &ARM7TDMI::Thumb_LoadStoreRelativePC<rD>;
  }
  
  // THUMB.7 Load/store with register offset
  if ((instruction & 0xF200) == 0x5000) {
    const auto opcode = (instruction >> 10) & 3;
    const auto rO = (instruction >>  6) & 7;
    
    return &ARM7TDMI::Thumb_LoadStoreOffsetReg<opcode, rO>;
  }
  
  // THUMB.8 Load/store sign-extended byte/halfword
  if ((instruction & 0xF200) == 0x5200) {
    const auto opcode = (instruction >> 10) & 3;
    const auto rO = (instruction >>  6) & 7;
    
    return &ARM7TDMI::Thumb_LoadStoreSigned<opcode, rO>;
  }
  
  // THUMB.9 Load store with immediate offset
  if ((instruction & 0xE000) == 0x6000) {
    const auto opcode  = (instruction >> 11) & 3;
    const auto offset5 = (instruction >>  6) & 0x1F;
    
    return &ARM7TDMI::Thumb_LoadStoreOffsetImm<opcode, offset5>;
  }
  
  // THUMB.10 Load/store halfword
  if ((instruction & 0xF000) == 0x8000) {
    const bool load = (instruction >> 11) & 1;
    const auto offset5 = (instruction >>  6) & 0x1F;
    
    return &ARM7TDMI::Thumb_LoadStoreHword<load, offset5>;
  }
  
  // THUMB.11 SP-relative load/store
  if ((instruction & 0xF000) == 0x9000) {
    const bool load = (instruction >> 11) & 1;
    const auto rD = (instruction >>  8) & 7;
    
    return &ARM7TDMI::Thumb_LoadStoreRelativeToSP<load, rD>;
  }
  
  // THUMB.12 Load address
  if ((instruction & 0xF000) == 0xA000) {
    const bool use_r13 = (instruction >> 11) & 1;
    const auto rD = (instruction >>  8) & 7;
    
    return &ARM7TDMI::Thumb_LoadAddress<use_r13, rD>;
  }
  
  // THUMB.13 Add offset to stack pointer
  if ((instruction & 0xFF00) == 0xB000) {
    const bool subtract = (instruction >> 7) & 1;
    
    return &ARM7TDMI::Thumb_AddOffsetToSP<subtract>;
  }
  
  // THUMB.14 push/pop registers
  if ((instruction & 0xF600) == 0xB400) {
    const bool load  = (instruction >> 11) & 1;
    const bool pc_lr = (instruction >>  8) & 1;
    
    return &ARM7TDMI::Thumb_PushPop<load, pc_lr>;
  }
  
  // THUMB.15 Multiple load/store
  if ((instruction & 0xF000) == 0xC000) {
    const bool load = (instruction >> 11) & 1;
    const auto rB = (instruction >>  8) & 7;
    
    return &ARM7TDMI::Thumb_LoadStoreMultiple<load, rB>;
  }
  
  // THUMB.16 Conditional Branch
  if ((instruction & 0xFF00) < 0xDF00) {
    const auto condition = (instruction >> 8) & 0xF;
    
    return &ARM7TDMI::Thumb_ConditionalBranch<condition>;
  }
  
  // THUMB.17 Software Interrupt
  if ((instruction & 0xFF00) == 0xDF00) {
    return &ARM7TDMI::Thumb_SWI;
  }
  
  // THUMB.18 Unconditional Branch
  if ((instruction & 0xF800) == 0xE000) {
    return &ARM7TDMI::Thumb_UnconditionalBranch;
  }
  
  // THUMB.19 Long branch with link
  if ((instruction & 0xF000) == 0xF000) {
    const auto opcode = (instruction >> 11) & 1;
    
    return &ARM7TDMI::Thumb_LongBranchLink<opcode>;
  }

  return &ARM7TDMI::Thumb_Undefined;
}

template <std::uint32_t instruction>
static constexpr Instruction32 EmitHandler32() {
  const std::uint32_t opcode = instruction & 0x0FFFFFFF;
  
  const bool pre  = instruction & (1 << 24);
  const bool add  = instruction & (1 << 23);
  const bool wb   = instruction & (1 << 21);
  const bool load = instruction & (1 << 20);

  switch (opcode >> 26) {
  case 0b00:
    if (opcode & (1 << 25)) {
      // ARM.8 Data processing and PSR transfer ... immediate
      const bool set_flags = instruction & (1 << 20);
      const int  opcode = (instruction >> 21) & 0xF;

      if (!set_flags && opcode >= 0b1000 && opcode <= 0b1011) {
        const bool use_spsr  = instruction & (1 << 22);
        const bool to_status = instruction & (1 << 21);

        return &ARM7TDMI::ARM_StatusTransfer<true, use_spsr, to_status>;
      } else {
        const int field4 = (instruction >> 4) & 0xF;

        return &ARM7TDMI::ARM_DataProcessing<true, opcode, set_flags, field4>;
      }
    } else if ((opcode & 0xFF000F0) == 0x1200010) {
      // ARM.3 Branch and exchange
      // TODO: Some bad instructions might be falsely detected as BX.
      // How does HW handle this?
      return &ARM7TDMI::ARM_BranchAndExchange;
    } else if ((opcode & 0x10000F0) == 0x0000090) {
      // ARM.1 Multiply (accumulate), ARM.2 Multiply (accumulate) long
      const bool accumulate = instruction & (1 << 21);
      const bool set_flags  = instruction & (1 << 20);

      if (opcode & (1 << 23)) {
        const bool sign_extend = instruction & (1 << 22);

        return &ARM7TDMI::ARM_MultiplyLong<sign_extend, accumulate, set_flags>;
      } else {
        return &ARM7TDMI::ARM_Multiply<accumulate, set_flags>;
      }
    } else if ((opcode & 0x10000F0) == 0x1000090) {
      // ARM.4 Single data swap
      const bool byte = instruction & (1 << 22);

      return &ARM7TDMI::ARM_SingleDataSwap<byte>;
    } else if ((opcode & 0xF0) == 0xB0 ||
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

      if (!set_flags && opcode >= 0b1000 && opcode <= 0b1011) {
        const bool use_spsr  = instruction & (1 << 22);
        const bool to_status = instruction & (1 << 21);

        return &ARM7TDMI::ARM_StatusTransfer<false, use_spsr, to_status>;
      } else {
        const int field4 = (instruction >> 4) & 0xF;

        return &ARM7TDMI::ARM_DataProcessing<false, opcode, set_flags, field4>;
      }
    }
    break;
  case 0b01:
    // ARM.9 Single data transfer, ARM.10 Undefined
    if ((opcode & 0x2000010) == 0x2000010) {
      return &ARM7TDMI::ARM_Undefined;
    } else {
      const bool immediate = ~instruction & (1 << 25);
      const bool byte = instruction & (1 << 22);

      return &ARM7TDMI::ARM_SingleDataTransfer<immediate, pre, add, byte, wb, load>;
    }
    break;
  case 0b10:
    // ARM.11 Block data transfer, ARM.12 Branch
    if (opcode & (1 << 25)) {
      return &ARM7TDMI::ARM_BranchAndLink<(opcode >> 24) & 1>;
    } else {
      const bool user_mode = instruction & (1 << 22);

      return &ARM7TDMI::ARM_BlockDataTransfer<pre, add, user_mode, wb, load>;
    }
    break;
  case 0b11:
    if (opcode & (1 << 25)) {
      if (opcode & (1 << 24)) {
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