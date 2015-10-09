# NanoboyAdvance

NanoboyAdvance is an experimental GBA emulator written using a mixture of c++ and c. Even though it's far away from being usable, it get's better and better with each commit. 

## Media

<img src="https://raw.githubusercontent.com/fredericmeyer/nanoboyadvance/master/screenshots/kirby_ingame.png" alt="kirby_ingame">

<b>Disclaimer:</b> I wanted to show you more pictures, but I though it would look ugly if I stack the pictures.

## Debugging

#### Great register overview
<img src="https://raw.githubusercontent.com/fredericmeyer/nanoboyadvance/master/screenshots/nsh.png" alt="nsh">

NanoboyAdvance has a great console interface based debugger which has been designed both for reverse engineers and emulator developers.

#### Commands
```
nanoboy ~# help
help: displays this text
showregs: displays all cpu registers
setreg [register] [value]: sets the value of a general purpose register
bpa [address]: sets an arm mode execution breakpoint
bpt [address]: sets a thumb mode execution breakpoint
bpr [offset]: breakpoint on read
bpw [offset]: breakpoint on write
bpsvc [bios_call]: halt *after* executing a swi instruction with the given call number
memdump [offset] [length]: displays memory in a formatted way
setmemb [offset] [byte]: writes a byte to a given memory address
setmemh [offset] [hword]: writes a hword to a given memory address
setmemw [offset] [word]: writes a word to a given memory address
dumpstck [count]: displays a given amount of stack entries
frame: run until the first line of the next frame gets rendered
c: continues execution
```
The commands may change in future. <b>Also I will add disassembling capability</b>.

## About me
I'm a 19 years old computer engineering student from Germany who loves computers and magical lowlevel stuff. Is there really anything more to tell? Computers are awesome. They're perfect, humans not.
