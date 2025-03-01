from nyx_ins import ins
from typing import List
from fixedint import UInt32 as uint32
from fixedint import UInt16 as uint16

class nyxEmulator:
    MUL_DELAY = 3
    LD_DELAY = 2
    JMP_DELAY = 2
    STR_DELAY = 2
    bits = 32
    UINT_MAX = 2**bits - 1
    INT_SIGN_BIT = 2**(bits - 1)

    def __init__(self, data: List[int]):
        self.framebuffer = [uint16(0)]*1024
        self.data = data
        self.registers = {
            "r0":uint32(0),
            "r1":uint32(0),
            "r2":uint32(0),
            "r3":uint32(0),
            "r4":uint32(0),
            "r5":uint32(0),
            "r6":uint32(0),
            "r7":uint32(0),
            "r8":uint32(0),
            "r9":uint32(0),
            "r10":uint32(0),
            "r11":uint32(0),
            "r12":uint32(0),
            "r13":uint32(0),
            "r14":uint32(0),
            "r15":uint32(0)
        }
        self.n = False
        self.z = False
        self.c = False

        self.s1_bus = uint32(0)
        self.s2_bus = uint32(0)

        self.dst_bus = uint32(0)
        self.alu_bus = [uint32(0)]*6
        self.op_sel_bus = [uint32(0)]*4

        self.mul_pipe = [(uint16(0), uint16(0))] * nyxEmulator.MUL_DELAY
        self.ld_pipe = []
        self.pc_pipe = []

        self.alu_sel = 0
        self.alu_bus = [uint32(0)]*8

        self.pc = 0
        self.program: List[ins] = []
        pass

    def load_program(self, prog: List[ins]):
        self.program = prog
    
    def barrel_shift(n, d):
        return uint32((n << d) % (1 << nyxEmulator.bits)) | uint32(n >> (nyxEmulator.bits - d))

    def registerFileOut(self, s1_sel, s2_sel):
        self.s1_bus = self.registers[s1_sel]
        self.s2_bus = self.registers[s2_sel]
    
    def registerFileIn(self, dst_sel):
        if dst_sel == 'r15':
            return
        self.registers[dst_sel] = self.dst_bus

        #op_sel[0] = ins[17] & ins[14]
        #op_sel[1] = ins[17] & ins[15]

    def ALU(self, useCarry):
        self.mul_pipe.insert(0, (self.s1_bus, self.s2_bus))
        a = self.mul_pipe.pop()
        self.op_sel_bus[1] = (a[0] * a[1])

        self.alu_bus[0] = int(self.s1_bus if self.alu_sel else ~self.s1_bus) + int(self.s2_bus) + (useCarry & self.c)
        if self.alu_bus[0] >= nyxEmulator.UINT_MAX:
            self.c = 1
        else:
            self.c = 0
        self.alu_bus[0b000] = uint32(self.alu_bus[0])
        self.alu_bus[0b001] = uint32(self.alu_bus[1])

        self.alu_bus[0b100] = nyxEmulator.barrel_shift(self.s1_bus, self.s2_bus)
        self.alu_bus[0b000]
        
        self.op_sel_bus[0] = self.alu_bus[self.alu_sel]

        self.n = (self.op_sel_bus[0] > nyxEmulator.UINT_MAX)
        self.z = (self.op_sel_bus[0] == 0)

        

        

