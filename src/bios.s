.arm
.align 4

@ Very much minimal GBA BIOS.
@ This file is only needed to emulate the IRQ handler. SWIs are handled in C++ code.
@ This file will not be compiled. See GBAMemory::hle_bios for the actual hex.

b excp_reset
nop
nop
nop
nop
nop
b excp_irq
nop

excp_reset:
mov pc, #0x8000000

excp_irq:
stmfd  sp!, {r0-r3, r12, lr}
mov    r0, #0x04000000
add    lr, pc, #0
ldr    pc, [r0, #-4]
ldmfd  sp!, {r0-r3, r12, lr}
subs   pc, lr, #4
