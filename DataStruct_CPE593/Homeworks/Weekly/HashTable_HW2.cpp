#include <iostream>
#include <vector>
#include <unordered_set>

using namespace std;



class Solution {
    
    
    public:
        bool isValidSudoku(vector<vector<char>>& board) {   
            // write your code here
            int box_index;
            
            unordered_set<char> rows[9];
            unordered_set<char> cols[9];
            unordered_set<char> boxes[9];
            
            for (int i = 0; i < 9; i++)
            {
                
                for (int j = 0; j < 9; j++)
                {
                    //the value is blank, skip it
                    if (board[i][j] == '.')
                        continue;
                        
                    //Check if the value is valid
                    if (rows[i].count(board[i][j]) > 0)
                        return 0;
                        
                    if (cols[j].count(board[i][j]) > 0)
                        return 0;
                        
                    box_index = (i/3)*3+j/3;
                    
                    if (boxes[box_index].count(board[i][j]) > 0)
                        return 0;
                        
                    //if it is valid, insert it onto all the table
                    rows[i].insert(board[i][j]);
                    cols[j].insert(board[i][j]);
                    boxes[box_index].insert(board[i][j]);
                    
                        
                    
                }
            }
            
            return 1;
        };
};

int main() {
    Solution sol;
    // Updated Sudoku board as per your input
    vector<vector<char>> board = {
    {'9', '3', '.', '.', '7', '.', '.', '.', '.'},
    {'6', '.', '.', '1', '9', '5', '.', '.', '.'},
    {'.', '5', '8', '.', '.', '.', '.', '6', '.'},
    {'8', '.', '.', '.', '6', '.', '.', '.', '3'},
    {'4', '.', '.', '8', '.', '3', '.', '.', '1'},
    {'7', '.', '.', '.', '2', '.', '.', '.', '6'},
    {'.', '6', '.', '.', '.', '.', '2', '7', '.'},
    {'.', '.', '.', '4', '1', '9', '.', '.', '5'},
    {'.', '.', '.', '.', '8', '.', '.', '7', '9'}
    };

    bool result = sol.isValidSudoku(board);
    
    if (result) {
        cout << The Sudoku board is valid. << endl;
    } else {    
        cout << The Sudoku board is invalid. << endl;
    }
    return 0;
}

