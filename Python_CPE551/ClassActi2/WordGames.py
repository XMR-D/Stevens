import random

import WordChooser as randomword
import WordFunctions


def Muddle():
    nb_guess = 6

    #construct the shuffled word
    picked_word = randomword.pickRandomWord()
    char_list = list(picked_word)
    random.shuffle(char_list)
    jambled_word = "".join(char_list)

    print("🔁🔁 Welcome to Muddle ! try to guess my shuffled word 🔁🔁")
    print(f"Here is the shuffled word: {jambled_word}")


    while (nb_guess > 0):
        print(f"You have {nb_guess} guess left..")

        user_guess = input("Please enter your guess : ")

        if (user_guess != picked_word):
            nb_guess -= 1
            print("❌ Not my word too bad !")
        else:
            print("✅ You fond my word, the force is strong with this one !")
            return
        
    print("❌❌❌ Not a wise one I see..... ❌❌❌")



def Snowman():
    print("❄️ Come back later it is work in progress !")
    return

def Wordly():
    print("⬛🟨🟩 Welcome to Wordly ! Will you guess my word ? ⬛🟨🟩")
    actual_word = randomword.pickRandomWord()
    length_of_Actual_word = len(actual_word)
    guesses = len(actual_word) + 1
    user_guess = ''

    while guesses > 0:
        
        
        
        print(f"\nYou have {guesses} times to guess the word")
        
        user_guess = input(f"Please enter the word: ").lower()
        
        if len(user_guess) != len(actual_word):
            print(f"Please provide a word with {length_of_Actual_word} letters ")
            continue

        if user_guess == actual_word:
            print(f"Congratulations! You are correct! The word is \033[42m{actual_word}\033[0m.")
            break
        else:
            print(f"Not quite my tempo....")
            WordFunctions.output_colored_words(user_guess, actual_word)

        guesses -= 1
        
        if guesses == 0:
            print(f"Too bad, you ran out of the guesses and failed. The word was {actual_word}!")
            return


def main():
    print("🔥 Welcome to my lair user, want to play a game ? 🔥")
    while(True):
        print("I have three tiny wordgames for you, pick your favorite:")
        print("         1 - Muddle 🔁")
        print("         2 - SnowMan ❄️")
        print("         3 - Wordly ⬛🟨🟩")
        gamepick = input("Or maybe you wanna quit? (type anything else than 1,2 or 3)\n")

        match gamepick :
            case "1":
                Muddle()
            case "2":
                Snowman()
            case "3":
                Wordly()
            case _:
                print("🔥 Then begone ! 🔥")
                break
    return


main()