from functools import reduce
import re
from typing import List

class ins:
    NO_PIPELINE = 0b0000
    PIPELINED = 0b1000

    ALU_ADDER = 0b0000
    ALU_OPT = 0b0100

    DONE_BIT = 1 << 7

    POS_OPCODE = 14
    POS_FLAGS = 12
    POS_SRCA = 8
    POS_SRCB = 4
    POS_IMM = 4
    POS_DST = 0

    MODE_DEFAULT = 0
    MODE_IMM = 1

    NOP = 0
    ZERO_REG = 'r15'

    # maps register names to it's hex codes
    REG_BITS = {
        "r0":0x0,
        "r1":0x1,
        "r2":0x2,
        "r3":0x3,
        "r4":0x4,
        "r5":0x5,
        "r6":0x6,
        "r7":0x7,
        "r8":0x8,
        "r9":0x9,
        "r10":0xa,
        "r11":0xb,
        "r12":0xc,
        "r13":0xd,
        "r14":0xe,
        "r15":0xf
    }

    INS_EX_LST = [
        'mov',
        'cmp'
    ]

    # maps instructions to their code
    INS_BITS = {
        "add": NO_PIPELINE | ALU_ADDER | 0b00,
        "sub": NO_PIPELINE | ALU_ADDER | 0b01,
        "bsh": NO_PIPELINE | ALU_OPT | 0b00,
        "and": NO_PIPELINE | ALU_OPT | 0b01,
        "or" : NO_PIPELINE | ALU_OPT | 0b10,
        "not": NO_PIPELINE | ALU_OPT | 0b11,

        "ldb": PIPELINED | 0b001,
        "mul": PIPELINED | 0b010,
        "jmp": PIPELINED | 0b101,
        "beq": PIPELINED | 0b000,
        "ble": PIPELINED | 0b110,
        "str": PIPELINED | 0b100,

        "ldi": 0b1011,
        "hlt": 0b1111,
    }

    FLAGS_BITS = {
        "f":0b00,
        "s":0b10,
        "c":0b01,
    }

    def __init__(self):
        self.opcode: str = 'add'
        self.src_a: str = 'r15'
        self.src_b: str | int = 'r15'
        self.dst: str = 'r15'
        self.imm: int = 0
        self.flags: str | None = None
        self.mode: int = self.MODE_DEFAULT
        self.isLabel: bool = False
        self.label: str | None = None
        self.line_no: int = 0
        self.comment: str = ''

    def sanitize_register(register, line_no:int):
        if register not in ins.REG_BITS.keys():
            print(f'Error: at line {line_no}: {register} is not a valid register name')
            return None
        return register

    def check_eX_ins(self, oprands_lst:List[str], line_no):
        # take 2/3 arguments

        error_prefix = f'Error: at line {line_no}'
        
        if self.opcode == 'nop':
            self.opcode = 'add'
            self.src_a = ins.ZERO_REG
            self.src_b = ins.ZERO_REG
            self.dst = ins.ZERO_REG
            self.flags = None
            self.comment = 'nop'
            return True
        
        if self.opcode == 'cmp':
            self.opcode = 'sub'
            
            if len(oprands_lst) != 2:
                print(f"{error_prefix}: cmp must have 2 oprands")
                return False
            
            self.src_a = ins.sanitize_register(oprands_lst[1], line_no)
            self.src_b = ins.sanitize_register(oprands_lst[0], line_no)
            if not self.src_a:
                print(f"{error_prefix}: invalid register {oprands_lst[1]}")
                return False
            if not self.src_b:
                print(f"{error_prefix}: invalid register {oprands_lst[0]}")
                return False
            self.dst = ins.ZERO_REG
            self.comment = f'cmp {self.src_b}, {self.src_a}'
            return True
        
        if self.opcode == 'test':
            self.opcode = 'add'

            if len(oprands_lst) != 1:
                print(f"{error_prefix}: test must have a oprands")
                return False
            
            self.src_a = ins.sanitize_register(oprands_lst[0], line_no)
            if not self.src_a:
                print(f"{error_prefix}: invalid register {oprands_lst[0]}")
                return False
            self.src_b = ins.ZERO_REG
            self.dst = ins.ZERO_REG
            self.comment = f'test {self.src_a}'
            return True

        if self.opcode in ['beq', 'jmp', 'blt']:
            # only the label is present
            if len(oprands_lst) == 1:
                if not oprands_lst[0].startswith('.'):
                    print(f"{error_prefix}: the first operand for {self.opcode} must be a label (start with '.')")
                    return False
                self.dst = ins.ZERO_REG
                self.label = oprands_lst[0]
                self.isLabel = True
                self.mode = ins.MODE_IMM
                return True

        if self.opcode == 'mov':
            # only the label is present
            if len(oprands_lst) != 2:
                print(f"{error_prefix}: mov must have 2 oprands")
                return False
            self.opcode = 'add'
            self.src_a = ins.sanitize_register(oprands_lst[0], line_no)
            self.src_b = ins.ZERO_REG
            self.dst = ins.sanitize_register(oprands_lst[1], line_no)
            if not self.src_a:
                print(f"{error_prefix}: invalid register {oprands_lst[0]}")
                return False
            if not self.dst:
                print(f"{error_prefix}: invalid register {oprands_lst[1]}")
                return False
            self.comment = f'mov {self.dst}, {self.src_a}'
            return True

        return False

    def check_and_set_ins(self, oprands_lst:List[str], line_no):
        """
        Validates the operands based on the instruction type and sets the class variables.
        
        Args:
            oprands_lst (list): List of operand strings
        
        Returns:
            boolean: True(if successfull), False(if not successfull)
        """
        # Define valid opcodes by their operand count
        three_oprands = ['add', 'sub', 'mul', 'or', 'not', 'and', 'bsh', 'ldb', 'str']
        label_ins = ['jmp', 'beq', 'blt']
        zero_oprands = ['hlt']

        self.line_no = line_no
        
        # Special case instructions
        immediate_ins = {'bsh': 4, 'ldi': 10}  # opcode: bit_size

        imm10_ins = ['ldi', 'jmp', 'beq', 'blt']
        
        # set correct ins mode
        if self.opcode in imm10_ins:
            self.mode = ins.MODE_IMM

        error_prefix = f'Error: at line {line_no}'
        
        # Check for errors based on opcode type
        if self.opcode == "hlt" and len(oprands_lst) > 0:
            print(f"{error_prefix}: 'hlt' instruction should have no operands")
            return False
        
        if self.opcode == "hlt":
            self.mode = ins.MODE_IMM
            self.imm = ins.DONE_BIT
            self.dst = 'r14'
            return True
        
        # Check operand count
        expected_count = 3 if self.opcode in three_oprands else 2
        if len(oprands_lst) != expected_count:
            print(f"{error_prefix}: '{self.opcode}' expects {expected_count} operands, got {len(oprands_lst)}")
            return False
        
        # Check flags for different instructions
        if self.opcode in imm10_ins and self.flags is not None:
            print(f"Warning: at line {line_no}: flags '{self.flags}' specified for 'ldi' instruction are ignored")
        
        # Special case for ldb - must have either f or s flag
        if self.opcode == 'ldb':
            if len(self.flags) > 1:
                print(f"{error_prefix}: 'ldb' only supports 'f' or 's' flag!")
                return False
            if not ('f' in self.flags or 's' in self.flags):
                print(f"{error_prefix}: 'ldb' instruction must have either 'f' or 's' flag, got '{self.flags}'")
                return False
        
        self.dst = ins.sanitize_register(oprands_lst[0], line_no)
        if not self.dst:
            return False

        # Handle different instruction types
        if self.opcode in label_ins:
            
            # Check if second operand is a label (starts with .)
            if not oprands_lst[1].startswith('.'):
                print(f"{error_prefix}: second operand for {self.opcode} must be a label (start with '.')")
                return False
            
            self.label = oprands_lst[1]
            self.isLabel = True
            return True
        
        # Handle immediate values for bsh and ldi
        elif self.opcode in immediate_ins:
            if not oprands_lst[1].startswith('#'):
                print(f"{error_prefix}: second operand for {self.opcode} must be an immediate value (start with '#')")
                return False
            
            # Parse the immediate value
            imm_val = oprands_lst[1][1:]  # Remove the # prefix
            
            try:
                # Handle different number formats
                if imm_val.startswith('0x'):
                    self.imm = int(imm_val, 16)
                elif imm_val.startswith('0b'):
                    self.imm = int(imm_val, 2)
                else:
                    self.imm = int(imm_val, 10)
                
                # Check if immediate value fits in the specified bit width
                bit_width = immediate_ins[self.opcode]
                if self.imm < 0 or self.imm >= (1 << bit_width):
                    print(f"{error_prefix}: The number for {self.opcode} must be a {bit_width}-bit number (0-{(1 << bit_width) - 1})")
                    return False
                    
            except ValueError:
                print(f"{error_prefix}: invalid immediate value format '{imm_val}'")
                return False
            
            # For bsh, we need to set the source register
            if self.opcode == 'bsh':
                self.src_b = self.imm
                self.src_a = ins.sanitize_register(oprands_lst[2], line_no)
                if not self.src_a:
                    return False
        
        # Handle standard 3-operand instructions
        else:
            self.src_b = ins.sanitize_register(oprands_lst[1], line_no)
            if not self.src_b:
                return False
            
            if expected_count == 3:
                self.src_a = ins.sanitize_register(oprands_lst[2], line_no)
                if not self.src_a:
                    return False
        
        # Validate flags for all instructions
        if self.flags is not None and self.opcode != 'ldb':
            # Check each character is a valid flag
            valid_flags = {'f', 's', 'c'}
            for flag in self.flags:
                if flag not in valid_flags:
                    print(f"{error_prefix}: invalid flag '{flag}' in '{self.flags}'")
                    return False
        
        return True

    # returns False if error occured
    def fromStr(self, asmline:str, line_no, useExIns=False):
        # ins[.(fscnz)] reg, (reg(s2), imm, label), reg(s1)

        asmline = asmline.strip()
        find_brk = asmline.find(' ')
        if find_brk < 0:
            if asmline not in ['hlt', 'nop']:
                print(f"Error: at line {line_no}: Too few oprands for instruction!")
                return False
            elif useExIns:
                self.opcode = asmline
                if self.check_eX_ins([], line_no):
                    return True
            self.opcode = 'hlt'
            return self.check_and_set_ins([], line_no)
        operation = asmline[0:find_brk]
        oprands = asmline[find_brk:].strip()
        self.flags = None

        if '.' in operation:
            self.opcode = operation.split('.')[0]
            self.flags = operation.split('.')[1]
            if not all(c in self.FLAGS_BITS.keys() for c in self.flags):
                print(f"Error: at line {line_no}: Flags can only be any of f,s or c")
                return False
        else:
            self.opcode = operation

        
        if self.opcode not in ins.INS_BITS.keys():
            if (useExIns and self.opcode not in ins.INS_EX_LST) or not useExIns:
                print(f'Error: at line {line_no}: no instruction named {self.opcode}.')
                return False

        oprands_lst = [s.strip() for s in oprands.split(',')]

        if useExIns:
            isExIns = self.check_eX_ins(oprands_lst, line_no)
            if isExIns:
                return True

        return self.check_and_set_ins(oprands_lst, line_no)
    
    def create(asmline, line_no, useExIns=False):
        inst_val = ins()
        ret_val = inst_val.fromStr(asmline, line_no, useExIns)
        if not ret_val:
            return None
        return inst_val

    def asInt(self):
        opcode = ins.INS_BITS[self.opcode] << ins.POS_OPCODE
        dst = ins.REG_BITS[self.dst] << ins.POS_DST
        flags = 0
        if self.flags:
            flags = reduce(lambda x,y: x | y, map(lambda x: ins.FLAGS_BITS[x], self.flags))
        flags <<= ins.POS_FLAGS

        if self.mode == ins.MODE_DEFAULT:
            src_a = ins.REG_BITS[self.src_a] << ins.POS_SRCA
            
            src_b = 0
            if isinstance(self.src_b, int):
                src_b = self.src_b
            else:
                src_b = ins.REG_BITS[self.src_b] << ins.POS_SRCB
            
            return opcode | flags | src_a | src_b | dst
        elif self.mode == ins.MODE_IMM:
            imm10 = self.imm << ins.POS_IMM
            return opcode | flags | imm10 | dst
        
        return ins.NOP
    
    def asASM(self):
        comment = ''
        flags = ''
        if self.flags:
            flags = f'.{self.flags}'
        if self.comment:
            comment = f' ; {self.comment}'

        if self.isLabel:
            return f"{self.opcode}{flags} {self.dst}, {self.label}" + comment
        
        if self.opcode == 'hlt':
            return f"hlt"
        
        # For immediate instructions (bsh, ldi)
        if isinstance(self.src_b, int) or self.mode == self.MODE_IMM:
            imm_repr = f"#{self.imm}"
            
            if self.opcode == 'bsh':
                return f"{self.opcode}{flags} {self.dst}, {imm_repr}, {self.src_a}" + comment
            else:  # ldi
                return f"{self.opcode}{flags} {self.dst}, {imm_repr}" + comment
        
        # For standard 3-operand instructions
        #if self.opcode in ['add', 'mul', 'or', 'not', 'ldb', 'st']:
        #    return f"Instruction(opcode='{self.opcode}', flags='{self.flags}', dst='{self.dst}', src_b='{self.src_b}', src_a='{self.src_a}')"
        
        # Fallback for any other case
        return f"{self.opcode}{flags} {self.dst}, {self.src_b}, {self.src_a}" + comment

    def __repr__(self):
        """
        Returns a string representation of the instruction.
        
        Returns:
            str: String representation of the instruction
        """
        bitstring = "{0:#0{1}x}".format(self.asInt(),8)

        if self.isLabel:
            return f"{self.opcode}(bits={bitstring}, flags={self.flags}, dst={self.dst}, label={self.label}(ins:{self.imm}))"
        
        if self.opcode == 'hlt':
            return f"hlt(bits={bitstring})"
        
        # For immediate instructions (bsh, ldi)
        if isinstance(self.src_b, int) or self.mode == self.MODE_IMM:
            imm_repr = f"#{self.imm}"
            # Show in different formats for better visibility
            if self.imm > 9:
                imm_repr += f" (0x{self.imm:x}, 0b{self.imm:b})"
            
            if self.opcode == 'bsh':
                return f"{self.opcode}(bits={bitstring}, flags={self.flags}, dst={self.dst}, shift={imm_repr}, src_a={self.src_a})"
            else:  # ldi
                return f"{self.opcode}(bits={bitstring}, dst={self.dst}, imm10={imm_repr})"
        
        # For standard 3-operand instructions
        #if self.opcode in ['add', 'mul', 'or', 'not', 'ldb', 'st']:
        #    return f"Instruction(opcode='{self.opcode}', flags='{self.flags}', dst='{self.dst}', src_b='{self.src_b}', src_a='{self.src_a}')"
        
        # Fallback for any other case
        return f"{self.opcode}(bits={bitstring}, flags={self.flags}, dst={self.dst}, src_b={self.src_b}, src_a={self.src_a})"