#include "DoublyLinkedList.h"
#include <stdexcept>
#include <cctype>    // std::isdigit

// ---------------------------------------------------------- timeToMinutes ---
// Parses exactly "HH:MM" (two digits, colon, two digits).
int timeToMinutes(const std::string& hhmm) {
    if (hhmm.size() != 5 || hhmm[2] != ':')
        throw std::invalid_argument("timeToMinutes: expected \"HH:MM\"");

    const int digitPos[] = {0, 1, 3, 4};
    for (int pos : digitPos)
        if (!std::isdigit(static_cast<unsigned char>(hhmm[pos])))
            throw std::invalid_argument("timeToMinutes: non-digit in time");

    int hours   = (hhmm[0] - '0') * 10 + (hhmm[1] - '0');
    int minutes = (hhmm[3] - '0') * 10 + (hhmm[4] - '0');
    if (hours > 23 || minutes > 59)
        throw std::invalid_argument("timeToMinutes: time out of range");

    return hours * 60 + minutes;
}

// ---------------------------------------------------------- minutesToTime ---
// Builds a zero-padded "HH:MM" string by hand (no stream formatting).
std::string minutesToTime(int minutes) {
    if (minutes < 0 || minutes >= 24 * 60)
        throw std::invalid_argument("minutesToTime: minutes out of range");

    int hours = minutes / 60;
    int mins  = minutes % 60;

    std::string out = "00:00";
    out[0] = static_cast<char>('0' + hours / 10);
    out[1] = static_cast<char>('0' + hours % 10);
    out[3] = static_cast<char>('0' + mins / 10);
    out[4] = static_cast<char>('0' + mins % 10);
    return out;
}

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

// -------------------------------------------------------------- pushFront ---
DLLNode* DoublyLinkedList::pushFront(const ScheduleItem& item) {
    DLLNode* node = new DLLNode(item);
    if (head_ == nullptr) {
        // List is empty: new node becomes both head and tail.
        head_ = tail_ = node;
    } else {
        node->next  = head_;
        head_->prev = node;
        head_       = node;
    }
    ++size_;
    return node;
}

// ---------------------------------------------------------- insertBefore ---
DLLNode* DoublyLinkedList::insertBefore(DLLNode* node, const ScheduleItem& item) {
    if (node == nullptr)
        throw std::invalid_argument("DoublyLinkedList::insertBefore: null node");

    DLLNode* newNode = new DLLNode(item);
    newNode->next    = node;
    newNode->prev    = node->prev;

    if (node->prev != nullptr)
        node->prev->next = newNode;
    else
        head_ = newNode;   // inserting before the current head

    node->prev = newNode;
    ++size_;
    return newNode;
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

// ---------------------------------------------------------- insertSorted ---
// Walk from the head past every node whose start is <= the new start, then
// link the new node in front of the first node that starts later.
DLLNode* DoublyLinkedList::insertSorted(const ScheduleItem& item) {
    DLLNode* cur = head_;
    while (cur != nullptr && cur->data.startMin <= item.startMin)
        cur = cur->next;

    if (cur == nullptr)          // empty list, or new item starts last
        return pushBack(item);

    return insertBefore(cur, item);   // handles the new-head case too
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
