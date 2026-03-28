# Author: Guillaume Wantiez
# Date: 02/23/26
# Description: Generates a square map of capture rates using a constant dimension. 
# It identifies the coordinates of the highest and lowest rates within the grid.

import random as rand

DIM = 8

maxcap_rate_x = 0
maxcap_rate_y = 0

lowcap_rate_x = 500
lowcap_rate_y = 500

max_rate = 0
low_rate = 500

grid = [[0 for _ in range(DIM)] for _ in range(DIM)]


for x in range(len(grid)):
    print("")
    for y in range(len(grid[x])):
        grid[y][x] = rand.randint(0, 500)
        
        if (grid[y][x] > max_rate):
            max_rate = grid[y][x]
            maxcap_rate_x = x
            maxcap_rate_y = y
        
        if (grid[y][x] < low_rate):
            low_rate = grid[y][x]
            low_rate_x = x
            low_rate_y = y
        print(f"{grid[y][x]:3}", end=' ') 

print("\n")
print(f"The highest capture rate was {max_rate} at location {maxcap_rate_x + 1},{maxcap_rate_y + 1}.")
print(f"The lowest capture rate was {low_rate} at location {low_rate_x + 1},{low_rate_y + 1}.")
print("The map looks like the following:")
