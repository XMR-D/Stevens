# Author: Guillaume Wantiez
# Date: 03/02/2026
# Description: Program that read a poem inside another file 
# write a summary in an output file and handle errors

import os

def main():
    poem_title = ""
    poem_author = ""
    poem_lines = []

    while (1):
        poem_requested = input("Please input the name of the poem you wish summarized: ")
        try:
            with open(poem_requested, 'r') as file:
                poem_title = file.readline()
                poem_author = file.readline()

                for line in file:
                    poem_lines.append(line)
                file.close()

            with open("Output.txt", 'w') as output:
                output.write(f"The name of the poem is {poem_title}")
                output.write(f"The author of the poem is {poem_author}")
                output.write(f"The number of lines in the poem is {len(poem_lines)}\n")
                output.write(f"A preview of the poem is:\n{poem_lines[0]}{poem_lines[1]}{poem_lines[2]}")

        except:
            print(f"{poem_requested} does not exist!")

main()