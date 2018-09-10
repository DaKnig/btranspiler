# btranspiler

a translator (or transpiler) from brainfuck to gbz80 asm
## GOALS

- easily translatable- maybe translate it to gbz80 asm at some point?

- simplicity - it's not the best best of it's kind, even though it is the only one of it's kind at present ;-)

- assembler included? (to become a compiler) -edit - will probably be implemented as a seperate project

## implementation specs- the standard specs of the brainduck project

uses Game Boy Assembly Language Primer (see [Awesome-GBDev/boilerplates](https://github.com/gbdev/awesome-gbdev#boilerplates) )

the data line "wraps around"

the data line is 4096 bytes long

can go up to 512 loops "deep" - depending on the machine the code is compiled on, see `STACKSIZE`, easily changable
assumes the input is ok- if you're not sure, put it through bprecompiler
	
data line is in WRAM - range C000-CFFF
(I will probably add the ability to use the banked wram on CGB or banked SRAM on compatible carts. dont hold your breath, though...)
## progress
currently, it only copies the data to vram, doesn't actually display anything as the LCD is turned off

still need to mess with addr, mostly working...

implemented all the instructions excluding input

`.` printing is implemented by using my "terminal" (PLEASE suggest a better, more fitting name)

`,` input is in progress - will be implemented when it would be added to "terminal"

## optimizations
translates multiple consecutive adds/moves (`+`,`-`/`>`,`<`) into a single add/move instruction
