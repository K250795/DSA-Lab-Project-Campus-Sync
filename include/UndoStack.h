#pragma once
#include "DoublyLinkedList.h"

struct StackNode {
    DoublyLinkedList listSnapshot;  // deep copy, owned by value
    StackNode*       below;

    explicit StackNode(const DoublyLinkedList& snapshot)
        : listSnapshot(snapshot), below(nullptr) {}
};

// Linked LIFO stack of full schedule snapshots. Push and pop each copy the
// whole list, so they cost O(n) for an n-event schedule.
class UndoStack {
public:
    UndoStack();
    ~UndoStack();

    UndoStack(const UndoStack&)            = delete;
    UndoStack& operator=(const UndoStack&) = delete;

    void push(const DoublyLinkedList& list);

    // Removes and returns the top snapshot; throws std::underflow_error if empty.
    DoublyLinkedList pop();

    // Throws std::underflow_error if empty.
    const DoublyLinkedList& top() const;

    void clear();

    bool empty() const { return top_ == nullptr; }
    int  size()  const { return size_; }

private:
    StackNode* top_;
    int        size_;
};
