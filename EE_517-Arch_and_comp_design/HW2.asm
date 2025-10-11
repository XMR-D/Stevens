Exercise 2====================================================================================
//Using X1 for i and setting i to 0
Start:
    ADDI X1, XZR, #0
//Entering loop
Loop:
    //Setting up d[i] in X3
    //X1 is used for i and i is needed afterwards
    //X2 is just a temp reg
    LSL X2, X1, #3
    ADD X22, X22, X2
    LDUR X3, [X22, #0]
    //Comparing d[i] and b
    //If d[i] != b then exit else continue to if statement
    SUB X3, X3, X20
    CBNZ X3, Exit
    //a - i > 0 <=> a > i, 
    //if a <= i then go back to loop 
    //else increment i and go back to loop
    SUBS XZR, X19, X1
    B.LTE Loop
    ADDI X1, X1, #1
    BR Loop
    
Exit:



Start:
    //Set up d[a]
    LSL X1, X19, #3
    ADD X22, X22, X1

    //compare and branch to Else if a <= b
    SUBS XZR, X19, X20
    B.LTE Else

    //d[a] = b + 8 then exit
    ADDI X21, X21, #8
    STUR X21, [X22, #0]
    BR Exit
Else:
    //d[a] = b - 16 then exit
    SUBI X21, X21, #16
    STUR X21, [X22, #0]
Exit:


Exercise 3====================================================================================

Write a LEGv8 assembly code to find the largest and smallest of n non-zero positive integers.
Assume the value of n is stored in register X19 and each of the n values is stored in an array.
Each element of the array is 8 bytes long and the array’s base address is stored in register X20.
Store the largest and smallest values in register X21 and X22 respectively. Comment on your
assembly code. (20 Points)

Main:
   //X1 is i, X21 is largest, and X22 is smallest

   //CHANGE HERE TO INITIALIZE SMALLEST AND GREATEST TO FIRST VALUE !!!!!!
   ADDI X1, XZR, #0
   ADDI X21, XZR, #0
   ADDI X22, XZR, #0
Loop:
   //while loop start Check if n > 0, if not exit else continue
   CBZ X19, Exit
   //Setting up array[i] (array[i] will be in X4)
   LSL X2, X1, #3
   ADD X3, X20, X2
   LDUR X4, [X3, #0]

   //Compare smallest with array[i], if smallest > array[i] update value
   SUBS XZR, X22, X4
   B.GT Chgsmallest
//Chklarge is a label used to branch back here if we went to Chgsmallest
Chklarge:
   //Compare largest with array[i], if array[i] > largest update value
   SUBS XZR, X21, X4
   B.LT Chglargest
//i++ and n-- then go to start of loop
Endloop:
   ADDI X1, X1, #1
   SUBI X19, X19,XZR, X20
Chglargest:
   ADD X21, X4, XZR
   B Endloop
Exit:

Exercise 4 =====================================================================
//RECURSION ADD NUMS
add_nums:
   //Grow stack by 16 bytes (enough place for two 8 bytes words)
   //Store caller address and actual n
   SUBI SP, SP, #16
   STUR LR, [SP, #8]
   STUR X19, [SP, #0]

   //test if we need to stop recursion (n >= 1)
   SUBIS XZR, X19, #1
   B.GE recursion

   //We need to stop calling more recursion and start actual sum
   ADDI SP, SP, #16
   BR LR
recursion:
   //n = n-1 and call back add_nums to prepare for another recursion
   SUBI X19, x19, #1
   BL add_nums

   //If here operation is needed
   //Fetch back saved n and caller address from stack
   LDUR X19, [SP, #0]
   LDUR LR, [SP, #8]

   //Pop stack to align on next pair (n/caller address)
   ADDI SP, SP , #16

   //Add fetched n to result and go back to linked address
   ADD X20, X20, X19

   //Eventually exit of the whole function will be through here.
   BR LR
