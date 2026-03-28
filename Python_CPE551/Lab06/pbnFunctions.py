# Author: Guillaume Wantiez
# Date: 03/02/2026
# Description: Func library used in paintByNumbers.py

import os

def getFileName():
    while(1):
        filename = input("Please input file you wish to have painted: ")
        if (os.path.isfile(filename)):
            return filename
        else:
            print("image does not exist!")

def convertLine(line):
    newline = []
    linelist = line.strip('\n').split(',')

    for elm in linelist:
        match elm:
            case '1':
                newline.append(' ')
            case '2':
                newline.append(',')
            case '3':
                newline.append('_')
            case '4':
                newline.append('(')
            case '5':
                newline.append('O')
            case '6':
                newline.append(')')
            case '7':
                newline.append('-')
            case '8':
                newline.append('"')
            case _:
                newline.append('')
    return "".join(newline)

def processFile(filename):
    file = open(filename, 'r')
    output = open("painting.txt", 'w')

    for line in file:
        newline = convertLine(line)
        output.write(f"{newline}\n")
    
    file.close()
    output.close()



