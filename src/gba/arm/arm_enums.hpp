#pragma once

namespace GBA
{
    enum cpu_mode
    {
        MODE_USR = 0x10,
        MODE_FIQ = 0x11,
        MODE_IRQ = 0x12,
        MODE_SVC = 0x13,
        MODE_ABT = 0x17,
        MODE_UND = 0x1B,
        MODE_SYS = 0x1F
    };

    enum cpu_bank
    {
        BANK_NONE,
        BANK_FIQ,
        BANK_SVC,
        BANK_ABT,
        BANK_IRQ,
        BANK_UND,
        BANK_COUNT
    };

    enum cpu_bank_register
    {
        BANK_R13 = 0,
        BANK_R14 = 1
    };

    enum status_mask
    {
        MASK_MODE  = 0x1F,
        MASK_THUMB = 0x20,
        MASK_FIQD  = 0x40,
        MASK_IRQD  = 0x80,
        MASK_VFLAG = 0x10000000,
        MASK_CFLAG = 0x20000000,
        MASK_ZFLAG = 0x40000000,
        MASK_NFLAG = 0x80000000
    };

    enum exception_vector
    {
        EXCPT_RESET     = 0x00,
        EXCPT_UNDEFINED = 0x04,
        EXCPT_SWI       = 0x08,
        EXCPT_PREFETCH_ABORT = 0x0C,
        EXCPT_DATA_ABORT     = 0x10,
        EXCPT_INTERRUPT      = 0x18,
        EXCPT_FAST_INTERRUPT = 0x1C
    };

    enum saved_status_register
    {
        SPSR_DEF = 0,
        SPSR_FIQ = 1,
        SPSR_SVC = 2,
        SPSR_ABT = 3,
        SPSR_IRQ = 4,
        SPSR_UND = 5,
        SPSR_COUNT = 6
    };
}
