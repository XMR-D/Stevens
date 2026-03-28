# Author: Guillaume Wantiez
# Date: 02/16/2026
# Description: An interactive program that allows displaying ASCII art loaded from
# external files. One file version.

import re

def sleepingCat(border):
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
    line_printed = False
    with open('Paints/pikachu.txt', 'r', encoding='utf-8') as file:
        lines = file.readlines()
    
    # We use re.sub to calculate the width without the invisible color codes
    max_len = max(len(re.sub(r'\x1b\[[0-9;]*m', '', line.strip('\n\r'))) for line in lines)    
    
    for line in lines:
        if not line_printed:
            printHeaderFooter(border, max_len + 2)
            line_printed = True
        
        clean_line = line.strip('\n\r')
        visible_len = len(re.sub(r'\x1b\[[0-9;]*m', '', clean_line))
        padding = " " * (max_len - visible_len)
        
        print(border + clean_line + padding + border)

    printHeaderFooter(border, max_len + 2)
    print("\033[0m")



def printHeaderFooter(border, size):
    print(border * size)

def blank(border):
    canvas_size = 20
    printHeaderFooter(border, canvas_size + 2)
    for i in range(5):
        print(border + (canvas_size * ' ') + border)
    printHeaderFooter(border, canvas_size + 2)

def intro():
    print("    1. The S.S. Satisfaction")
    print("    2. Mina in Repose")
    print("    3. Jinx from arcane")
    print("    4. Colored Pikachu")
    print("    5. EXIT THE EXHIBIT")

    selected_paint = input("Please select a painting to print: ")
    
    if selected_paint == "5":
        return (selected_paint, '')
    
    border_style = input("What border would you like around your painting: ")
    return (selected_paint, border_style)

def main():
    print("Welcome to the painting printer")
    print("    We have many options:")

    while True:
        (select, border) = intro()
        match select:
            case "1":
                sailingShip(border)
            case "2":
                sleepingCat(border)
            case "3":
                Jinx(border)
            case "4":
                Pikachu(border)
            case "5":
                print("Thanks for comming!")
                break
            case _:
                blank(border)
                print("\nHmmmm....we don't seem to have that painting.")
        
        print("\nWe hope you enjoy your art! Wanna see another canvas ?\n")

if __name__ == "__main__":
    main()