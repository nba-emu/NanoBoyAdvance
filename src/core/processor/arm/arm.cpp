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

#include <cstring>
#include "arm.hpp"

namespace Core {

    ARM::ARM() {
        ARM::reset();
    }

    void ARM::reset()  {
        ctx.pipe.index = 0;

        std::memset(ctx.reg,  0, sizeof(ctx.reg));
        std::memset(ctx.bank, 0, sizeof(ctx.bank));
        ctx.cpsr   = MODE_SYS;
        ctx.p_spsr = &ctx.spsr[SPSR_DEF];

        refillPipeline();
    }

    inline ARM::Bank ARM::modeToBank(ARM::Mode mode) {
        switch (mode) {
        case MODE_USR:
        case MODE_SYS:
            return BANK_NONE;
        case MODE_FIQ:
            return BANK_FIQ;
        case MODE_IRQ:
            return BANK_IRQ;
        case MODE_SVC:
            return BANK_SVC;
        case MODE_ABT:
            return BANK_ABT;
        case MODE_UND:
            return BANK_UND;
        default:
            return BANK_NONE;
        }
    }

    // Based on mGBA (endrift's) approach to banking.
    // https://github.com/mgba-emu/mgba/blob/master/src/arm/arm.c
    void ARM::switchMode(ARM::Mode new_mode) {
        ARM::Mode old_mode = static_cast<Mode>(ctx.cpsr & MASK_MODE);

        if (new_mode == old_mode) {
            return;
        }

        ARM::Bank new_bank = modeToBank(new_mode);
        ARM::Bank old_bank = modeToBank(old_mode);

        if (new_bank != old_bank) {
            if (new_bank == BANK_FIQ || old_bank == BANK_FIQ) {
                int old_fiq_bank = old_bank == BANK_FIQ;
                int new_fiq_bank = new_bank == BANK_FIQ;

                // save general purpose registers to current bank.
                ctx.bank[old_fiq_bank][2] = ctx.reg[8];
                ctx.bank[old_fiq_bank][3] = ctx.reg[9];
                ctx.bank[old_fiq_bank][4] = ctx.reg[10];
                ctx.bank[old_fiq_bank][5] = ctx.reg[11];
                ctx.bank[old_fiq_bank][6] = ctx.reg[12];

                // restore general purpose registers from new bank.
                ctx.reg[8]  = ctx.bank[new_fiq_bank][2];
                ctx.reg[9]  = ctx.bank[new_fiq_bank][3];
                ctx.reg[10] = ctx.bank[new_fiq_bank][4];
                ctx.reg[11] = ctx.bank[new_fiq_bank][5];
                ctx.reg[12] = ctx.bank[new_fiq_bank][6];
            }

            // save SP and LR to current bank.
            ctx.bank[old_bank][BANK_R13] = ctx.reg[13];
            ctx.bank[old_bank][BANK_R14] = ctx.reg[14];

            // restore SP and LR from new bank.
            ctx.reg[13] = ctx.bank[new_bank][BANK_R13];
            ctx.reg[14] = ctx.bank[new_bank][BANK_R14];

            ctx.p_spsr = &ctx.spsr[new_bank];
        }

        ctx.cpsr = (ctx.cpsr & ~MASK_MODE) | (u32)new_mode;
    }

    void ARM::signalIRQ() {
        if (~ctx.cpsr & MASK_IRQD) {
            bool thumb = ctx.cpsr & MASK_THUMB;

            // store return address in r14<irq>
            ctx.bank[BANK_IRQ][BANK_R14] = ctx.r15 - (thumb ? 4 : 8) + 4;

            // save program status and switch mode
            ctx.spsr[SPSR_IRQ] = ctx.cpsr;
            switchMode(MODE_IRQ);
            ctx.cpsr = (ctx.cpsr & ~MASK_THUMB) | MASK_IRQD;

            // jump to exception vector
            ctx.r15 = EXCPT_INTERRUPT;
            refillPipeline();
        }
    }
}
