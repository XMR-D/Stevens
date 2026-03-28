# Author: Guillaume Wantiez
# Date: 02/16/2026
# Description: An interactive program that calculates and displays the total 
# surface area of a square pyramid based on user-provided base side length 
# and vertical height.

import math as math

# function definitions
def calcBaseArea(side):
    return side ** 2


# add your function definition for calcSideArea here
def calcSideArea(side, height):
    slant_height = math.sqrt((height **2) + ((side/2)**2))
    ret_val = ((side*slant_height)/2) * 4
    print(f"Total surface area of all four sides of the square pyramid is {ret_val} square feet.") 
    return ret_val

# add your function definition for prntSurfArea here
def prntSurfArea(side_area, base_area):
    print(f"The total surface area of the square pyramid is {side_area + base_area} square feet.") 

def main():
    side = float(input("Enter the side length of the base of the square pyramid in feet: "))

    height = float(input("Enter the height of the square pyramid in feet: "))

    base_area = calcBaseArea(side)
    print(f"Base surface area of the square pyramid is {base_area} square feet.")


    # add your function to calculate the side area and assign
    # the result to side_area, then print the result
    side_area = calcSideArea(side, height)

    # add your function call to print the total surface area
    prntSurfArea(side_area, base_area)

if __name__ == "__main__":
    main()