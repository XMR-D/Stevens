# Author: Guillaume Wantiez
# Date: 02/16/2026
#Description: An interactive ASCII art gallery that displays bordered artworks 
# from text files while handling color-code alignment for specific paintings.


import re

def sleepingCat(border):
    """Prints a bordered ASCII cat from a file."""
    line_printed = False
    with open('Paints/sleepingCat.txt', 'r', encoding='utf-8') as file:
        lines = file.readlines()
    
    max_len = max(len(line.strip('\n\r')) for line in lines)
    
    for line in lines:
        if not line_printed:
            printHeaderFooter(border, max_len + 2)
            line_printed = True

        clean_line = line.strip('\n\r')
        padding = " " * (max_len - len(clean_line))
        print(border + clean_line + padding + border)

    printHeaderFooter(border, max_len + 2)

def sailingShip(border):
    """Prints a bordered ASCII ship from a file."""
    line_printed = False
    with open('Paints/sailingShip.txt', 'r', encoding='utf-8') as file:
        lines = file.readlines()
    
    max_len = max(len(line.strip('\n\r')) for line in lines)
    
    for line in lines:
        if not line_printed:
            printHeaderFooter(border, max_len + 2)
            line_printed = True

        clean_line = line.strip('\n\r')
        padding = " " * (max_len - len(clean_line))
        print(border + clean_line + padding + border)

    printHeaderFooter(border, max_len + 2)
    
def Jinx(border):
    """Prints a bordered ASCII Jinx from a file."""
    line_printed = False
    with open('Paints/jinx.txt', 'r', encoding='utf-8') as file:
        lines = file.readlines()
    
    max_len = max(len(line.strip('\n\r')) for line in lines)
    
    for line in lines:
        if not line_printed:
            printHeaderFooter(border, max_len + 2)
            line_printed = True

        clean_line = line.strip('\n\r')
        padding = " " * (max_len - len(clean_line))
        print(border + clean_line + padding + border)

    printHeaderFooter(border, max_len + 2)

def Pikachu(border):
    """Prints a bordered colorful ASCII Pikachu while ignoring ANSI color codes."""
    line_printed = False
    with open('Paints/pikachu.txt', 'r', encoding='utf-8') as file:
        lines = file.readlines()
    
    # Calculate width ignoring invisible ANSI color codes
    max_len = max(len(re.sub(r'\x1b\[[0-9;]*m', '', line.strip('\n\r'))) for line in lines)    
    
    for line in lines:
        if not line_printed:
            printHeaderFooter(border, max_len + 2)
            line_printed = True
        
        clean_line = line.strip('\n\r')
        visible_len = len(re.sub(r'\x1b\[[0-9;]*m', '', clean_line))
        padding = " " * (max_len - visible_len)
        
        # Reset color at the end of content to keep the border clean
        print(border + clean_line + padding + border)

    printHeaderFooter(border, max_len + 2)
    print("\033[0m")

def printHeaderFooter(border, size):
    """Prints a decorative top or bottom border line."""
    print(border * size)

def blank(border):
    """Prints an empty bordered canvas."""
    canvas_size = 20
    printHeaderFooter(border, canvas_size + 2)
    for i in range(5):
        print(border + (canvas_size * ' ') + border)
    printHeaderFooter(border, canvas_size + 2)