typedef void (arm::*ThumbInstruction)(u16);
static const ThumbInstruction THUMB_TABLE[1024];

template <int imm, int type>
void Thumb1(u16 instruction);

template <bool immediate, bool subtract, int field3>
void Thumb2(u16 instruction);

template <int op, int reg_dest>
void Thumb3(u16 instruction);

template <int op>
void Thumb4(u16 instruction);

template <int op, bool high1, bool high2>
void Thumb5(u16 instruction);

template <int reg_dest>
void Thumb6(u16 instruction);

template <int op, int reg_offset>
void Thumb7(u16 instruction);

template <int op, int reg_offset>
void Thumb8(u16 instruction);

template <int op, int imm>
void Thumb9(u16 instruction);

template <bool load, int imm>
void Thumb10(u16 instruction);

template <bool load, int reg_dest>
void Thumb11(u16 instruction);

template <bool stackptr, int reg_dest>
void Thumb12(u16 instruction);

template <bool sub>
void Thumb13(u16 instruction);

template <bool pop, bool rbit>
void Thumb14(u16 instruction);

template <bool load, int reg_base>
void Thumb15(u16 instruction);

template <int cond>
void Thumb16(u16 instruction);

void Thumb17(u16 instruction);
void Thumb18(u16 instruction);

template <bool h>
void Thumb19(u16 instruction);
