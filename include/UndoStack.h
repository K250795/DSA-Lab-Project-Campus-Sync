#pragma once
#include "DoublyLinkedList.h"

// Internal LIFO stack node: owns a deep copy of a DoublyLinkedList state.
struct StackNode {
    DoublyLinkedList listSnapshot;  // deep copy stored by value (Rule of Three handles it)
    StackNode*       below;         // pointer to the previous stack frame

    explicit StackNode(const DoublyLinkedList& snapshot)
        : listSnapshot(snapshot), below(nullptr) {}
};

// Pointer-based LIFO Undo Stack.
//
// Each push() stores a deep copy of the entire schedule list so that
// pop() can restore it exactly.  Proper destructor cleanup prevents leaks.
class UndoStack {
public:
    UndoStack();
    ~UndoStack();

    // Disallow copying (snapshots would need their own deep copy semantics).
    UndoStack(const UndoStack&)            = delete;
    UndoStack& operator=(const UndoStack&) = delete;

    // Push a snapshot of 'list' onto the stack.
    void push(const DoublyLinkedList& list);

    // Pop the top snapshot and return it as a DoublyLinkedList (by value).
    // Caller must ensure the stack is not empty before calling.
    DoublyLinkedList pop();

    // Inspect the top snapshot without removing it.
    const DoublyLinkedList& top() const;

    // Delete every snapshot and leave the stack empty (safe to reuse).
    void clear();

    bool empty() const { return top_ == nullptr; }
    int  size()  const { return size_; }

private:
    StackNode* top_;
    int        size_;
};
