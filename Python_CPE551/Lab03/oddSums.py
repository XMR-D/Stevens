# Author: Guillaume Wantiez
# Date: 02/09/2026
# Description: This program generates a random range with a lower bound 
#              between 1-10 and an upper bound between 11-20. It then 
#              iterates through this range to calculate and display the 
#              sum of all odd numbers found within it.

import random as r

def oddSum():
    lower_bound = r.randint(1, 10)
    upper_bound = r.randint(11, 20)
    odd_sum = 0

    for i in range(lower_bound, upper_bound):
        if (i % 2 != 0):
            odd_sum += i

    print(f"Randomly generated lower bound : {lower_bound}")
    print(f"Randomly generated upper bound : {upper_bound}")
    print(f"Odd sum : {odd_sum}")
    
if __name__ == "__main__":
    oddSum()
