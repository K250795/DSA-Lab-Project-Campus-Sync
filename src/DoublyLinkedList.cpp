#include "DoublyLinkedList.h"
#include <stdexcept>

// ----------------------------------------------------------- constructor ---
DoublyLinkedList::DoublyLinkedList()
    : head_(nullptr), tail_(nullptr), size_(0) {}

// -------------------------------------------------- copy constructor ---
DoublyLinkedList::DoublyLinkedList(const DoublyLinkedList& other)
    : head_(nullptr), tail_(nullptr), size_(0) {
    copyFrom(other);
}

// ------------------------------------------------- copy assignment ---
DoublyLinkedList& DoublyLinkedList::operator=(const DoublyLinkedList& other) {
    if (this != &other) {
        clear();
        copyFrom(other);
    }
    return *this;
}

// ------------------------------------------------------------- destructor ---
DoublyLinkedList::~DoublyLinkedList() {
    clear();
}

// --------------------------------------------------------------- pushBack ---
DLLNode* DoublyLinkedList::pushBack(const ScheduleItem& item) {
    DLLNode* node = new DLLNode(item);
    if (tail_ == nullptr) {
        // List is empty: new node becomes both head and tail.
        head_ = tail_ = node;
    } else {
        node->prev  = tail_;
        tail_->next = node;
        tail_       = node;
    }
    ++size_;
    return node;
}

// ----------------------------------------------------------- insertAfter ---
DLLNode* DoublyLinkedList::insertAfter(DLLNode* node, const ScheduleItem& item) {
    if (node == nullptr)
        throw std::invalid_argument("DoublyLinkedList::insertAfter: null node");

    DLLNode* newNode  = new DLLNode(item);
    newNode->prev     = node;
    newNode->next     = node->next;

    if (node->next != nullptr)
        node->next->prev = newNode;
    else
        tail_ = newNode;   // inserting after the current tail

    node->next = newNode;
    ++size_;
    return newNode;
}

// ----------------------------------------------------------------- remove ---
void DoublyLinkedList::remove(DLLNode* node) {
    if (node == nullptr)
        throw std::invalid_argument("DoublyLinkedList::remove: null node");

    if (node->prev != nullptr)
        node->prev->next = node->next;
    else
        head_ = node->next;   // removing the head

    if (node->next != nullptr)
        node->next->prev = node->prev;
    else
        tail_ = node->prev;   // removing the tail

    delete node;
    --size_;
}

// ------------------------------------------------------------------ clear ---
void DoublyLinkedList::clear() {
    DLLNode* cur = head_;
    while (cur != nullptr) {
        DLLNode* next = cur->next;
        delete cur;
        cur = next;
    }
    head_ = tail_ = nullptr;
    size_ = 0;
}

// --------------------------------------------------------------- copyFrom ---
// Performs a full deep copy (all nodes) from 'other' into this empty list.
void DoublyLinkedList::copyFrom(const DoublyLinkedList& other) {
    for (DLLNode* cur = other.head_; cur != nullptr; cur = cur->next)
        pushBack(cur->data);
}
