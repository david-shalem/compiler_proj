#!/bin/python3
import sys
import subprocess
import itertools

OUTPUT = 'output.s'

def isFloat(line : str):
    return line.count('.') == 1 and line.replace('.', '', 1).isdigit()

getComments = lambda input : [line[2:-1] for line in input.readlines() if "//" in line]

runCommand = lambda args : subprocess.run(args, stdout=subprocess.PIPE).stdout.decode('utf-8')

compSas = lambda fileName : runCommand(['./compi', fileName, OUTPUT])

getOutput = lambda : runCommand(['./' + OUTPUT]).split('\n')[5:-1]

def main():
    if(sys.argv.__len__() != 2):
        raise Exception("error: script must accept one argument")
    fileName = sys.argv[1]
    with open(fileName, "r") as sasFile:
        comments = getComments(sasFile)
    
    if("SUCCESS" not in compSas(fileName) ):
        raise(Exception("Compilation failed"))
    
    for (comment, output) in itertools.zip_longest(comments, getOutput()):
        if(comments == None or output == None):
            break

        if(comment != output and not (isFloat(comment) and abs(float(comment) - float(output)) < 0.001)):
            raise Exception("Expected '" + comment + "' and got '" + output + "'")

    

if __name__ == '__main__':
    main()