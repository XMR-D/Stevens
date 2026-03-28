# Author: Guillaume Wantiez
# Date: 03/12/2026
# Description: Repository program that manages stopwords and persists them via pickling

import os
import pickle

PICKLE_FILE = "stopwordset.data"

def readStopWordsFile(stopwords):
    while True:
        filename = input("Enter the name of the file to be read in: ")
        if os.path.exists(filename):
            break
        print("File does not exist!")

    with open(filename, 'r') as file:
        for line in file:
            word = line.strip().lower()
            if word in stopwords:
                print(f"'{word}' is already in the Set.")
            else:
                stopwords.add(word)

def writeStopWordsFile(stopwords):
    filename = input("Enter the name of the file to be written to: ")
    with open(filename, 'w') as file:
        for word in stopwords:
            file.write(f"{word}\n")

def displayStopWords(stopwords):
    print(f"Currently we have {len(stopwords)} stopwords:")
    for word in stopwords:
        print(word)

def storeStopWords(stopwords):
    with open(PICKLE_FILE, 'wb') as file:
        pickle.dump(stopwords, file)

def restoreStopWords():
    with open(PICKLE_FILE, 'rb') as file:
        return pickle.load(file)

def main():
    if os.path.exists(PICKLE_FILE):
        stopwords = restoreStopWords()
    else:
        stopwords = set()

    while True:
        print("\nWelcome to the stopword repository!")
        print("1. Add a new list of stopwords")
        print("2. Write current set of stopwords to a file")
        print("3. Display current set of stopwords")
        print("4. Quit")
        
        choice = input("Please enter your choice: ")
        
        if choice == '1':
            readStopWordsFile(stopwords)
        elif choice == '2':
            writeStopWordsFile(stopwords)
        elif choice == '3':
            displayStopWords(stopwords)
        elif choice == '4':
            storeStopWords(stopwords)
            print("Thank you for using the stopword repository!")
            break
        else:
            print("Invalid choice.")

if __name__ == "__main__":
    main()