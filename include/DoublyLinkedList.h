#pragma once
#include <string>

// A single schedule event stored inside the doubly linked list.
struct ScheduleItem {
    std::string title;
    std::string startTime;   // e.g. "08:00"
    std::string endTime;     // e.g. "09:30"
    int         locationId;  // references a campus node id
};

// Internal doubly-linked node.
struct DLLNode {
    ScheduleItem data;
    DLLNode*     prev;
    DLLNode*     next;

    explicit DLLNode(const ScheduleItem& item)
        : data(item), prev(nullptr), next(nullptr) {}
};

// Pointer-based Doubly Linked List that stores ScheduleItem entries.
//
// Design goals:
//   - O(1) insertion / deletion given a pointer to the node.
//   - Rule of Three: destructor, deep-copy constructor, copy-assignment.
//   - No STL list / deque usage.
class DoublyLinkedList {
public:
    DoublyLinkedList();
    DoublyLinkedList(const DoublyLinkedList& other);           // deep copy
    DoublyLinkedList& operator=(const DoublyLinkedList& other); // deep copy
    ~DoublyLinkedList();

    // Append a new item at the tail.  Returns a pointer to the new node.
    DLLNode* pushBack(const ScheduleItem& item);

    // Insert 'item' immediately after 'node'.
    // Returns a pointer to the newly created node.
    DLLNode* insertAfter(DLLNode* node, const ScheduleItem& item);

    // Remove and destroy the given node.  The pointer is invalidated.
    void remove(DLLNode* node);

    // Remove and destroy all nodes.
    void clear();

    // Traversal helpers.
    DLLNode* head() const { return head_; }
    DLLNode* tail() const { return tail_; }

    bool empty() const { return head_ == nullptr; }
    int  size()  const { return size_; }

private:
    DLLNode* head_;
    DLLNode* tail_;
    int      size_;

    // Helper: copy all nodes from 'other' into this (assumed empty).
    void copyFrom(const DoublyLinkedList& other);
};
