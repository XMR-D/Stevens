#include <iostream>
#include <vector>
#include <cstddef>

using namespace std;

struct ListNode {
    int val;
    ListNode *next;
    ListNode(int x) : val(x), next(nullptr) {}
};

class Solution {
    public:
        ListNode *detectCycle(ListNode *head) {
      
        ListNode * slow = head;
        ListNode * fast = head;
        int step = 2;
        int cycle_detected = 0;

        while (fast != nullptr)
        {
            /* Move fast ptr two steps forward */
            while (fast && step > 0)
            {
                fast = fast->next;
                step--;
            }
            
            /* If fast is NULL we did not found any cycle */
            if (fast == NULL)
                break;
                
            /* Else move slow one step forward */
            if (slow)
                slow = slow->next;
            
            /* 
             * If after movement fast == slow then cycle is detected 
             * So we break the loop.
             */
            if (fast == slow)
            {
                cycle_detected++;
                break;
            }
            
            step = 2;
            
        }
        if (cycle_detected)
        {
            /* Reset slow to head and make both ptr step forward until they meet */
            slow = head;
            
            while (fast != slow)
            {
                slow = slow->next;
                fast = fast->next;
            }
            
            return slow;
        }
        return NULL;
    }
};

// Helper function to create a cycle in the linked list
void createCycle(ListNode* head, int pos) {
    if (!head || pos == -1) return;

    ListNode* tail = head;
    ListNode* cycleNode = nullptr;
    int index = 0;

    while (tail->next) {
        if (index == pos) {
            cycleNode = tail;
        }
        tail = tail->next;
        index++;
    }

    if (cycleNode) {
        tail->next = cycleNode; // Create a cycle
    }
}

int main() {
    int n, cyclePos;
    std::cout << "Enter the number of nodes in the linked list: ";
    std::cin >> n;

    if (n <= 0) {
        std::cout << "No nodes to create a linked list." << std::endl;
        return 0;
    }

    std::vector<int> values(n);
    std::cout << "Enter the values of the nodes: ";
    for (int i = 0; i < n; ++i) {
        std::cin >> values[i];
    }

    std::cout << "Enter the position to create a cycle (-1 for no cycle): ";
    std::cin >> cyclePos;

    // Create the linked list
    ListNode* head = new ListNode(values[0]);
    ListNode* current = head;
    for (int i = 1; i < n; ++i) {
        current->next = new ListNode(values[i]);
        current = current->next;
    }

    // Create the cycle if specified
    createCycle(head, cyclePos);

    // Detect the cycle
    Solution solution;
    ListNode* cycleStart = solution.detectCycle(head);

    if (cycleStart) {
        std::cout << "Cycle detected. Cycle starts at node with value: " << cycleStart->val << std::endl;
    } else {
        std::cout << "No cycle detected." << std::endl;
    }

    return 0;
}