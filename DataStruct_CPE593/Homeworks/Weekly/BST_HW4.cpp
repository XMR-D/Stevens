#include <iostream>
#include <sstream>
#include <queue>
#include <climits>
using namespace std;

// Node structure
struct Node {
    int data;
    Node *left, *right;

    Node(int val) {
        data = val;
        left = NULL;
        right = NULL;
    }
};



// Class to check if a tree is a valid BST
class Solution {
public:
    bool CheckTree(Node *root, int maxNode, int minNode) {
        
        if (root == NULL)
            return 1;

        if (root->data > maxNode || root->data < minNode)
            return 0;

        if (!CheckTree(root->left, root->data, minNode))
            return 0;
            
        if (!CheckTree(root->right, maxNode, root->data))
            return 0;
            
        return 1;
    }
    
    bool isValidBST(Node *root) {
        
        if (root == NULL)
            return 1;
        
        int minNode = -1;
        int maxNode = root->data;
        
        return CheckTree(root->left, maxNode, minNode);
        
    }

};

// Function to create a binary tree from a space-separated string of node values
Node* createTreeFromInput(const string& input) {
    stringstream ss(input);
    int value;
    ss >> value;

    if (value == -1) return NULL;  // If the root is -1, return NULL

    Node* root = new Node(value);
    queue<Node*> q;
    q.push(root);

    while (!q.empty()) {
        Node* current = q.front();
        q.pop();

        // Left child
        if (ss >> value) {
            if (value != -1) {
                current->left = new Node(value);
                q.push(current->left);
            }
        }

        // Right child
        if (ss >> value) {
            if (value != -1) {
                current->right = new Node(value);
                q.push(current->right);
            }
        }
    }

    return root;
}

// Main function
int main() {
    Solution solution;

    cout << "Enter tree values in level order (space-separated), use -1 for NULL nodes:\n";
    string input;
    getline(cin, input);  // Take a whole line of input

    Node* root = createTreeFromInput(input);

    // Check if the tree is a valid BST
    if (solution.isValidBST(root)) {
        cout << "The tree is a valid BST.\n";
    } else {
        cout << "The tree is NOT a valid BST.\n";
    }

    return 0;
}