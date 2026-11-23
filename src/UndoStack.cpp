#include "UndoStack.h"
#include <stdexcept>

UndoStack::UndoStack()
    : top_(nullptr), size_(0) {}

UndoStack::~UndoStack() {
    clear();
}

// Deleting a StackNode also frees its snapshot via the list's destructor.
void UndoStack::clear() {
    StackNode* cur = top_;
    while (cur != nullptr) {
        StackNode* below = cur->below;
        delete cur;
        cur = below;
    }
    top_  = nullptr;
    size_ = 0;
}

void UndoStack::push(const DoublyLinkedList& list) {
    StackNode* node = new StackNode(list);   // deep copy
    node->below     = top_;
    top_            = node;
    ++size_;
}

DoublyLinkedList UndoStack::pop() {
    if (top_ == nullptr)
        throw std::underflow_error("UndoStack::pop() called on empty stack");

    StackNode* node = top_;
    top_            = node->below;
    --size_;

    // Copy the snapshot out before deleting the node that owns it.
    DoublyLinkedList snapshot = node->listSnapshot;
    delete node;
    return snapshot;
}

const DoublyLinkedList& UndoStack::top() const {
    if (top_ == nullptr)
        throw std::underflow_error("UndoStack::top() called on empty stack");
    return top_->listSnapshot;
}
