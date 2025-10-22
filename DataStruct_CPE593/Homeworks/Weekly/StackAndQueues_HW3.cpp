#include <iostream>
#include <cstring>
#include <vector>

using namespace std;

int stack[200] = {0};
int stack_top = -1;

class Solution {
public:

    static int pushStack(int val) {
        
        if (stack_top == 199)
        {
            cout << "Can't push, stack full" << endl;
            return -1;
        }
        else
        {
            stack[stack_top] = val;
            stack_top++;
            return 0;
        }
        
    }
    
    static int popStack() {
        if (stack_top == -1)
        {
            return -1;
        }
        
        else 
        {
            int ret = stack[stack_top - 1];
            stack[stack_top - 1] = 0;
            stack_top--;
            return ret;
        }
    }
    
    static int IsStackEmpty()
    {
        return (stack_top == -1 ? 1 : 0);
    }
    
    static void HandleCollision(int ast_from_stack, int ast_from_vec)
    {
        
        
        // Colision
        if (ast_from_stack > 0 && ast_from_vec < 0)
        {
            if ((ast_from_vec * -1) == ast_from_stack)
                return;
            
            while (ast_from_stack > 0 || !IsStackEmpty())
            {
                // the negative one is smaller hence it is destroyed
                if ((ast_from_vec * -1) < ast_from_stack)
                {
                    pushStack(ast_from_stack);
                    break;
                }
                else if ((ast_from_vec * -1) == ast_from_stack)
                    break;
                else
                    ast_from_stack = popStack();
            }
            
            if (IsStackEmpty())
                pushStack(ast_from_vec);
        }
        else // In any other cases no colision
        {
            pushStack(ast_from_stack);
            pushStack(ast_from_vec);
        }
    }
    

    vector<int> asteroidCollision(vector<int>& asteroids) {
        long unsigned int size = asteroids.size();
        
        
        for (int i = 0; i <= size; i++)
        {
            if (IsStackEmpty())
                pushStack(asteroids[i]);
            else
            {
                int ast_from_stack = popStack();
                HandleCollision(ast_from_stack, asteroids[i]);
            }
        }
        
        vector<int> ret;
        
        while (!IsStackEmpty())
        {
            int val = popStack();
            
            if (val != 0)
                ret.emplace(ret.begin(), val);
        }
        return ret;
        
    }
    
};

void runTestCases() {
    Solution solution;
    vector<vector<int>> testCases = {
        {5, 10, -5},       // Case 1: 5 and 10 survive
        {8, -8},           // Case 2: Both destroy each other
        {10, 2, -5},       // Case 3: 10 survives, -5 is destroyed
        {-5, -10, 10, 5},  // Case 4: -5, -10 survive, 10 and 5 survive separately
        {3, 9, -2, -10}    // Case 5: 3 survives, 9 vs -2 (9 survives), 9 vs -10 (-10 survives)
    };

    for (const auto& testCase : testCases) {
        vector<int> result = solution.asteroidCollision(const_cast<vector<int>&>(testCase));
        cout << "Input: ";
        for (int asteroid : testCase) {
            cout << asteroid << " ";
        }
        cout << "\nOutput: ";
        for (int asteroid : result) {
            cout << asteroid << " ";
        }
        cout << "\n---------------------\n";
    }
}

int main() {
    runTestCases();
    return 0;
}