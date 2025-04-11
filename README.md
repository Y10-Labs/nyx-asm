# nyx-asm

## Key points in Architecture

* Register File: 16 general-purpose registers (r0-r15)
    - r15 is hardwired to zero
    - r14 is used for the halt instruction
* Memory System:
    - SBM: Shifting BRAMs
    - FBRAM: Framebuffer BRAM
* ALU (Arithmetic Logic Unit): Performs operations including:
    - Arithmetic: Addition, subtraction, multiplication
    - Logical: AND, OR, NOT
    - Shift operations: Barrel shift

    It does multiplication by default. Based on instruction it does other operations and selects the desired one.
* Instruction Decode unit: Decodes binary instructions into control signals
    - Handles opcode parsing, operand selection, and flag processing

## Files and Components

1. nyx.py (Assembler)

The assembler converts human-readable assembly code into binary machine code for execution.

```bash
python nyx.py input.asm -o output.bin [-pv] [-eX] [-s output.asm]
```
Options:

    - input: Assembly source file

    - -o/--output: Output binary file (default: a.out)

    - -pv/--parser_verbose: Enable verbose parser output

    - -eX/--extended_isa: Enable extended instruction set

    - -s/--asm_file: Output assembled instructions in human-readable format

2. nyx_ins.py (Instruction Encoding)

Defines the instruction format and encoding for the ISA.

Instruction Format:

    - 24-bit (3-byte) instructions

    - Bit fields:

        - Opcode: Bits 14-16

        - Flags: Bits 12-14

        - Source Register A: Bits 8-12

        - Source Register B: Bits 4-8

        - Destination Register: Bits 0-4

3. assembly_docs.txt (Assembly Language Reference)

Reference documentation for the assembly language.

## Instruction Set

### Basic Instructions

1. Arithmetic: add, sub, adi, mul

2. Logical: and, or, not

3. Memory: ldb (load from bram), str (store)

4. Control Flow: jmp, beq, ble

5. Miscellaneous: bsh (barrel shift), ldi (load immediate), hlt (halt)

### Extended Instructions (enabled with -eX)

1. mov: Move data between registers

2. cmp: Compare registers

3. test: Test register value

4. inc: Increment register

5. lds: Load from shifting brams

6. ldc: Load from framebuffer

These were added later. We can use default ones but these can make it a bit easier to code.

### Instruction Format

```
opcode[.flags] dest, src2, src1 ; comment
```

Flags:

    - f: Frame buffer access
    - s: Shifting bram access

Other NZC flags serve similar purpose as in ARM assembly. N is negative, Z is zero, C is carry.

Refer assembly-docs file for more details!

## Writing Assembly Code

### Examples

1. Basic Arithmetic:
```
add r1, r2, r3      ; r1 = r2 + r3
sub r4, r5, r6      ; r4 = r6 - r5
mul r7, r8, r9      ; r7 = r8 * r9
```

2. Using Immediates:

```
ldi r1, #10         ; Load immediate 10 into r1
bsh r2, #0x3, r3    ; Barrel shift r3 by 3 bits into r2
```

3. Control Flow:

```
.loop:
  sub r1, r1, r0    ; Decrement r1
  beq r0, .done     ; Branch to .done if result is zero
  jmp r0, .loop     ; Jump back to .loop
.done:
  hlt               ; Halt execution
```

4. Memory Operations:

```
ldb.f r1, r2, r3    ; Load from framebuffer
ldb.s r4, r5, r6    ; Load from static memory
str.f r7, r8, r9    ; Store to framebuffer
```

### Register Usage Convention

- r0: General purpose, often used for temporary storage

- r14: Reserved for halt instruction

- r15: Zero register (always contains 0)

## Common Errors and Debugging

* Undefined Label: Ensure all referenced labels are defined

* Invalid Register: Only r0-r15 are valid registers

* Immediate Value Overflow: Ensure - immediate values fit their bit limits

    - 4-bit for bsh (#0-15)

    - 10-bit for ldi (#0-1023)

* Missing Required Flags: ldb requires either f or s flag

## Limitations

- Program memory limited to 1024 instructions

- Register values are 32-bit unsigned integers

- No floating-point operations

- Limited interrupts and I/O capabilities
