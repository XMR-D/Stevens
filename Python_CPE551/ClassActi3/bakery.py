
curr_label = 0
#return a tuple indicating the label to which the act pixel needs to be set up
# and also the eventual relationship we need to store, if not required return -1, 1 on the slots
#
#  return (new_label, relA, relB)
def check_pixel(grid, x, y):

    global curr_label

    #TODO: Check boarders
    
    pixel_1 = grid[y, x-1]
    pixel_2 = grid[y-1, x]

    # label conflict
    if (pixel_1 != 0 and pixel_2 != 0):
        if (pixel_1 != pixel_2):
            return ((pixel_1, pixel_1, pixel_2) if pixel_1 < pixel_2 else (pixel_2, pixel_1, pixel_2))
        else:
            return (pixel_1, -1, -1)
    
    if (pixel_1 != 0):
        return (pixel_1, -1, -1)
    
    if (pixel_2 != 0):
        return (pixel_2, -1, -1)
    
    if (pixel_1 == 0 and pixel_2 == 0):
        (curr_label+1, pixel_1, pixel_2)
    
    #return no label needed (on a 0 pixel)
    return (0, -1, -1)
    





def input_pics():
    filename = input('Please tell me what file do you want to open: ')
    file = []
    try:
        with open(filename, 'r') as images:
            for line in images:
                file.append(line.rstrip().split())
    except:
        with open('SimpleImage.txt', 'r') as images:
            for line in images:
                file.append(line.rstrip().split())
    for line in file:
        print("")
        for element in line:
            print(element, end=' ')



log_files = [ "a", "b", "c", "toto"]
print(f'{file}' for file in log_files)


prefix = "toto"

if prefix in log_files:
    print(prefix + )