# green background \033[42m
# yellow background \033[43m
# default background \033[0m
def output_colored_words(user_guess, actual_word):
    for pos in range(len(user_guess)):
        if user_guess[pos] == actual_word[pos]:
            print(f"\033[42m{user_guess[pos]}\033[0m", end='')
        elif user_guess[pos] in actual_word:
            print(f"\033[43m{user_guess[pos]}\033[0m", end='')
        else:
            print(f"{user_guess[pos]}", end='')
    print("")
