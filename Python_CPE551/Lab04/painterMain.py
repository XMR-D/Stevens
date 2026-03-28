# Author: Guillaume Wantiez
# Date: 02/16/2026
# Description: An interactive program that allows displaying ASCII art loaded from
# external files. multiple file version, all other functions are defined in painterFuncs.py

import painterFuncs as pF

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
                pF.sailingShip(border)
            case "2":
                pF.sleepingCat(border)
            case "3":
                pF.Jinx(border)
            case "4":
                pF.Pikachu(border)
            case "5":
                print("Thanks for comming!")
                break
            case _:
                pF.blank(border)
                print("\nHmmmm....we don't seem to have that painting.")
        
        print("\nWe hope you enjoy your art! Wanna see another canvas ?\n")

if __name__ == "__main__":
    main()