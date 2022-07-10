
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
Misc Edge Case|         10 |          4 - 5 |      7 - 8 |           7 |         1 |          3 |
Layer Toggle  |          1 |           pass |       fail |        pass |      fail |       pass |
OAM Update    |          1 |           pass |       fail |        fail |      fail |       pass |

