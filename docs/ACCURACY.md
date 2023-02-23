
# mGBA suite comparison

Comparison of NBA and other emulators on the [mGBA test suite](https://github.com/mgba-emu/suite) by endrift:

Testname      | Test Count | NanoBoyAdvance 1.6 | mGBA 0.10.1 |    VBA-M 2.1.5 | Ares v131 | SkyEmu V2  |
--------------|------------|--------------------|-------------|----------------|-----------|------------|
Memory        |       1552 |               1552 |        1552 |           1426 |      1552 |       1552 |
IO read       |        130 |                126 |         120 |            100 |       124 |        125 |
Timing        |       2020 |               2020 |        1768 |           1024 |      1570 |       2020 |
Timer         |        936 |                903 |         744 |            440 |       454 |        587 |
Timer IRQ     |         90 |                 90 |          70 |              8 |         0 |         90 |
Shifter       |        140 |                140 |         140 |            132 |       132 |        140 |
Carry         |         93 |                 93 |          93 |             93 |        93 |         93 |
Multiply Long |         72 |                 52 |          52 |             52 |        52 |         52 |
BIOS math     |        615 |                615 |         615 |            615 |       615 |        615 |
DMA tests     |       1256 |               1256 |        1232 |           1068 |      1212 |       1256 |
Misc Edge Case|     10[^1] |                  8 |           4 |              7 |         1 |          3 |
Layer Toggle  |          1 |               pass |        fail |           pass |      fail |       pass |
OAM Update    |          1 |               pass |        fail |           fail |      fail |       pass |

[^1]: Real hardware passes 9/10 tests.

Passing these tests does not necessarily translate to compatibility or *overall* accuracy.
But they give a good indication of how truthful an emulator is to hardware in certain areas, such as timing and DMA.

# Game compatibility

This list intends to document games that used to or currently have issues in NanoBoyAdvance and rely on peculiar edge-cases.
Some of these issues are minor visual bugs, that do not affect gameplay, others are game breaking.

## Known working

- Hello Kitty Collection: Miracle Fashion Maker (mid-instruction DMA affects open bus: [mGBA blog article](https://mgba.io/2020/01/25/infinite-loop-holy-grail/))
- Classic NES series (various edge-cases: [mGBA blog article](https://mgba.io/2014/12/28/classic-nes/))
- James Pond - Codename Robocod (requires IRQ delaying: see [ARM7TDMI manual](https://documentation-service.arm.com/static/5e8e1323fd977155116a3129?token=) about IRQ/FIQ latencies)
- Phantasy Star Collection (DMA from write-only IO returns open bus: https://github.com/nba-emu/NanoBoyAdvance/issues/109)
- Pinball Tycoon ([internal affine registers are updated conditionally](https://github.com/mgba-emu/mgba/issues/1668#issuecomment-925306878))
- Gunstar Heroes (requires limiting sprite render cycles: https://github.com/nba-emu/NanoBoyAdvance/issues/98)
- Golden Sun (Sprite attributes are updated regardless of transparency: https://github.com/nba-emu/NanoBoyAdvance/issues/99) (minor issue)
- Iridion 3D (BGX/BGY writes are latched at the start of the scanline: https://github.com/nba-emu/NanoBoyAdvance/issues/176) (minor issue)
- Gadget Racers (requires sub-scanline precision: https://github.com/nba-emu/NanoBoyAdvance/issues/230)
- Metal Max Kai II (original revision) (requires sub-scanline precision: https://github.com/nba-emu/NanoBoyAdvance/issues/229) (minor issue)
- Madden NFL 06 (requires VRAM access stalling: https://github.com/nba-emu/NanoBoyAdvance/issues/241)

## Known broken

- All Inside-cap visual novel ports (emulator detection gets set off: https://github.com/nba-emu/NanoBoyAdvance/issues/203)
