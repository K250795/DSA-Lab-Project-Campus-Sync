#include "UndoStack.h"
#include <stdexcept>

// ----------------------------------------------------------- constructor ---
UndoStack::UndoStack()
    : top_(nullptr), size_(0) {}

// ------------------------------------------------------------- destructor ---
UndoStack::~UndoStack() {
    // Walk the linked chain and delete every StackNode.
    // The DoublyLinkedList member inside each StackNode is destroyed
    // automatically by its own destructor (Rule of Three).
    StackNode* cur = top_;
    while (cur != nullptr) {
        StackNode* below = cur->below;
        delete cur;
        cur = below;
    }
}

// ------------------------------------------------------------------- push ---
void UndoStack::push(const DoublyLinkedList& list) {
    // StackNode constructor performs a deep copy via DoublyLinkedList's
    // copy constructor.
    StackNode* node = new StackNode(list);
    node->below     = top_;
    top_            = node;
    ++size_;
}

// -------------------------------------------------------------------- pop ---
DoublyLinkedList UndoStack::pop() {
    if (top_ == nullptr)
        throw std::underflow_error("UndoStack::pop() called on empty stack");

    StackNode* node = top_;
    top_            = node->below;
    --size_;

    // Move the snapshot out before deleting the StackNode so that the
    // DoublyLinkedList destructor inside the node does not destroy the data
    // we are about to return.  We rely on the copy constructor here.
    DoublyLinkedList snapshot = node->listSnapshot;  // deep copy (copy ctor)
    delete node;
    return snapshot;   // NRVO / move semantics will elide extra copies
}

// -------------------------------------------------------------------- top ---
const DoublyLinkedList& UndoStack::top() const {
    if (top_ == nullptr)
        throw std::underflow_error("UndoStack::top() called on empty stack");
    return top_->listSnapshot;
}
