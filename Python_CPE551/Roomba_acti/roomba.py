
from array import array

grid = [['D', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '], 
        [' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '], 
        [' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '], 
        [' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '], 
        [' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '], 
        [' ', ' ', ' ', ' ', ' ', 'd', ' ', ' ', ' ', ' '], 
        [' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '], 
        [' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '], 
        [' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '], 
        [' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ']]

roomba_start_pos = (0,0)

def facing_direction(prev, act):
    diff_x = prev[0] - act[0]
    diff_y = prev[1] - act[1]

    if (diff_x > 0):
        return 0
    elif (diff_x < 0):
        return 1
    elif (diff_y > 0):
        return 2
    else:
        return 3
    
def bump(actual_pos, direction):
    if (direction == 0 and (actual_pos[0]+1) > 9):
        return True
    elif(direction == 1 and (actual_pos[0]-1) < 0):
        return True
    elif(direction == 2 and (actual_pos[1]-1) < 0):
        return True
    elif(direction == 3 and (actual_pos[1]+1) > 9):
        return True
    else:
        return False

def turn_n_move(actual_pos, direction):

    if (bump(actual_pos, direction) == True):
        return (-1, -1)
    # RIGHT = 0
    if (direction == 0):
        return (actual_pos[0] + 1, actual_pos[1])
    #LEFT = 1
    elif(direction == 1):
        return (actual_pos[0] - 1, actual_pos[1])
    
    #UP = 2
    elif(direction == 2):
        return (actual_pos[0], actual_pos[1] - 1)
    
    #DOWN = 3
    elif(direction == 3):
        return (actual_pos[0], actual_pos[1] + 1)


def is_dirtpos(grid, actual_pos):
    return (grid[actual_pos[0]][actual_pos[1]] == 'd')


#Phase 1
def find_dirt(grid, roomba_start_pos):
    actual_pos = roomba_start_pos
    act_facing_dir = 0

    print(f"Roomba exiting dock")

    while (is_dirtpos(grid, actual_pos) == False):
        #If facing right
        if (act_facing_dir == 0):
            pos = turn_n_move(actual_pos, 0)
            if (pos[0] == -1):
                pos = turn_n_move(actual_pos, 3)
                act_facing_dir = 1
            else:
                act_facing_dir = 0

        #if facing left
        elif (act_facing_dir == 1):
            pos = turn_n_move(actual_pos, 1)
            if (pos[0] == -1):
                pos = turn_n_move(actual_pos, 3)
                act_facing_dir = 0
            else:
                act_facing_dir = 1
        actual_pos = pos

        print(f"Roomba moved to : {actual_pos[0]}, {actual_pos[1]}")


    print(f"Roomba reach dirt at : {actual_pos[0]}, {actual_pos[1]}")
    return actual_pos


#Phase 2
def get_back(actual_pos):
    while(actual_pos[0] != 0):
        actual_pos = turn_n_move(actual_pos, 1)
    while(actual_pos[1] != 0):
        actual_pos = turn_n_move(actual_pos, 2)

    print("Roomba reach dock back")
    return actual_pos

get_back(find_dirt(grid, roomba_start_pos))