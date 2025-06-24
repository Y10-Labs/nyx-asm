# nyx-asm with SIMD

## Key points in Architecture

* Register File:
    - 16 general-purpose registers(r0-r15).
    - r0 is hardwired to 0.
    - SIMD(Single Instruction, Multiple Data) is available. Adjacent registers are taken as one group i.e., (r2, r3), (r4, r5),.., (r14, r15).
    - Consider (r2, r3) as X1, (r4, r5) as X2 and so on till (r14, r15) as X7.
* Memory System:
    - FBRAM: Framebuffer BRAM which stores output of rasterizer temporarily.
    - Normal BRAM: We get input data to the rasterizer from this BRAM.
* ALU (Arithmetic Logic Unit): Performs operations including:
    - Arithmetic: Addition, subtraction, multiplication.
    - Shift: Shift left/right.

## Instruction Set Architecture

### Instruction Set

1. Arithmetic:
    - add: Normally adds the source registers and stores result in destination register. If SIMD enabled, it add both registers respectively in Xi and Xj to destination Xk. For example, (r2, r3) + (r4, r5) = (r6, r7) then r6 = r2 + r4 and r7 = r3 + r5.
    - sub: Normally calculates difference between the source registers and stores result in destination register. If SIMD enabled, it subtracts both registers respectively in Xi and Xj to destination Xk. For example, (r2, r3) - (r4, r5) = (r6, r7) then r6 = r2 - r4 and r7 = r3 - r5.
    - sh: shift by immediate
    - imul16: Multiplies the source registers and stores result in destination register.

2. General:
    - jmp: Used to jump to different instruction.
    - pop_fifo: Pops the stack from input BRAM to the rasterizer. Loads the data to the destination register.
    - push_fd: Pushes the output of rasterizer to the frame buffer.
    - zcmpw: Compares the z values and modifies the pixel data only if it was closer to the screen.

### Instruction Encoding

- 4 bits destination
- 4 bits source1, source2
- 10 bits immediate
- 1 bit for inc/dec. This is used in combination with add. If it is inc then ri + 1, if it is dec then ri + 2.
- 3 bits N, Z, U flags.
    - 100: Negative
    - 010: Zero
    - xx1: Unconditional
    - 110: Negative or Zero
    - 000: Positive
- General opcode i.e., jmp, pop_fifo, push_fd, zcmpw, etc are 4 bit long.
- Arithmetic opcode i.e., add, sub, sh, imul16 are 2 bit long.

- The total instruction encoding is done in 32 bits as described above starting from MSB to LSB in the same order.

- Two operations are implemented with one instruction. One general and one arithmetic instruction per cycle. Also here in the general opcode if first two bits are zero then it can act as arithmetic. So, we can do two operations in one instruction cycle.