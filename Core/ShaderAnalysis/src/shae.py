# Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
#  
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#  
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#  
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

from enum import Enum
from itertools import tee
import argparse
import sys
import collections
import operator
import re

def pairwise (iterable):
    a, b = tee (iterable)
    next (b, None)
    return zip (a, b)

class InstructionFlags(Enum):
    Default = 0
    Branch = 1      # Conditional branch, so both the target and the next
                    # instruction are successors
    Jump = 2        # Unconditional branch, so only the target is a valid
                    # successor

class OpCode:
    def __init__(self, name, operandsWritten, operandsRead, instructionClass, flags = 0, cycleCount = None):
        self.__name = name
        self.__operandsWritten = frozenset (operandsWritten)
        self.__operandsRead = frozenset (operandsRead)
        self.__instructionClass = instructionClass
        self.__cycleCount = cycleCount
        self.__flags = flags

    @property
    def IsBranch(self):
        return self.__flags & InstructionFlags.Branch.value

    @property
    def IsJump(self):
        return self.__flags & InstructionFlags.Jump.value

    @property
    def Name(self):
        return self.__name
    
    @property
    def Class(self):
        return self.__instructionClass

    @Class.setter
    def Class(self, value):
        self.__instructionClass = value

    @property
    def Written(self):
        return self.__operandsWritten

    @property
    def Read(self):
        return self.__operandsRead

    @property
    def ReadWritten(self):
        return self.__operandsRead.intersection (self.__operandsWritten)

class RegisterSet:
    def __init__(self, read, written):
        self.__read = read
        self.__written = written

    @property
    def Read(self):
        return self.__read

    @property
    def Written(self):
        return self.__written

    def __str__ (self):
        return 'Written: {}, read: {}'.format (len (self.__written), len (self.__read))

def MergeRegisterSets(registerSets):
    read = frozenset().union (*[rs.Read for rs in registerSets])
    written = frozenset().union (*[rs.Written for rs in registerSets])

    return RegisterSet (read, written)

class Instruction:
    '''An instruction consists of an op-code, the arguments passed to it, and
    an optional label if the instruction is a jump target.'''
    def __init__ (self, binCode, opcode, args, label = None):
        import gcn
        self.__opcode = gcn.opcodes.get (opcode, OpCode (opcode, {}, {}, gcn.InstructionClass.Unknown, 0, None))
        self.__args = args
        self.__label = label

        self.__usedVGPRs = self.__ComputeUsedVGPRs()

        # Some instructions have 2 possible encodings: VOP2 or VOP3. For these instructions, check
        # the highest bit of binary code to find out the exact encoding type.
        if self.__opcode.Class == gcn.InstructionClass.VOP2 or self.__opcode.Class == gcn.InstructionClass.VOP3:
            hiCode = binCode.split(' ')[0]
            if (int(hiCode, 16) & 0x80000000) == 0:
                self.__opcode.Class = gcn.InstructionClass.VOP2
            else:
                self.__opcode.Class = gcn.InstructionClass.VOP3

    def SetLabel(self, label):
        assert label is not None
        self.__label = label

    @property
    def OpCode(self):
        return self.__opcode

    def __ComputeUsedVGPRs (self):
        class VGPRSet:
            '''Helper class to add register ranges and filter out non-vector registers.'''
            def __init__ (self):
                self.__used = set ()

            def Add (self, reg):
                if reg.startswith ('abs('):
                    reg = reg [4:-1]
                elif reg.startswith ('-'):
                    reg = reg [1:]

                if reg[0] != 'v' or reg == 'vcc' or reg.startswith ('vmcnt'):
                    return

                if reg[1] == '[':
                    # vector register range
                    s, e = reg [2:-1].split (':')
                    s = int (s)
                    e = int (e)
                    for i in range(s, e+1):
                        self.__used.add (i)
                else:
                    self.__used.add (int (reg[1:]))

            def Get(self):
                return self.__used

            def __str__(self):
                return str(self.__used)

        read = VGPRSet ()
        written = VGPRSet ()
        
        for i, arg in enumerate (self.__args):
            if i in self.__opcode.ReadWritten:
                written.Add (arg)
                read.Add (arg)
            elif i in self.__opcode.Written:
                written.Add (arg)
            else:
                read.Add (arg)

        return RegisterSet (read.Get (), written.Get ())

    def __str__ (self):
        return self.ToString ()

    def ToString (self, includeLabel=True):
        if self.__label and includeLabel:
            return 'label_{}: {} {}'.format (self.__label, self.__opcode.Name, ', '.join (self.__args))
        else:
            return '{} {}'.format (self.__opcode.Name, ', '.join (self.__args))

    @property
    def Label(self):
        return self.__label

    @property
    def UsedVGPRs (self):
        return self.__usedVGPRs

    def IsBranch(self):
        return self.__opcode.IsBranch

    def IsJump(self):
        return self.__opcode.IsJump

    def GetBranchTarget(self):
        return self.__args [0]

    def GetJumpTarget(self):
        return self.__args [0]

class BasicBlock:
    def __init__(self, label, instructions, order):
        self.__instructions = instructions
        self.__label = label

        self.__order = order

        self.__successors = []
        self.__predecessors = []

    @property
    def Empty(self):
        return len(self.__instructions) == 0

    @property
    def IsEntry(self):
        return self.__label == 'entry'

    @property
    def IsExit(self):
        return self.__label == 'exit'

    def GetUsedVGPRs(self):
        return MergeRegisterSets ([i.UsedVGPRs for i in self.__instructions])

    @property
    def Label(self):
        return self.__label

    def Link (self, otherBlock):
        self.__successors.append (otherBlock)
        otherBlock.__predecessors.append (self)

    @property
    def Successors (self):
        return self.__successors

    @property
    def Predecessors (self):
        return self.__predecessors

    @property
    def Order(self):
        return self.__order

    @property
    def Instructions(self):
        return self.__instructions

    def __str__ (self):
        return ('{} ({} instructions)'.format (self.__label, len (self.__instructions)))

class IsaReader:
    def GetInstructions (self):
        return []

class _BaseIsaReader(IsaReader):
    def GetLines(self, inputStream):
        lines = []
        for line in inputStream:
            line = line.strip ()
            if line.startswith(';'):
                continue
            comment = line.find ('//')
            commentStr = ""
            instItems = []
            if comment != -1:
                # Split line into 2 parts: instruction & comment.
                commentStr = line [comment+2 :]
                line = line [:comment]
            if line:
                # Parse the comment. It should contain the instruction binary code.
                binCodeStr = self.GetBinEncodingText(commentStr)
                # Add the binary code and other instruction parts into "instItems"
                if binCodeStr != "":
                    instItems.append(binCodeStr)
                instItems.extend(line.split())
                lines.append (instItems)

        return lines

    def ReadInstructions (self, lines):
        nextLabel = None
        result = []
        inShader = False

        for lineno, line in enumerate (lines):
            # shader <foo>, asic(VI), type(PS) and similar are ignored here
            if line[0].startswith ('asic') or line[0].startswith ('type') or line[0].startswith ('@kernel'):
                continue
            elif line[0] == 'shader':
                continue
            elif not ' ' in line[0] and line[0][-1] == ':':
                nextLabel = line[0][:-1]
                continue

            binCode = line [0]
            opCode  = line [1]
            args = []

            # Join everything after the opcode and separate at the comma again
            for param in ' '.join (line [2:]).split (','):
                paramElements = param.split ()
                if paramElements:
                    args.append (paramElements[0])

            result.append (Instruction (binCode, opCode, args, nextLabel))
            nextLabel = None

        return result

    # Tries to find binary code text in the "comment" string:
    # 000000000130: D1190201 00000100
    #              `--- bin code ---'
    def GetBinEncodingText(self, comment):
        result = ""
        if  comment == "":
            return result
        foundBinCode = re.search(r' *[0-9a-fA-F]+: *([0-9a-fA-F]{8})( *[0-9a-fA-F]{8})?', comment)
        if foundBinCode:
            result = comment[comment.find(':')+1 :].lstrip(' ')
        return result

class ShaderAnalyzerIsaReader(_BaseIsaReader):
    def GetInstructions (self, inputStream):
        lines = self.GetLines (inputStream)
        
        inShader = False

        shaderLines = []
        
        for lineno, line in enumerate (lines):
            # we skip everything until we find a line starting with
            # 'shader', and then we'll stop once we read 'end'
            # This enables us to read dumps which contain arbitrary metadata
            # before/after the shader
            if line [0] == 'shader' or line [0] == 'end':
                if line [0] == 'shader' and not inShader:
                    inShader = True
                    continue
                elif line [0] == 'end' and inShader:
                    break
                else:
                    raise RuntimeError ("Mismatched shader/end in line {}".format (lineno))

            if inShader:
                shaderLines.append (line)

        return self.ReadInstructions (shaderLines)

class HSAILIsaReader(_BaseIsaReader):
    def GetInstructions(self, inputStream):
        lines = self.GetLines (inputStream)

        inShader = False

        shaderLines = []

        for lineno, line in enumerate (lines):
            if (line [0] == 'Disassembly' and line [1] == 'for') or line [0] == 'end':
                if line [0] == 'Disassembly' and line [1] == 'for':
                    inShader = True
                    continue
                elif line [0] == 'end' and inShader:
                    break
                else:
                    raise RuntimeError ("Mismatched Disassembly for/end in line {}".format (lineno))

            if inShader:
                shaderLines.append (line)

        return self.ReadInstructions (shaderLines)

class ShaderDumpIsaReader(_BaseIsaReader):
    def GetInstructions (self, inputStream):
        lines = self.GetLines (inputStream)

        inShader = False

        shaderLines = []
        
        for lineno, line in enumerate (lines):
            if ' '.join (line) == '; -------- Disassembly --------------------':
                inShader = True
                continue
            elif line[0] == 'end' and inShader:
                break
            
            if inShader:
                shaderLines.append (line)

        return self.ReadInstructions (shaderLines)

class RawIsaReader(_BaseIsaReader):
    def GetInstructions (self, inputStream):
        lines = self.GetLines (inputStream)
        
        result = []
        for line in lines:
            result.append (list (map (str.lower, line)))
            if line[0].lower () == 's_endpgm':
                break
        
        return self.ReadInstructions (result)

def GetIsaReader (inputStream, isaFormat):
    if isaFormat == 'auto':
        start = inputStream.buffer.peek (64)
        if start.startswith (b'AMD Kernel Code for'):
            return HSAILIsaReader ()
        elif start.startswith (b';--------'):
            return ShaderDumpIsaReader ()
        else:
            return ShaderAnalyzerIsaReader ()
    elif isaFormat == 'HSAIL':
        return HSAILIsaReader ()
    elif isaFormat == 'ShaderAnalyzer':
        return ShaderAnalyzerIsaReader ()
    elif isaFormat == 'ShaderDump':
        return ShaderDumpIsaReader ()
    elif isaFormat == 'raw':
        return RawIsaReader ()
    else:
        raise RuntimeError ("Unsupported input format: '{}'".format (isaFormat))

def LoadIsa (inputStream, isaFormat):
    reader = GetIsaReader (inputStream, isaFormat)
    instructions = reader.GetInstructions (inputStream)

    basicBlocks = []
    basicBlockMap = {}

    currentBlockLabel = 'entry'
    currentBlockInstructions = []
    startOnNextBlock = True

    # Build basic blocks
    for i, instruction in enumerate (instructions):
        label = instruction.Label

        if label or startOnNextBlock:
            block = BasicBlock (currentBlockLabel, currentBlockInstructions, len(basicBlocks))
            basicBlocks.append (block)

            assert block.Label not in basicBlockMap
            basicBlockMap [block.Label] = block

            currentBlockInstructions = []

            if startOnNextBlock and label is None:
                currentBlockLabel = 'basic_block_{}'.format (len(basicBlocks))
                startOnNextBlock = False
            else:
                currentBlockLabel = label
                startOnNextBlock = False

        if instruction.IsBranch () or instruction.IsJump ():
            startOnNextBlock = True
        currentBlockInstructions.append (instruction)

    # Add last block if some instructions are left
    if currentBlockInstructions:
        block = BasicBlock (currentBlockLabel, currentBlockInstructions, len(basicBlocks))
        basicBlocks.append (block)
        basicBlockMap [block.Label] = block

    # Terminal block, we call it exit because there is no jump to its label
    block = BasicBlock ('exit', [], len(basicBlocks))
    basicBlocks.append (block)

    assert block.Label not in basicBlockMap
    basicBlockMap [block.Label] = block

    for pair in pairwise(basicBlocks):
        instructions = pair [0].Instructions

        if instructions:
            if instructions[0].Label != pair[0].Label:
                instructions[0].SetLabel (pair[0].Label)

            if instructions[-1].IsBranch ():
                # Link branch target
                pair [0].Link (basicBlockMap [instructions[-1].GetBranchTarget ()])
            elif instructions[-1].IsJump ():
                # Link jump target and exit -- nothing else to link here
                pair [0].Link (basicBlockMap [instructions[-1].GetJumpTarget ()])
                continue

        # Link unless the last instruction was an jump (see above)
        pair [0].Link (pair [1])

    return basicBlocks

def DumpCFGDot(input, output, isaFormat):
    """Write out the control-flow-graph, with each node being a basic block."""
    basicBlocks = LoadIsa (input, isaFormat)
    def FormatBlockContent(block, compact):
        if compact:
            return '{} instructions\\l'.format (len (block.Instructions))
        else:
            return ''.join([i.ToString (includeLabel=False) + '\\l' for i in block.Instructions])

    links = []
    output.write ('digraph {\nnode [shape=box]\n')
    for block in basicBlocks:
        shape = ''
        if block.IsEntry or block.IsExit:
            shape = 'style=rounded'
        output.write ('"n_{0}" [ label= "{0}\\n{1}" {2}]\n'.format (block.Label, FormatBlockContent (block, args.compact), shape))

        for succ in block.Successors:
            links.append ((block.Label, succ.Label))

    for link in links:
        output.write ('"n_{}":s -> "n_{}";\n'.format (link [0], link [1]))
    output.write ('}\n')

def DumpBasicBlockVGPRUsage(input, output, isaFormat):
    """Write out the VGPRs used per basic block, without considering
    inter-block dependencies."""
    basicBlocks = LoadIsa (input, isaFormat)

    for bb in basicBlocks:
        vgpr = bb.GetUsedVGPRs ()
        output.write ('Block: {0: >16} | Read: {1: >4} | Written: {2: >4}\n'.format (bb.Label, len(vgpr.Read), len(vgpr.Written)))

class PICFGNode:
    """A node in the per-instruction CFG tree. Each PICFG node has one
    instruction at most."""
    def __init__(self, instruction, block, inBlockOrder):
        self.__instruction = instruction
        self.__block = block
        self.__successors = []
        self.__predecessors = []
        self.__in = set ()
        self.__out = set ()
        self.__order = inBlockOrder

    @property
    def Use(self):
        '''Return the registers used (read) in this node.'''
        if self.__instruction:
            return self.__instruction.UsedVGPRs.Read
        else:
            return set ()

    @property
    def Def(self):
        '''Return the registers defined (written) in this node.'''
        if self.__instruction:
            return self.__instruction.UsedVGPRs.Written
        else:
            return set ()

    @property
    def In(self):
        """Registers that are live at the incoming edges to this node. Update
        using UpdateIn()."""
        return self.__in

    @property
    def Out(self):
        """Registers that are live at the outgoing edges of this node. Update
        using UpdateOut()."""
        return self.__out

    def UpdateOut(self):
        """Updates the set of live registers at the outgoing edges.

        :return: True if the set of registers has changed in this update."""
        oldOutSize = len (self.__out)
        self.__out = self.__out.union (*[succ.In for succ in self.__successors])

        return oldOutSize != len (self.__out)

    def UpdateIn(self):
        """Updates the set of live registers at the incoming edges.

        :return: True if the set of registers has changed in this update."""
        oldInSize = len (self.__in)
        self.__in = set.union (self.Use, self.Out - self.Def)

        return oldInSize != len (self.__in)

    @property
    def Block(self):
        """Get a reference to the original enclosing block."""
        return self.__block

    @property
    def Order(self):
        """Get an order number within the enclosing basic block."""
        return self.__order

    @property
    def Successors(self):
        """Get the successor nodes."""
        return self.__successors

    @property
    def Predecessors(self):
        """Get the predecessor nodes."""
        return self.__predecessors

    @property
    def Instruction(self):
        """Get the contained instruction."""
        return self.__instruction

    def Link(self, other):
        self.__successors.append (other)
        other.__predecessors.append (self)

def LowerBasicBlockCFGtoPICFG(basicBlock, visitedBasicBlocks={}):
    if basicBlock.Label in visitedBasicBlocks:
        return visitedBasicBlocks [basicBlock.Label]

    blockNodes = [PICFGNode(instruction,basicBlock,i) for i,instruction in enumerate (basicBlock.Instructions)]

    if not blockNodes:
        blockNodes = [PICFGNode(None, basicBlock, 0)] # Create an empty node for the entry/exit basic block

    for pair in pairwise(blockNodes):
        pair[0].Link (pair [1])

    visitedBasicBlocks [basicBlock.Label] = blockNodes

    for successor in basicBlock.Successors:
        nodes = LowerBasicBlockCFGtoPICFG(successor, visitedBasicBlocks)
        if nodes:
            blockNodes [-1].Link (nodes [0])

    return blockNodes

class GatherOrder(Enum):
    Successors = 0
    Predecessors = 1

def _GatherDepthFirst (node, order):
    # Gather nodes by visiting the successors/predecessors
    # Do not use recursion here as the graphs can be really deep
    visitStack = [node]
    visited = set ()
    result = []

    while visitStack:
        currentNode = visitStack.pop ()

        if id(currentNode) in visited:
            continue
        else:
            visited.add (id (currentNode))
            result.append (currentNode)
            if order == GatherOrder.Successors:
                for successor in currentNode.Successors:
                    visitStack.append (successor)
            else:
                for predecessor in currentNode.Predecessors:
                    visitStack.append (predecessor)

    return result

def GatherSuccessorsDepthFirst (node):
    return _GatherDepthFirst (node, GatherOrder.Successors)

def GatherPredecessorsDepthFirst(node):
    return _GatherDepthFirst(node, GatherOrder.Predecessors)

def DumpInstructionVGPRUsage(input, output, isaFormat, summaryOnly):
    '''Dump the live registers for every instruction.'''
    basicBlocks = LoadIsa (input, isaFormat)

    entry = LowerBasicBlockCFGtoPICFG (basicBlocks [0])
    nodes = GatherSuccessorsDepthFirst (entry[0])

    entryNode = entry[0]

    # Find the exit node by looking for a node without successors
    exitNode = None
    for node in nodes:
        if not node.Successors:
            exitNode = node
            break

    dfs = GatherPredecessorsDepthFirst (exitNode)

    while True:
        updates = []
        for node in dfs:
            updates.append (node.UpdateOut ())
            updates.append (node.UpdateIn ())

        if any (updates):
            continue
        else:
            break

    # Enumerate the PICFG in original order
    enumeratedNodes = [{'sortKey':(node.Block.Order,node.Order),'node':node} for node in dfs]
    enumeratedNodes = sorted(enumeratedNodes, key=operator.itemgetter('sortKey'))
    instructionNodes = [node['node'] for node in enumeratedNodes]

    def _Max(s):
        if s:
            return max(s)
        else:
            return 0

    print('Legend:', file=output)
    print('  \':\' means that the register is kept alive, while it is not actively being used by the current instruction', file=output)
    print('  \'^\' means that the current instruction writes to the register', file=output)
    print('  \'v\' means that the current instruction reads from the register', file=output)
    print('  \'x\' means that the current instruction both reads from the register and writes to it', file=output)
    print(' \'Rn\': Number of live registers\n', file=output)

    maxVGPR = 0
    highestVGPR = 0
    for node in instructionNodes:
        maxVGPR = max (len (set.union (node.In, node.Out)), maxVGPR)
        highestVGPR = max (highestVGPR, _Max (node.In), _Max (node.Out))
 
    print(' Line | Rn  | {:{width}} | Instruction'.format('Reg State', width=highestVGPR+1), file=output)
    print('--------------------------------------------------------------------------------------------------------------------------', file=output)

    if not summaryOnly:
        for lineNumber, node in enumerate (instructionNodes):
            if node.Instruction is None:
                # entry/exit node
                continue

            liveVGPR = set.union (node.In, node.Out)
            readVGPR = node.Use
            writtenVGPR = node.Def

            vgprStr = ''
            for i in range (highestVGPR+1):
                isRead = i in readVGPR
                isWritten = i in writtenVGPR
                if isRead and isWritten:
                    vgprStr += 'x'
                elif isRead:
                    vgprStr += 'v'
                elif isWritten:
                    vgprStr += '^'
                elif i in liveVGPR:
                    vgprStr += ':'
                else:
                    vgprStr += ' '

            print ('{0: >5} | {1: >3} | {2: <9} | {3}'.format (lineNumber, len (liveVGPR), vgprStr, node.Instruction), file=output)

        print (file=output)
    # +1, if we only use VGPR 0 then the number of allocated ones is 1
    print ('Maximum # VGPR used {0: >3}, # VGPR allocated: {1: >3}'.format (maxVGPR, highestVGPR+1), file=output)

def DumpPICFGDot(input, output, isaFormat):
    '''Dump a per-instruction control flow graph.'''
    basicBlocks = LoadIsa (input, isaFormat)

    entry = LowerBasicBlockCFGtoPICFG (basicBlocks [0])
    nodes = GatherSuccessorsDepthFirst (entry[0])

    links = []
    output.write ('digraph {\nnode [shape=box]\n')
    for node in nodes:
        shape = ''
        if node.Instruction:
            label = node.Instruction.ToString (includeLabel=False)
        else:
            if node.Block.IsEntry or node.Block.IsExit:
                shape = 'style=rounded'
            if node.Block.IsEntry:
                label = 'Entry'
            elif node.Block.IsExit:
                label = 'Exit'

        output.write ('"{}" [ label= "{}\\l" {}]\n'.format (id (node), label, shape))

        for succ in node.Successors:
            links.append ((node, succ))

    for link in links:
        output.write ('"{}":s -> "{}";\n'.format (id (link [0]), id (link [1])))
    output.write ('}\n')

def DumpOpcodeHistogram (input, output, isaFormat, args):
    import math
    basicBlocks = LoadIsa (input, isaFormat)

    usage = {}

    for block in basicBlocks:
        for instruction in block.Instructions:
            opCodeName = instruction.OpCode.Name

            if args.group == 'operand-size':
                lastUnderscore = opCodeName.rfind ('_')
                if lastUnderscore == -1:
                    opCodeName = 'unknown'
                else:
                    opType = opCodeName [lastUnderscore+1:]

                    # check if starts with i, u, or f, otherwise it's something
                    # like _x2
                    if opType [0] in {'i', 'u', 'f', 'b'} and opType [1].isdigit ():
                        opCodeName = opType
                    else:
                        opCodeName = 'unknown'
            elif args.group == 'instruction-class':
                opCodeName = instruction.OpCode.Class.name

            usage [opCodeName] = usage.get (opCodeName, 0) + 1

    longestOpcodeName = max (map (len, usage.keys ())) + 2

    totalInstructionCount = sum (usage.values ())
    instructionLength = int (math.ceil (math.log10 (totalInstructionCount)) + 1)
    
    formatString = '{{0:{0}}} {{1:>{1}}} ({{2:6.2f}} %)\n'.format (longestOpcodeName, instructionLength)

    for key, value in sorted (usage.items (),key=operator.itemgetter (1), reverse=True):
        output.write (formatString.format (key, value, value / totalInstructionCount * 100))

__version__ = '1.2.4'

if __name__=='__main__':
    parser = argparse.ArgumentParser(description='SHAE shader analyzer')
    parser.add_argument('--version', action='version', version='%(prog)s {}'.format (__version__))
    parser.add_argument ('-f', '--format', choices=['HSAIL', 'ShaderAnalyzer', 'ShaderDump', 'raw', 'auto'],
        help="The input format.", default='auto')

    subparsers = parser.add_subparsers (help='Subcommands', dest='command')
    dump_dot = subparsers.add_parser ('dump-bb-cfg', help='Dump the basic-block CFG to a dot file.')
    dump_dot.add_argument ('input', type=argparse.FileType ('r', encoding='UTF-8'), default=sys.stdin)
    dump_dot.add_argument ('output', type=argparse.FileType ('w', encoding='UTF-8'), default=sys.stdout)
    dump_dot.add_argument ('-c', '--compact', action='store_true', help='Compact output', default=False)

    dump_picfg_dot = subparsers.add_parser ('dump-pi-cfg', help='Dump the per-instruction CFG to a dot file.')
    dump_picfg_dot.add_argument ('input', type=argparse.FileType ('r', encoding='UTF-8'), default=sys.stdin)
    dump_picfg_dot.add_argument ('output', type=argparse.FileType ('w', encoding='UTF-8'), default=sys.stdout)

    dump_bb_vgpr = subparsers.add_parser ('dump-bb-vgpr', help='Dump per-basic-block VGPR usage.')
    dump_bb_vgpr.add_argument ('input', type=argparse.FileType ('r', encoding='UTF-8'), default=sys.stdin)
    dump_bb_vgpr.add_argument ('output', type=argparse.FileType ('w', encoding='UTF-8'), default=sys.stdout)

    dump_vgpr = subparsers.add_parser ('analyse-liveness', help='Write per-instruction VGPR liveness and usage.',
        aliases=['analyze-liveness'])
    dump_vgpr.add_argument ('input', type=argparse.FileType ('r', encoding='UTF-8'), default=sys.stdin)
    dump_vgpr.add_argument ('output', type=argparse.FileType ('w', encoding='UTF-8'), default=sys.stdout)
    dump_vgpr.add_argument ('-s', '--summary', action='store_true', help='Write only the summary line', default=False)

    dump_opcode_histogram = subparsers.add_parser ('opcode-histogram', help='Write a histogram showing how often each opcode has been used.')
    dump_opcode_histogram.add_argument ('input', type=argparse.FileType ('r', encoding='UTF-8'), default=sys.stdin)
    dump_opcode_histogram.add_argument ('output', type=argparse.FileType ('w', encoding='UTF-8'), default=sys.stdout)
    dump_opcode_histogram.add_argument ('-g', '--group', choices=['operand-size', 'instruction-class'],
        help="Group instructions by type")
   
    args = parser.parse_args ()

    if args.command is None:
        parser.print_help ()
        sys.exit (1)

    if args.command == 'dump-bb-cfg':
        DumpCFGDot (args.input, args.output, args.format)
    elif args.command == 'dump-pi-cfg':
        DumpPICFGDot (args.input, args.output, args.format)
    elif args.command == 'dump-bb-vgpr':
        DumpBasicBlockVGPRUsage (args.input, args.output, args.format)
    elif args.command == 'analyse-liveness' or args.command == 'analyze-liveness':
        DumpInstructionVGPRUsage (args.input, args.output, args.format, args.summary)
    elif args.command == 'opcode-histogram':
        DumpOpcodeHistogram (args.input, args.output, args.format, args)
