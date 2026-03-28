# Author: Guillaume Wantiez
# Date: 03/02/2026
# Description: Program that average several numbers that are stored
# in an external file

import os

def main():

    with open("numbers.txt", 'r') as file:
        sum_ = 0
        nb_lines = 0

        for line in file:
            sum_ += int(line)
            nb_lines += 1

        average = sum_ / nb_lines
        print(f"The average of the numbers is : {average}")


main()