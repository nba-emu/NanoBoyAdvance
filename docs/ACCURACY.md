
Results for endrift's [Game Boy Advance Test Suite](https://github.com/mgba-emu/suite):

Testname      | Test Count | NanoBoyAdvance | mGBA 0.9.2 | VBA-M 2.1.4 | Ares v123 |
--------------|------------|----------------|------------|-------------|-----------|
Memory        |       1552 |           1552 |       1552 |        1338 |      1552 |
IO read       |        123 |            123 |        114 |         100 |       123 |
Timing        |       1660 |           1614 |       1560 |         692 |      1343 |
Timer         |        936 |            617 |        744 |         440 |       449 |
Timer IRQ     |         90 |             52 |         70 |           8 |         0 |
Shifter       |        140 |            140 |        140 |         132 |       132 |
Carry         |         93 |             93 |         93 |          93 |        93 |
Multiply Long |         72 |             52 |         52 |          52 |        52 |
BIOS math     |        615 |            615 |        615 |         615 |       615 |
DMA tests     |       1256 |           1256 |       1232 |        1032 |      1212 |
Misc Edge Case|         10 |          7 - 8 |      7 - 8 |           7 |         1 |
Layer Toggle  |          1 |           pass |       pass |        pass |      fail |
OAM Update    |          1 |           pass |       fail |        fail |      fail |

In addition NanoBoyAdvance passes the following tests:
- Aging cartridge by Nintendo (mGBA fails a single test, VBA-M and other emulators fail a lot more tests)
- [ARMWrestler](https://github.com/destoer/armwrestler-gba-fixed) by mic-
- [gba-tests](https://github.com/jsmolka/gba-tests) by jsmolka
