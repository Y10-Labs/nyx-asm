# assembler for lumen core arch

from nyx_ins import ins
from sys import exit
import argparse

parser = argparse.ArgumentParser()
parser.add_argument('input', type=str, help='input nyx microcode file')
parser.add_argument('-o', '--output', help='output binary file', default='a.out')
parser.add_argument('-pv', '--parser_verbose', help='enable verbose output from parser', action="store_true")
args = parser.parse_args()

asm = ''

isParserVerbose = args.parser_verbose
#isParserVerbose = True

inputFile = args.input
#inputFile = 'tester.nyx'
with open(inputFile) as f:
    asm = f.read()

# labels and their position
labels = {}
instruction_list = []

ins_count = 0
def parseline(asmline : str, line_no):
    global ins_count
    # case insensitive
    asmline = asmline.lower()
    comment_idx = asmline.find(';')
    if comment_idx >= 0:
        asmline = asmline[0: asmline.find(';')]
    # return if empty line
    if not asmline:
        return True
    
    # a label
    if asmline.startswith('.'):
        if not asmline.endswith(':'):
            print(f"Error: at line {line_no}: A label must be followed by :")
        else:
            label = asmline[:-1]
            if label in labels:
                print(f'Error: at line {line_no}: label redefinition')
                return False
            labels[label] = ins_count
            return True
    instruction = ins.create(asmline, line_no)
    if instruction is None:
        return False
    ins_count += 1
    instruction_list.append(instruction)
    return True

line_no = 1
failed = False
fail_count = 0
for asmline in asm.splitlines():
    hasFailed = not parseline(asmline, line_no)
    fail_count += hasFailed
    failed |= hasFailed
    line_no += 1
    pass

if failed:
    print(f'Encountered {fail_count} Fatal Errors. Stopping...')
    exit(1)

for instruction in instruction_list:
    if not instruction.isLabel:
        continue
    
    try:
        instruction.imm = labels[instruction.label]
    except KeyError:
        print(f'Error: at line {instruction.line_no}: no label {instruction.label} is defined!')
        fail_count += 1
        failed = True

if failed:
    print(f'Encountered {fail_count} Fatal Errors. Stopping...')
    exit(1)

if isParserVerbose:
    pinscnt = 0
    for instruction in instruction_list:
        bits = instruction.asInt()
        bitshex = "{0:#0{1}x}".format(bits,8)
        print(f'{pinscnt:02}:', instruction)
        pinscnt += 1

if ins_count > 1024:
    print("Warning: program memory limit extended!")
else:
    pval = 100 * (ins_count / 1024)
    print(f"Using {pval:.2f}% of available program memory")

with open(args.output, 'wb') as f:
    values = bytearray()
    for instruction in instruction_list:
        bits = instruction.asInt()
        values.append((bits >> 16) & 0xff)
        values.append((bits >> 8) & 0xff)
        values.append((bits >> 0) & 0xff)
    f.write(bytes(values))
print(f"Output written to {args.output}")
