#pragma once
#include <string>

// One schedule event. Times are minutes since midnight (08:30 -> 510), so
// ordering and overlap checks are plain integer comparisons.
struct ScheduleItem {
    std::string title;
    int         startMin;
    int         endMin;
    int         locationId;  // campus node id
};

// "HH:MM" -> minutes since midnight; throws std::invalid_argument on bad input.
int timeToMinutes(const std::string& hhmm);

// Minutes since midnight -> "HH:MM"; throws if outside [0, 1439].
std::string minutesToTime(int minutes);

// Ranges are half-open [start, end): 09:00-09:30 does not clash with 09:30-10:00.
bool itemsOverlap(const ScheduleItem& a, const ScheduleItem& b);

struct DLLNode {
    ScheduleItem data;
    DLLNode*     prev;
    DLLNode*     next;

    explicit DLLNode(const ScheduleItem& item)
        : data(item), prev(nullptr), next(nullptr) {}
};

// Doubly linked list of ScheduleItems with O(1) insert/remove at a known node.
// Rule of Three: copies are deep.
class DoublyLinkedList {
public:
    DoublyLinkedList();
    DoublyLinkedList(const DoublyLinkedList& other);
    DoublyLinkedList& operator=(const DoublyLinkedList& other);
    ~DoublyLinkedList();

    // All inserts return the new node.
    DLLNode* pushBack(const ScheduleItem& item);
    DLLNode* pushFront(const ScheduleItem& item);
    DLLNode* insertBefore(DLLNode* node, const ScheduleItem& item);
    DLLNode* insertAfter(DLLNode* node, const ScheduleItem& item);

    // Keeps ascending startMin order; equal start times keep insertion order. O(n).
    DLLNode* insertSorted(const ScheduleItem& item);

    // Unlinks and deletes 'node'; the pointer is invalid afterwards.
    void remove(DLLNode* node);

    // First exact (case-sensitive) title match, or nullptr.
    DLLNode* findByTitle(const std::string& title) const;
    bool removeByTitle(const std::string& title);

    // First event that clashes with 'item', or nullptr.
    DLLNode* findOverlap(const ScheduleItem& item) const;
    bool hasOverlap(const ScheduleItem& item) const { return findOverlap(item) != nullptr; }

    void clear();

    DLLNode* head() const { return head_; }
    DLLNode* tail() const { return tail_; }

    bool empty() const { return head_ == nullptr; }
    int  size()  const { return size_; }

private:
    DLLNode* head_;
    DLLNode* tail_;
    int      size_;

    // Appends a deep copy of every node in 'other'; this list must be empty.
    void copyFrom(const DoublyLinkedList& other);
};
