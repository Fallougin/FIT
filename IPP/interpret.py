# IPP - Principy programovacich jazyku a OOP
# Description: Projekt c.2 - Interpret jazyka IPPcode22
# XML code representation interpreter

# Name: interpret.py
# Autor: Artur Suvorkin (xsuvor00)

# System-specific parameters and functions https://docs.python.org/3/library/sys.html
import sys

# Regular expression operations https://docs.python.org/3/library/re.html
import re 

# Parser for command-line options, arguments and sub-commands https://docs.python.org/3/library/argparse.html
import argparse 

# The ElementTree XML API https://docs.python.org/3/library/xml.etree.elementtree.html
import xml.etree.ElementTree as ET 

frames = {"GF": {}}
framesStack = []
callStack = []
dataStack = []
labels = {}

instructionCounter = 0
dataStackCount = 0
dataStackMaxCount = 0

sourceFile = sys.stdin
inputFile = sys.stdin

exitBool = False
exitValue = 0

# Filling an ArgumentParser with information about program arguments is done by making calls to the add_argument() method.
parser = argparse.ArgumentParser(add_help = False, prefix_chars = "--")
parser.add_argument('--help', action = 'store_true')
parser.add_argument('--source', dest = 'source')
parser.add_argument('--input', dest = 'input')

currentInstruction = 0 # Currrent instructuion counter

try:
    args = parser.parse_args()
except:
    sys.exit(10)

# Functions for intereters
def getValue(arg):
    if arg["type"] == "var":
        value = getVarValue(arg)
        if value == None:
            print("Trying to get value from non-existent variable", file = sys.stderr)
            sys.exit(56)
        return value
    else:
        return arg["value"]

def getType(arg):
    if arg["type"] == "var":
        varType = getVarType(arg)
        if varType == None:
            print("Trying to get value from non-existent variable", file = sys.stderr)
            sys.exit(56)
        return getVarType(arg)
    else:
        return arg["type"]

def getLabel(var):
    if not var["value"] in labels:
        print("Undefiend label", file = sys.stderr)
        sys.exit(52)
    return labels[var["value"]] - 1

def getFrame(var):
    return var["value"][0:2], var["value"][3:]

def ifFrameExists(frame):
    if not frame in frames:
        print("Frame don't exists", file = sys.stderr)
        sys.exit(55)

def ifVarExistsFrame(frame, name):
    ifFrameExists(frame)
    if not name in frames[frame]:
        print("Undefiend variable", file = sys.stderr)
        sys.exit(54)

def getVarValue(var):
    frame, name = getFrame(var)
    ifVarExistsFrame(frame, name)
    value = frames[frame][name]["value"]
    return value

def getVarType(var):
    frame, name = getFrame(var)
    ifVarExistsFrame(frame, name)
    varType = frames[frame][name]["type"]
    return varType

def setVarValue(var, value):
    frame, name = getFrame(var)
    ifVarExistsFrame(frame, name)
    frames[frame][name]["value"] = value

def setVarType(var, varType):
    frame, name = getFrame(var)
    ifVarExistsFrame(frame, name)
    frames[frame][name]["type"] = varType

# Set of instructions
def move(args):
    setVarValue(args[0], getValue(args[1]))
    setVarType(args[0], getType(args[1]))

def defvar(args):
    frame, name = getFrame(args[0])
    ifFrameExists(frame)
    if name in frames[frame]:
        print("Revaluation of variable", file = sys.stderr)
        sys.exit(52)

    frames[frame][name] = {"value": None, "type": None}

def write(args):
    valType = getType(args[0])
    if(valType == "nil"):
        print("", end = "")
    elif(valType == "bool"):
        print(str(getValue(args[0])).lower(), end = "")
    else:
        print(getValue(args[0]), end = "")

def concat(args):
    if (getType(args[1]) != "string" or getType(args[2]) != "string"):
        print("CONCAT types disparity", file = sys.stderr)
        sys.exit(53)
    setVarType(args[0], "string")
    setVarValue(args[0], getValue(args[1])+getValue(args[2]))

def jumpifeq(args):
    global currentInstruction
    if((getType(args[1]) == "nil") ^ (getType(args[2]) == "nil")):
        return

    if(getType(args[1]) != getType(args[2])):
        print("JUMPIFEQ not same types", file = sys.stderr)
        sys.exit(53)

    label = getLabel(args[0])
    if(getValue(args[1]) == getValue(args[2])):
        currentInstruction = label

def jumpifneq(args):
    global currentInstruction
    if((getType(args[1]) == "nil") ^ (getType(args[2]) == "nil")):
        currentInstruction = getLabel(args[0])
        return
    if(getType(args[1]) != getType(args[2])):
        print("JUMPIFNEQ not same types 1:{} 2:{}", getValue(args[1]), getType(args[2]), file = sys.stderr)
        sys.exit(53)

    label = getLabel(args[0])
    if(getValue(args[1]) != getValue(args[2])):
        currentInstruction = label

def jump(args):
    global currentInstruction
    currentInstruction = getLabel(args[0])

def createframe(args):
    frames["TF"] = {}

def pushframe(args):
    ifFrameExists("TF")
    framesStack.append(frames["TF"])
    frames.pop("TF")
    frames["LF"] = framesStack[len(framesStack)-1]

def popframe(args):
    ifFrameExists("LF")
    frames["TF"] = frames["LF"]
    framesStack.pop()
    if len(framesStack) > 0:
        frames["LF"] = framesStack[len(framesStack)-1]
    else:
        frames.pop("LF")

def call(args):
    global currentInstruction, callStack
    callStack.append(currentInstruction)
    currentInstruction = getLabel(args[0])

def ret(args):
    global currentInstruction, callStack
    if len(callStack) == 0:
        print("RETURN miss value at call stack", file = sys.stderr)
        sys.exit(56)
    currentInstruction = callStack.pop()

def add(args):
    if (not(getType(args[1]) == "int" and getType(args[2]) == "int")):
        print("ADD variable types disparity", file = sys.stderr)
        sys.exit(53)

    setVarType(args[0], getType(args[1]))
    setVarValue(args[0], getValue(args[1]) + getValue(args[2]))

def sub(args):
    if (not(getType(args[1]) == "int" and getType(args[2]) == "int")):
        print("SUB variable types disparity", file = sys.stderr)
        sys.exit(53)

    setVarType(args[0], getType(args[1]))
    setVarValue(args[0], getValue(args[1]) - getValue(args[2]))

def mul(args):
    if (not(getType(args[1]) == "int" and getType(args[2]) == "int")):
        print("MUL variable types disparity", file = sys.stderr)
        sys.exit(53)

    setVarType(args[0], getType(args[1]))
    setVarValue(args[0], getValue(args[1]) * getValue(args[2]))

def idiv(args):
    if getValue(args[2]) == 0:
        sys.exit(57)

    if (not(getType(args[1]) == "int" and getType(args[2]) == "int")):
        print("IDIV variable types disparity", file = sys.stderr)
        sys.exit(53)

    setVarType(args[0], getType(args[1]))
    setVarValue(args[0], getValue(args[1]) // getValue(args[2]))

def lt(args):
    if(getType(args[1]) == "nil" or getType(args[2]) == "nil" or getType(args[1]) != getType(args[2])):
        print("LT is not the same type of variables", file = sys.stderr)
        sys.exit(53)

    setVarType(args[0], "bool")
    if getValue(args[1]) < getValue(args[2]):
        setVarValue(args[0], True)
    else:
        setVarValue(args[0], False)

def gt(args):
    if(getType(args[1]) == "nil" or getType(args[2]) == "nil" or getType(args[1]) != getType(args[2])):
        print("GT is not the same type of variables", file = sys.stderr)
        sys.exit(53)

    setVarType(args[0], "bool")
    if getValue(args[1]) > getValue(args[2]):
        setVarValue(args[0], True)
    else:
        setVarValue(args[0], False)

def eq(args):
    if((getType(args[1]) == "nil") ^ (getType(args[2]) == "nil")):
        setVarType(args[0], "bool")
        setVarValue(args[0], False)
        return

    if(getType(args[1]) != getType(args[2])):
        print("EQ not same types of variables", file = sys.stderr)
        sys.exit(53)

    setVarType(args[0], "bool")
    if getValue(args[1]) == getValue(args[2]):
        setVarValue(args[0], True)
    else:
        setVarValue(args[0], False)

def logAnd(args):
    if (getType(args[1]) != "bool" or getType(args[2]) != "bool"):
        print("AND not bool variables", file = sys.stderr)
        sys.exit(53)

    setVarType(args[0], "bool")
    setVarValue(args[0], getValue(args[1]) and getValue(args[2]))

def logOr(args):
    if (getType(args[1]) != "bool" or getType(args[2]) != "bool"):
        print("OR not bool variables", file = sys.stderr)
        sys.exit(53)

    setVarType(args[0], "bool")
    setVarValue(args[0], getValue(args[1]) or getValue(args[2]))

def logNot(args):
    if (getType(args[1]) != "bool"):
        print("NOT not bool variable", file = sys.stderr)
        sys.exit(53)

    setVarType(args[0], "bool")
    setVarValue(args[0], not getValue(args[1]))

def int2char(args):
    if getType(args[1]) != "int":
        print("INT2CHAR variable type disparity", file = sys.stderr)
        sys.exit(53)
    val = getValue(args[1])
    try:
        val = chr(val) # chr() function is used to get a string representing of a character which points to a Unicode code integer
    except:
        print("INT2CHAR chr function fail", file = sys.stderr)
        sys.exit(58)

    setVarValue(args[0], val)
    setVarType(args[0], "string")

def str2int(args):
    if getType(args[1]) != "string" or getType(args[2]) != "int":
        print("STR2INT variable types disparity", file = sys.stderr)
        sys.exit(53)
    string = getValue(args[1])
    index = getValue(args[2])
    if index < 0:
        print("STR2INT index < 0", file = sys.stderr)
        sys.exit(58)
    try:
        val = ord(string[index]) #ord() function returns the Unicode code from a given character.
    except:
        print("STR2INT ord function fail", file = sys.stderr)
        sys.exit(58)

    setVarValue(args[0], val)
    setVarType(args[0], "int")

def read(args):
    val = inputFile.readline()
    varType = getValue(args[1])
    setType = "nil"
    setValue = "nil"

    if len(val) == 0:
        setVarType(args[0], setType)
        setVarValue(args[0], setValue)
        return

    val = val.rstrip("\n")

    if varType == "bool":
        setType = "bool"
        if val.lower() == "true":
            setValue = True
        else:
            setValue = False
    elif varType == "int":
        setType = "int"
        try:
            setValue = int(val)
        except:
            setType = "nil"
            setValue = "nil"
    elif varType == "string":
        setType = "string"
        setValue = val

    setVarType(args[0], setType)
    setVarValue(args[0], setValue)

def strlen(args):
    if (getType(args[1]) != "string"):
        print("STRLEN variable type disparity", file = sys.stderr)
        sys.exit(53)
    setVarValue(args[0], len(getValue(args[1])))
    setVarType(args[0], "int")

def getchar(args):
    if getType(args[1]) != "string" or getType(args[2]) != "int":
        print("GETCHAR variable types disparity", file = sys.stderr)
        sys.exit(53)
    string = getValue(args[1])
    index = getValue(args[2])
    if index < 0:
        sys.exit(58)
        print(" GETCHAR index < 0", file = sys.stderr)
    try:
        value = string[index]
    except:
        print("GETCHAR index out of borders", file = sys.stderr)
        sys.exit(58)

    setVarValue(args[0], value)
    setVarType(args[0], "string")

def setchar(args):
    if getType(args[0]) != "string" or getType(args[1]) != "int" or getType(args[2]) != "string":
        print("SETCHAR variable types disparity", file = sys.stderr)
        sys.exit(53)
    index = getValue(args[1])
    if index < 0:
        sys.exit(58)
        print("SETCHAR index < 0", file = sys.stderr)
    try:
        val = getValue(args[0])
        val = list(val)
        val[index] = getValue(args[2])[0]
        val = ''.join(val)
        setVarValue(args[0], val)
    except:
        print("SETCHAR iindex out of borders", file = sys.stderr)
        sys.exit(58)

def typeFunc(args):
    if args[1]["type"] == "var":
        varType = getVarType(args[1])
        if varType == None:
            varType = ""
    else:
        varType = args[1]["type"]

    if varType == "var":
        frame, name = getFrame(args[1])
        if not name in frames[frame]:
            varType = ""

    setVarType(args[0], "string")
    setVarValue(args[0], varType)

def exitInterpret(args):
    global exitValue, exitBool
    if (getType(args[0]) != "int"):
        print("{}: EXIT variable types disparity".format(currentInstruction), file = sys.stderr)
        sys.exit(53)
    val = getValue(args[0])
    if val < 0 or val > 49:
        print("{}: EXIT value out of [0,49]".format(currentInstruction), file = sys.stderr)
        sys.exit(57)

    exitBool = True
    exitValue = val

def dprint(args):
    print(getValue(args[0]), file = sys.stderr)

def breakInterpret(args):
    print("Current instruction index: {}".format(currentInstruction), file = sys.stderr)
    print(frames, file = sys.stderr)

def getValueStack():
    global dataStack, dataStackCount
    if len(dataStack) == 0:
        sys.exit(56)

    dataStackCount -= 1
    return dataStack.pop()


def setValStack(val, varType):
    global dataStack, dataStackCount, dataStackMaxCount
    dataStack.append({"value": val, "type": varType})
    dataStackCount += 1
    if dataStackCount > dataStackMaxCount:
        dataStackMaxCount = dataStackCount


def pushs(args):
    global dataStack, dataStackCount, dataStackMaxCount
    dataStack.append({"value": getValue(args[0]), "type": getType(args[0])})
    dataStackCount += 1
    if dataStackCount > dataStackMaxCount:
        dataStackMaxCount = dataStackCount


def pops(args):
    global dataStack, dataStackCount
    if len(dataStack) == 0:
        print("Data stack is empty can't to pop", file = sys.stderr)
        sys.exit(56)

    dataStackCount -= 1
    var = dataStack.pop()
    setVarValue(args[0], var["value"])
    setVarType(args[0], var["type"])

def nothing(args):
    pass


# Parser for XML tree. Checks lexical and syntax program construction.
def checkXML():
    parseTree = {}
    orderList = []
    try:
        tree = ET.parse(sourceFile)
        root = tree.getroot()
    except:
        sys.exit(31)

    if root.tag != "program" or not("language" in root.attrib):
        sys.exit(32)

    if root.attrib["language"].lower() != "ippcode22":
        sys.exit(32)

    # Sorting instructions by opcode
    try:
        root[:] = sorted(root, key = lambda child: int(child.attrib["order"]))
    except:
        sys.exit(32)

    instLine = 0
    for instruction in root:
        # Check XML instruction; re. regex expression
        if not re.match("^instruction$", instruction.tag): 
            sys.exit(32)

        # Correct attributes and count of them
        if not("opcode" in instruction.attrib) or len(instruction.attrib) != 2:
            sys.exit(32)

        order = instruction.attrib["order"]
        instructionOpcode = instruction.attrib["opcode"].upper()

        # Duplicit order in instruction or order <= 0 
        if order in orderList or int(order) <= 0:
            sys.exit(32)

        orderList.append(order)
        # Sort arguments 1...max
        instruction[:] = sorted(instruction, key = lambda child: child.tag)

        args = []
        argumentCount = 1
        for arg in instruction:
            # XML argument tag
            if arg.tag != ("arg" + str(argumentCount)):
                sys.exit(32)

            # Check for correct attributes 
            if not("type" in arg.attrib) or len(arg.attrib) != 1:
                sys.exit(32)

            # Is defult set of arguemnts value if empty from XML
            if arg.text == None:
                arg.text = ""

            # Check if given type is correct to the given value
            if not argumentTypeCheck(arg.text, arg.attrib["type"]):
                sys.exit(32)

            argValue = decodeArgumentValue(arg.attrib["type"], arg.text)
            args.append({"type": arg.attrib["type"], "value": argValue})
            argumentCount += 1

        global instructions

        # Opcode check if exists
        if not instructionOpcode in instructions:
            sys.exit(32)

        # Check if count of arguments match to arguments counter
        if len(instructions[instructionOpcode]["args"]) != len(args):
            sys.exit(32)

        # Arguments check
        for i in range(0, len(args)):
            if instructions[instructionOpcode]["args"][i] == "symb":
                if args[i]["type"] in ["int", "string", "bool", "nil", "var"]:
                    continue
            elif instructions[instructionOpcode]["args"][i] == args[i]["type"]:
                continue

            sys.exit(32)

        # Add new line to parseTree
        parseTree[instLine] = {"instruction": instructionOpcode, "args": args, "order": order, "counter": 0}
        # Add label if opcode LABEL 
        if instructionOpcode == "LABEL":
            if args[0]["value"] in labels:
                sys.exit(52)
            labels[args[0]["value"]] = instLine
        
        instLine += 1

    return parseTree

# Contains information about instruction arguments.
instructions = {
    # Working with frames, function calls
    "MOVE": {"args": ["var", "symb"], "func": move},
    "CREATEFRAME": {"args": [], "func": createframe},
    "PUSHFRAME": {"args": [], "func": pushframe},
    "POPFRAME": {"args": [], "func": popframe},
    "DEFVAR": {"args": ["var"], "func": defvar},
    "CALL": {"args": ["label"], "func": call},
    "RETURN": {"args": [], "func": ret},
    # Working with the data stack
    "PUSHS": {"args": ["symb"], "func": pushs},
    "POPS": {"args": ["var"], "func": pops},
    # Arithmetic, relational, Boolean and conversion instructions
    "ADD": {"args": ["var", "symb", "symb"], "func": add},
    "SUB": {"args": ["var", "symb", "symb"], "func": sub},
    "MUL": {"args": ["var", "symb", "symb"], "func": mul},
    "IDIV": {"args": ["var", "symb", "symb"], "func": idiv},
    "LT": {"args": ["var", "symb", "symb"], "func": lt},
    "GT": {"args": ["var", "symb", "symb"], "func": gt},
    "EQ": {"args": ["var", "symb", "symb"], "func": eq},
    "AND": {"args": ["var", "symb", "symb"], "func": logAnd},
    "OR": {"args": ["var", "symb", "symb"], "func": logOr},
    "NOT": {"args": ["var", "symb"], "func": logNot},
    "INT2CHAR": {"args": ["var", "symb"], "func": int2char},
    "STRI2INT": {"args": ["var", "symb", "symb"], "func": str2int},
    # I/O instructions
    "READ":  {"args": ["var", "type"], "func": read},
    "WRITE": {"args": ["symb"], "func": write},
    # Working with strings
    "CONCAT": {"args": ["var", "symb", "symb"], "func": concat},
    "STRLEN": {"args": ["var", "symb"], "func": strlen},
    "GETCHAR": {"args": ["var", "symb", "symb"], "func": getchar},
    "SETCHAR": {"args": ["var", "symb", "symb"], "func": setchar},
    # Working with types
    "TYPE": {"args": ["var", "symb"], "func": typeFunc},
    # Program flow control instructions
    "LABEL": {"args": ["label"], "func": nothing},
    "JUMP": {"args": ["label"], "func": jump},
    "JUMPIFEQ": {"args": ["label", "symb", "symb"], "func": jumpifeq},
    "JUMPIFNEQ": {"args": ["label", "symb", "symb"], "func": jumpifneq},
    "EXIT": {"args": ["symb"], "func": exitInterpret},
    # Debugging instructions
    "DPRINT": {"args": ["symb"], "func": dprint},
    "BREAK":  {"args": [], "func": breakInterpret},
}

# Convert escape sequences to characters.
def sequencesToASCII(match):
    seq = match.group(0).replace("\\", "")
    return str(chr(int(seq)))


# Return formated value to relevant type of variable.
def decodeArgumentValue(argType, value):
    decodedValue = value
    if(argType == "string"):
        decodedValue = re.sub('\\\\[0-9]{3}', sequencesToASCII, value)
    elif(argType == "int"):
        decodedValue = int(value)
    elif(argType == "bool"):
        if value == "true":
            decodedValue = True
        else:
            decodedValue = False

    return decodedValue

#
def parseArguments():
    global sourceFile, inputFile
    if args.help:
        if len(sys.argv) == 2:
            help()
            sys.exit(0)
        else:
            sys.exit(10)

    if (args.input == None and args.source == None):
        sys.exit(10)

    if(args.input):
        try:
            inputFile = open(args.input)
        except:
            sys.exit(10)

    if(args.source):
        try:
            sourceFile = open(args.source)
        except:
            sys.exit(10)

# Checking if given variable type is correct to given value.
# For regex expression https://regexr.com/
def argumentTypeCheck(value, expectedType):
    expectedType = expectedType.lower()
   
    if(expectedType == "int"):
        if not re.match('^[+-]?[\d]+$', value):    
            return False
    elif(expectedType == "nil"):
        if not re.match('^nil$', value):
            return False
    elif(expectedType == "var"):
        if not re.match('^(GF|TF|LF)@[a-z_\-$&%*!?A-Z][a-z_\-$&%*!?0-9A-Z]*$', value):
            return False
    elif(expectedType == "type"):
        if not re.match('int$|^bool$|^string$|^nil$', value):
            return False
    elif(expectedType == "label"):
        if not re.match('(?i)^[a-z_\-$&%*!?][a-z_\-$&%*!?0-9]*$', value):
            return False
    elif(expectedType == "bool"):
        if not re.match('^false$|^true$', value):
            return False
    return True

def codeInterperter(tree, lastIndex):
    global currentInstruction, instructionCounter
    while currentInstruction <= lastIndex and (not exitBool):
        if not re.match('^DPRINT$|^LABEL$|^BREAK$', tree[currentInstruction]["instruction"]):
            tree[currentInstruction]["counter"] += 1
            instructionCounter += 1

        instructions[tree[currentInstruction]["instruction"]
                     ]["func"](tree[currentInstruction]["args"])
        currentInstruction += 1

# Help instruction
def help():
    print("Usage: interpret.py [ --source=file | --input=file ]")
    print(" --source | Source file of IPPcode22")
    print(" --input  | Input file for program to read it")

def main():
    global exitValue
    parseArguments()
    tree = checkXML()

    if len(list(tree)) != 0:
        lastInstruction = int(list(tree)[-1])
        codeInterperter(tree, lastInstruction)

    sys.exit(exitValue)

# https://stackoverflow.com/questions/419163/what-does-if-name-main-do
if __name__ == '__main__':
    main()
