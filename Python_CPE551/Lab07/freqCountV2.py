# Author: Guillaume Wantiez
# Date: 03/12/2026
# Description: Program that counts word frequencies excluding stopwords

import os

def readStopWordsFile(stopwords):
    while True:
        filename = input("Please enter the name of the stopwords file: ")
        if os.path.exists(filename):
            break
        print(f"{filename} does not exist! Please enter the name of the stopwords file: ")

    with open(filename, 'r') as file:
        for line in file:
            stopwords.add(line.strip().lower())

def readTextFile(word_counts, stopwords):
    while True:
        filename = input("Please enter the name of the file to analyze: ")
        if os.path.exists(filename):
            break
        print(f"{filename} does not exist! Please enter the name of the file: ")

    with open(filename, 'r') as file:
        for line in file:
            clean_line = line.strip().lower()
            words = clean_line.split()
            
            for word in words:
                if word not in stopwords:
                    if word in word_counts:
                        word_counts[word] += 1
                    else:
                        word_counts[word] = 1

def outputFreq(word_counts):
    print(f"The file contained {len(word_counts)} unique words.")
    for word, count in word_counts.items():
        print(f"{word}:{count}")

def main():
    word_dictionary = {}
    stopwords_set = set()
    
    readStopWordsFile(stopwords_set)
    readTextFile(word_dictionary, stopwords_set)
    outputFreq(word_dictionary)

if __name__ == "__main__":
    main()