#include "DoublyLinkedList.h"
#include <stdexcept>
#include <cctype>

// Accepts exactly two digits, a colon and two digits.
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

// Half-open ranges intersect when each starts before the other ends.
bool itemsOverlap(const ScheduleItem& a, const ScheduleItem& b) {
    return a.startMin < b.endMin && b.startMin < a.endMin;
}

DoublyLinkedList::DoublyLinkedList()
    : head_(nullptr), tail_(nullptr), size_(0) {}

DoublyLinkedList::DoublyLinkedList(const DoublyLinkedList& other)
    : head_(nullptr), tail_(nullptr), size_(0) {
    copyFrom(other);
}

DoublyLinkedList& DoublyLinkedList::operator=(const DoublyLinkedList& other) {
    if (this != &other) {
        clear();
        copyFrom(other);
    }
    return *this;
}

DoublyLinkedList::~DoublyLinkedList() {
    clear();
}

DLLNode* DoublyLinkedList::pushBack(const ScheduleItem& item) {
    DLLNode* node = new DLLNode(item);
    if (tail_ == nullptr) {
        head_ = tail_ = node;
    } else {
        node->prev  = tail_;
        tail_->next = node;
        tail_       = node;
    }
    ++size_;
    return node;
}

DLLNode* DoublyLinkedList::pushFront(const ScheduleItem& item) {
    DLLNode* node = new DLLNode(item);
    if (head_ == nullptr) {
        head_ = tail_ = node;
    } else {
        node->next  = head_;
        head_->prev = node;
        head_       = node;
    }
    ++size_;
    return node;
}

DLLNode* DoublyLinkedList::insertBefore(DLLNode* node, const ScheduleItem& item) {
    if (node == nullptr)
        throw std::invalid_argument("DoublyLinkedList::insertBefore: null node");

    DLLNode* newNode = new DLLNode(item);
    newNode->next    = node;
    newNode->prev    = node->prev;

    if (node->prev != nullptr)
        node->prev->next = newNode;
    else
        head_ = newNode;

    node->prev = newNode;
    ++size_;
    return newNode;
}

DLLNode* DoublyLinkedList::insertAfter(DLLNode* node, const ScheduleItem& item) {
    if (node == nullptr)
        throw std::invalid_argument("DoublyLinkedList::insertAfter: null node");

    DLLNode* newNode  = new DLLNode(item);
    newNode->prev     = node;
    newNode->next     = node->next;

    if (node->next != nullptr)
        node->next->prev = newNode;
    else
        tail_ = newNode;

    node->next = newNode;
    ++size_;
    return newNode;
}

// Skip every node starting at or before the new one (<= keeps ties stable),
// then insert in front of the first later node.
DLLNode* DoublyLinkedList::insertSorted(const ScheduleItem& item) {
    DLLNode* cur = head_;
    while (cur != nullptr && cur->data.startMin <= item.startMin)
        cur = cur->next;

    if (cur == nullptr)
        return pushBack(item);

    return insertBefore(cur, item);
}

void DoublyLinkedList::remove(DLLNode* node) {
    if (node == nullptr)
        throw std::invalid_argument("DoublyLinkedList::remove: null node");

    if (node->prev != nullptr)
        node->prev->next = node->next;
    else
        head_ = node->next;

    if (node->next != nullptr)
        node->next->prev = node->prev;
    else
        tail_ = node->prev;

    delete node;
    --size_;
}

DLLNode* DoublyLinkedList::findByTitle(const std::string& title) const {
    for (DLLNode* cur = head_; cur != nullptr; cur = cur->next)
        if (cur->data.title == title)
            return cur;
    return nullptr;
}

// Scans the whole list: pushBack/pushFront/insertAfter don't keep it sorted,
// so stopping early on start time would be unsafe.
DLLNode* DoublyLinkedList::findOverlap(const ScheduleItem& item) const {
    for (DLLNode* cur = head_; cur != nullptr; cur = cur->next)
        if (itemsOverlap(cur->data, item))
            return cur;
    return nullptr;
}

bool DoublyLinkedList::removeByTitle(const std::string& title) {
    DLLNode* node = findByTitle(title);
    if (node == nullptr)
        return false;
    remove(node);
    return true;
}

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

void DoublyLinkedList::copyFrom(const DoublyLinkedList& other) {
    for (DLLNode* cur = other.head_; cur != nullptr; cur = cur->next)
        pushBack(cur->data);
}
