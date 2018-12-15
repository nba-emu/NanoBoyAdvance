/*
 * Copyright (C) 2018 Frederic Meyer. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

namespace ARM {

/* Processor Mode */
enum Mode {
    MODE_USR = 0x10,
    MODE_FIQ = 0x11,
    MODE_IRQ = 0x12,
    MODE_SVC = 0x13,
    MODE_ABT = 0x17,
    MODE_UND = 0x1B,
    MODE_SYS = 0x1F
};

/* Register Bank */
enum Bank {
    BANK_NONE = 0,
    BANK_FIQ  = 1,
    BANK_SVC  = 2,
    BANK_ABT  = 3,
    BANK_IRQ  = 4,
    BANK_UND  = 5,
    BANK_COUNT
};

/* Condition Codes */
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

} // namespace ARM