# Author: Guillaume Wantiez
# Date: 03/12/2026
# Description: Program that counts word frequencies in a file using a dictionary

import os

def readTextFile(word_counts):
    while True:
        filename = input("Please enter the name of the file to analyze: ")
        if os.path.exists(filename):
            break
        print(f"{filename} does not exist!")

    with open(filename, 'r') as file:
        for line in file:
            clean_line = line.strip().lower()
            words = clean_line.split()
            
            for word in words:
                if word in word_counts:
                    word_counts[word] += 1
                else:
                    word_counts[word] = 1

def outputFreq(word_counts):
    print(f"\nNumber of unique words: {len(word_counts)}")
    print("Word frequencies:")
    for word, count in word_counts.items():
        print(f"{word}: {count}")

def main():
    word_dictionary = {}
    readTextFile(word_dictionary)
    outputFreq(word_dictionary)

if __name__ == "__main__":
    main()