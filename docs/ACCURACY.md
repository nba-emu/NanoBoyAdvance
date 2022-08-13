
# mGBA suite comparison

Comparison of NBA and other emulators on the [mGBA test suite](https://github.com/mgba-emu/suite) by endrift:

Testname      | Test Count | NanoBoyAdvance | mGBA 0.9.3 | VBA-M 2.1.4 | Ares v128 | SkyEmu     |
--------------|------------|----------------|------------|-------------|-----------|------------|
Memory        |       1552 |           1552 |       1552 |        1338 |      1552 |       1552 |
IO read       |        123 |            123 |        114 |         100 |       123 |        123 |
Timing        |       2020 |           2020 |       1708 |         751 |      1570 |       2020 |
Timer         |        936 |            903 |        744 |         440 |       465 |        587 |
Timer IRQ     |         90 |             90 |         70 |           8 |         0 |         90 |
Shifter       |        140 |            140 |        140 |         132 |       132 |        140 |
Carry         |         93 |             93 |         93 |          93 |        93 |         93 |
Multiply Long |         72 |             52 |         52 |          52 |        52 |         52 |
BIOS math     |        615 |            615 |        615 |         615 |       615 |        615 |
DMA tests     |       1256 |           1256 |       1232 |        1032 |      1212 |       1256 |
Misc Edge Case|         10 |              8 |      7 - 8 |           7 |         1 |          3 |
Layer Toggle  |          1 |           pass |       fail |        pass |      fail |       pass |
OAM Update    |          1 |           pass |       fail |        fail |      fail |       pass |

Passing these tests does not necessarily translate to compatibility or *overall* accuracy.
But they give a good indication of how truthful an emulator is to hardware in certain areas, such as timing and DMA.

# Game compatibility

This list intends to document games that used to or currently have issues in NanoBoyAdvance and rely on peculiar edge-cases.
Some of these issues are minor visual bugs, that do not affect gameplay, others are game breaking.

## Known working

- Hello Kitty Collection: Miracle Fashion Maker (mid-instruction DMA affects open bus: [mGBA blog article](https://mgba.io/2020/01/25/infinite-loop-holy-grail/))
- Classic NES series (various edge-cases: [mGBA blog article](https://mgba.io/2014/12/28/classic-nes/))
- James Pond - Codename Robocod (requires IRQ delaying: see ARM7TDMI-**S** manual about IRQ/FIQ latencies)
- Phantasy Star Collection (DMA from write-only IO returns open bus: https://github.com/nba-emu/NanoBoyAdvance/issues/109)
- Pinball Tycoon ([internal affine registers are updated conditionally](https://github.com/mgba-emu/mgba/issues/1668#issuecomment-925306878))
- Gunstar Heroes (requires limiting sprite render cycles: https://github.com/nba-emu/NanoBoyAdvance/issues/98)
- Golden Sun (Sprite attributes are updated regardless of transparency: https://github.com/nba-emu/NanoBoyAdvance/issues/99) (minor issue)
- Iridion 3D (BGX/BGY writes are latched at the start of the scanline: https://github.com/nba-emu/NanoBoyAdvance/issues/176) (minor issue)

## Known broken

- Gadget Racers (requires sub-scanline precision: https://github.com/nba-emu/NanoBoyAdvance/issues/230)
- Metal Max Kai II (original revision) (requires sub-scanline precision: https://github.com/nba-emu/NanoBoyAdvance/issues/229) (minor issue)
- Madden NFL 06 (requires VRAM access stalling: https://github.com/nba-emu/NanoBoyAdvance/issues/241)
- All Inside-cap visual novel ports (emulator detection gets set off: https://github.com/nba-emu/NanoBoyAdvance/issues/203)
