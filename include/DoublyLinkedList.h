#pragma once
#include <string>

// A single schedule event stored inside the doubly linked list.
// Times are stored as minutes since midnight (e.g. 08:30 -> 510) so that
// ordering and overlap checks are plain integer comparisons.
struct ScheduleItem {
    std::string title;
    int         startMin;    // start time, minutes since midnight [0, 1439]
    int         endMin;      // end time,   minutes since midnight [0, 1439]
    int         locationId;  // references a campus node id
};

// Convert "HH:MM" (24-hour, e.g. "08:30") to minutes since midnight.
// Throws std::invalid_argument on malformed or out-of-range input.
int timeToMinutes(const std::string& hhmm);

// Convert minutes since midnight to zero-padded "HH:MM".
// Throws std::invalid_argument if minutes is outside [0, 1439].
std::string minutesToTime(int minutes);

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

    // Insert 'item' keeping the list ordered by ascending startMin.
    // Items with an equal start time keep insertion order (stable).
    // Returns a pointer to the newly created node.
    DLLNode* insertSorted(const ScheduleItem& item);

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
