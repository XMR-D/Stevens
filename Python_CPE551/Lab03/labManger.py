# Author: Guillaume Wantiez
# Date: 02/23/26
# Description: Lab manager that use lists and implement basic function: Add, Remove, Display

leave = False

def addEquipment(equipments):
    if (len(equipments) == 5):
        print("Your laboratory cannot support any more equipment!")
        return
    item = input("What would you like to add to the laboratory: ")
    equipments.append(item)
    print(f"{item} has been added")
    return

def removeEquipment(equipments):
    item = input("What would you like to add to the laboratory: ")
    try:
        equipments.remove(item)
    except:
        print(f"{item} was not present and could not be removed")
        return
    
    print(f"{item} has been removed")
    return

def displayEquipment(equipments):
    if (len(equipments) == 0):
        print("Your laboratory is empty")
        return

    print("Your laboratory currently contains: ")
    for i in range(len(equipments)):
        print(equipments[i])
    return

def Handle_action(act, equipments):
    global leave
    match(act):
        case 1:
            addEquipment(equipments)
            return
        case 2:
            removeEquipment(equipments)
            return
        case 3:
            displayEquipment(equipments)
            return
        case 4:
            leave = True
            return
        case _:
            print(f"{act} is not a valid option. Please try again")


def main():
    equipments = []
    act = 0
    print("Welcome to the inventory manager for your laboratory!")

    while(not leave):
        print("You can choose from the following options:")
        print("1. Add Equipment")
        print("2. Remove Equipment")
        print("3. Display Current Equipment")
        print("4. Leave the Laboratory Manager")

        act = input("What would you like to do: ")
        Handle_action(int(act), equipments)
        print("")

    return


if __name__ == "__main__":
    main()