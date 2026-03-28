# Author: Guillaume Wantiez
# Date: 02/09/2025
# Description: This program is a coordinate-guessing game where the user attempts 
#              to locate a hidden particle in a 10x10 grid. The user has three 
#              chances to guess the (x, y) coordinates, receiving directional 
#              feedback (greater than/less than) after each incorrect attempt 
#              to narrow down the search area.



def badLuck(x, y, p_x, p_y):
    if ((x > 10 or x < 1) or (y > 10 or y < 1)):
        print(f"No good ! ({x}, {y}) is outside of the range!")
        return
    
    if (x > p_x):
        print("Bad luck! The particle's x position is less than your x position!")
    elif (x < p_x):
        print("Bad luck! The particle's x position is greater than your x position!")
    else:
        print("Spot on for your x position !")
    
    if (y > p_y):
        print("Bad luck! The particle's y position is less than your x position!")
    elif (y < p_y):
        print("Bad luck! The particle's y position is greater than your y position!")
    else:
        print("Spot on for your y position !")

def main():
    user_guess = 3

    particule_x = 4
    particule_y = 6

    while (user_guess > 0):
        print(f"The particle is somewhere in this space! You have {user_guess} chances to guess it.")
        user_guess_x = int(input("What do you think its x coordinate is (1-10)? "))
        user_guess_y = int(input("What do you think its y coordinate is (1-10)? "))

        if ((user_guess_x == particule_x) and (user_guess_y == particule_y)):
            print(f"Good guess! ({particule_x}, {particule_y}) was the position")
            return
        else:
            user_guess -= 1
            badLuck(user_guess_x, user_guess_y, particule_x, particule_y)
        
    print(f"Oh no! You ran out of chances. ({particule_x},{particule_y}) was the particle's position!")
    return


if __name__ == "__main__":
    main()