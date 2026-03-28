# Author: Guillaume Wantiez
# Date: 02/09/2026
# Description: This program simulates a baseball "at-bat" by generating a 
#              random flight distance for a hit. Based on the distance, it determines 
#              the outcome (Strike, Bunt, 2nd Base, 3rd Base, or Home Run) using 
#              conditional logic to track the player's progress on the field.

import random as r

def batterUp():
    ball_dist = r.randint(0, 450)

    if (ball_dist != 0):
        print(f"The ball flew {ball_dist} feet ", end="")

    if ball_dist > 400:
        print("and the batter scored a home run! That's one point for our team!")
    elif (ball_dist >= 135) and (ball_dist <= 400):
        print("and the batter made it to third base!")
    elif (ball_dist >= 10) and (ball_dist <= 134):
        print("and the batter made it to second base!")
    elif (ball_dist >= 1) and (ball_dist <= 9):
        print("because the batter bunted, and made it to first base!")
    else:
        print("The batter has made a strike! Oh no!")

def main():
    batterUp()

if __name__ == "__main__":
    main()