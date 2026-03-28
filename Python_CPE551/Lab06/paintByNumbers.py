# Author: Guillaume Wantiez
# Date: 03/02/2026
# Description: Program that translate a file into a painting

import pbnFunctions as pbn


def main():
    #try:
    userfile = pbn.getFileName()
    pbn.processFile(userfile)
    print("Your image can be found in painting.txt . Enjoy!")
    #except:
    #    print("Error while handling file")

    return

if __name__ == "__main__":
    main()