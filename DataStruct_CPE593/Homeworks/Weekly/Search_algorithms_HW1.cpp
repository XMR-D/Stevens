/******************************************************************************

                              Online C++ Compiler.
               Code, Compile, Run and Debug C++ program online.
Write your code in this editor and press "Run" button to compile and execute it.

*******************************************************************************/

#include <iostream>
#include <array>

using namespace std;


int BSearch(array<array<int, 4>, 3> grid, int element, int low, int high)
{
    if (low > high)
        return 0;
        
    int mid = (low + high) / 2;
    
    
    int cols = grid[0].size();  // number of columns
    int n = mid / cols;         
    int m = mid % cols; 
    
    if (grid[n][m] == element)
        return 1;
    
    else if (grid[n][m] < element)
        return BSearch(grid, element, mid + 1, high);
    else
        return BSearch(grid, element, low, mid - 1);

}

int main()
{
    array<array<int, 4>, 3> arr = {{
        {{1, 2, 5, 7}},
        {{10, 14, 16, 20}},
        {{23, 32, 34, 80}}
    }};
    
    int target = 2;
    cout << "Target: " << target << endl;
    if (BSearch(arr, target, 0, 9))
        cout << "True" << endl;
    else
        cout << "False" << endl;
        
    target = 3;
    cout << "Target: " << target << endl;
    if (BSearch(arr, target, 0, 9))
        cout << "True" << endl;
    else
        cout << "False" << endl;

}