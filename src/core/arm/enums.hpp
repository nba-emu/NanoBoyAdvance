/**
  * Copyright (C) 2017 flerovium^-^ (Frederic Meyer)
  *
  * This file is part of NanoboyAdvance.
  *
  * NanoboyAdvance is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  * 
  * NanoboyAdvance is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  * GNU General Public License for more details.
  * 
  * You should have received a copy of the GNU General Public License
  * along with NanoboyAdvance. If not, see <http://www.gnu.org/licenses/>.
  */

#pragma once

namespace GameBoyAdvance {
    enum Mode {
        MODE_USR = 0x10,
        MODE_FIQ = 0x11,
        MODE_IRQ = 0x12,
        MODE_SVC = 0x13,
        MODE_ABT = 0x17,
        MODE_UND = 0x1B,
        MODE_SYS = 0x1F
    };

    enum Bank {
        BANK_NONE,
        BANK_FIQ,
        BANK_SVC,
        BANK_ABT,
        BANK_IRQ,
        BANK_UND,
        BANK_COUNT
    };
    
    enum BankedRegister {
        BANK_R13 = 0,
        BANK_R14 = 1
    };
    
    enum SavedStatusRegister {
        SPSR_DEF = 0,
        SPSR_FIQ = 1,
        SPSR_SVC = 2,
        SPSR_ABT = 3,
        SPSR_IRQ = 4,
        SPSR_UND = 5,
        SPSR_COUNT = 6
    };
    
    enum Condition {
        COND_EQ = 0,
        COND_NE = 1,
        COND_CS = 2,
        COND_CC = 3,
        COND_MI = 4,
        COND_PL = 5,
        COND_VS = 6,
        COND_VC = 7,
        COND_HI = 8,
        COND_LS = 9,
        COND_GE = 10,
        COND_LT = 11,
        COND_GT = 12,
        COND_LE = 13,
        COND_AL = 14,
        COND_NV = 15
    };

    enum StatusMask {
        MASK_MODE  = 0x1F,
        MASK_THUMB = 0x20,
        MASK_FIQD  = 0x40,
        MASK_IRQD  = 0x80,
        MASK_VFLAG = 0x10000000,
        MASK_CFLAG = 0x20000000,
        MASK_ZFLAG = 0x40000000,
        MASK_NFLAG = 0x80000000
    };

    enum ExceptionVector {
        EXCPT_RESET     = 0x00,
        EXCPT_UNDEFINED = 0x04,
        EXCPT_SWI       = 0x08,
        EXCPT_PREFETCH_ABORT = 0x0C,
        EXCPT_DATA_ABORT     = 0x10,
        EXCPT_INTERRUPT      = 0x18,
        EXCPT_FAST_INTERRUPT = 0x1C
    };
    
    enum MemoryFlags {
        MEM_NONE   = 0,
        
        // Use for debug/internal accesses
        MEM_DEBUG  = (1 << 0),
        
        // (Non-)Sequential Accesses
        MEM_SEQ    = (1 << 1),
        MEM_NONSEQ = (1 << 2),
      
        MEM_SIGNED = (1 << 3),
        MEM_ROTATE = (1 << 4)
    };
}
