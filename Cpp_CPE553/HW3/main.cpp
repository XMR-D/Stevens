#include <cstdlib>
#include <cstring>
#include <iostream>
#include <math.h>

using namespace std;

//PROBLEM 1 : ANSWER
static void
ReadElements(int *A, int n)
{
    cout << "Enter " << n;
    cout << " elements, additional elements will be ignored" << endl;

    int i = 0;

    for (int i = 0; i < n; i++) {
        cin >> A[i];
    }

}

static void
PrintElements(int *A, int n)
{
    for (int i = 0; i < n-1; i++) {
        cout << A[i] << ' ';
    }
    cout << A[n-1] << endl;
}

static void
PrintElementsRev(int *A, int n)
{
    for (int i = n-1; i > 0; i--) {
        cout << A[i] << ' ';
    }
    cout << A[0] << endl;
}

static void
OddsOnly(int *A, int n, int* e, int m)
{
    int * new_table = nullptr;
    for (int i = 0; i < n; i++) {

        if (A[i] % 2 != 0) {
            
            m++;
            new_table = new int[m];

            memcpy(new_table, e, (m-1)*sizeof(int));

            new_table[m-1] = A[i];
            delete [] e;
            e = new_table;
        }
    }

    for (int i = 0; i < m-1; i++) {
        cout << e[i] << ' ';
    }
    cout << e[m-1] << endl;
    
    delete [] e;

}

//PROBLEM 2 : ANSWER
static bool
LinearSearch(int* arr, int arr_size, int searchValue, int* index)
{
    for (int i = 0; i < arr_size; i++) {
        if (arr[i] == searchValue) {
            *index = i;
            return true;
        }
    }
    return false;
}

//PROBLEM 3 : ANSWER
static void
swapValues(int * a, int * b)
{
    int tmp = *b;

    *b = *a;
    *a = tmp;
}

int main() {

    cout << "########" << endl;
    cout << "Problem One" << endl;
    cout << "########" << endl;

    // read how many number of elements you want to create
    int n;
    cout << "Please enter number of elements: ";
    cin >> n;
    // Use function ReadElements() to read array (A) from the user with given
    //number of elements
    // define new empty array dynamically with n number as int *A = new int[n];
    
    
    // write your code here
    int* A = new int[n];

    ReadElements(A, n);
    // Use function PrintElements() that can print the entered array
    // write your code here
    cout << "Print Array" << endl;
    PrintElements(A, n);
    // Use function PrintElementsRev() that can print the array in reverse order
    // write your code here
    cout << "Print Reverse Array" << endl;
    PrintElementsRev(A, n);
    
    // Use function OddsOnly that can print only the odd elements of the user array
    // define a null pointer *e to use it with OddsOnly function
    // and int m equal zero
    // write your code here
    // you can use NULL here or nullptr for *e
    int* e = nullptr; // null pointer
    int m = 0; // number of odd elements
    // print odds array
    cout << "Print Odds Only Array" << endl;
    // Use function
    OddsOnly(A, n, e, m);

    // do not forget
    delete [] A; // Do not forget to deallocate dynamically allocated memory
    cout << "====[ end ]====" << endl;
    cout << " " << endl;
   

    cout << "########" << endl;
    cout << "Problem Two" << endl;
    cout << "########" << endl;
    // Read only this part of the problem
    //define the size of the array and the array
    const int N = 10; // Size of the array
    int arr[N];

    /* Initialize a random seed based on time entropy */
    srand(time(NULL));
    // Initialize the array with random values (for demonstration)
    for (int i = 0; i < N; i++) {
        //write code to generate random number between 0-99 and fill in the array
        /* use rand modulo 100 to generate a number between 0 and 99 inclusive*/
        arr[i] = rand() % 100;
    }
    // Display the original array
    cout << "Original Array: ";
    PrintElements(arr, N);
    cout << endl;

    int searchValue;
    cout << "Enter a value to search for: ";
    cin >> searchValue;

    bool found = false;
    int index = -1; // To store the index of the found value
    // Linear search algorithm
    //implement linear search algorithms

    found = LinearSearch(arr, N, searchValue, &index);

    if (found) {
        cout << "The value " << searchValue << " is found at index " << index
        << endl;
    } else {
        cout << "The value " << searchValue << " is not found in the array."
        << endl;
    }
    
    cout << "====[ end ]====" << endl;
    cout << " " << endl;

    cout << "########" << endl;
    cout << "Problem Three" << endl;
    cout << "########" << endl;

    int num1 = 0;
    int num2 = 0;
    
    cout << "Enter the first integer: ";
    cin >> num1;
    cout << "Enter the second integer: ";
    cin >> num2;
    cout << "Before swapping: num1 = " << num1 << ", num2 = " << num2 <<
    endl;
    // Call the swapValues function to swap the values of num1 and num2 using
    //pointers
    //define swap value function before main function
    swapValues(&num1, &num2);
    // Display the values after swapping
    cout << "After swapping: num1 = " << num1 << ", num2 = " << num2 <<
    endl;

    cout << "====[ end ]====" << endl;
    cout << " " << endl;
    
}