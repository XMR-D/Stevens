# Author: Guillaume Wantiez
# Date: 02/09/2025
# Description: This program convert Hindu-arabic Numbers into Roman numerals
#              the program will prompt the user to enter a number between 1-9 inclusively
#              then perform the translation to roman number, if the number is outside the defined range
#              an appropriate message will be prointed out

def translate(num):
    # Translate the number using a small trick
    # Unicode of I is 2160, performing 2159 + (entered val) is sufficent.
    # then, to print it just use chr, hex is used to compute the correct val
    roman_char = 0x215F + num
    print(f"Your roman numeral is: {chr(roman_char)}.")
        

def ToRoman():
    raw_num = int(input("Please enter a number between 1 and 9 to convert to a roman numeral: "))
    if (raw_num > 9 or raw_num < 1):
        print(f"{raw_num} is outside the allowed range of 1-9.")
    else:
        translate(raw_num)


def main():
    ToRoman()

if __name__ == "__main__":
    main()