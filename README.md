## Project Description

Program that simulates a subset of the MIPS instruction set in C. 

### Supported Instructions

The instructions supported by this program are
`add`, `sub`, `and`, `or`, `addiu`, `slt`, `beq`, `lw`, `sw`, `j`, and `syscall`.
`syscall` only supports `syscall code 34` which prints integer in hex and
`syscall code 10` which terminates the program. See the 
[MIPS Instruction Set](https://www.dsi.unive.it/~gasparetto/materials/MIPS_Instruction_Set.pdf)
for the complete description of the MIPS instructions.

### Supported Registers

The registers supported by this program are
`$zero`, `$v0-$v1`, `$a0-$a3`, `$t0-$t9`, `$s0-$s7`. Their initial values are
`0x0`.

### Memory Description

The valid range of memory location is `0x00000000` to `0x000003E8`, inclusive.
All memory locations initially contain a value of `0x0`.

## How to Use

To compile the program, execute `gcc mips_in_C.c -o mips_in_C` in the CL and run
via `mips_in_C` in Windows, or `./mips_in_C` in Linux.

### Input

First, input a positive integer `N` which signifies the number of MIPS instructions
to be executed. Then, enter the `N` MIPS instructions in their own line. The
instruction must be encoded as a 32-bit signed integer. Refer to the
[MIPS Assembly/Instruction Formats](https://en.wikibooks.org/wiki/MIPS_Assembly/Instruction_Formats)
to know how the instructions must be encoded. For R-type instructions, bits of
fields that are not in use should be set to `0`.

### Output

The output will be represented as a 32-bit hexadecimal number.
