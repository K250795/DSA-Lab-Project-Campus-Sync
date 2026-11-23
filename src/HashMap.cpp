#include "HashMap.h"

// ----------------------------------------------------------- constructor ---
// Every bucket starts as an empty chain.
HashMap::HashMap(int initialCapacity)
    : buckets_(initialCapacity < 1 ? 1 : initialCapacity, nullptr), size_(0) {}

// ----------------------------------------------------------------- insert ---
// Scan the key's chain: overwrite on a match, otherwise prepend a new entry.
bool HashMap::insert(int key, int value) {
    int idx = bucketIndex(key);
    for (HashEntry* cur = buckets_[idx]; cur != nullptr; cur = cur->next) {
        if (cur->key == key) {
            cur->value = value;           // existing key: update in place
            return false;
        }
    }
    buckets_[idx] = new HashEntry(key, value, buckets_[idx]);   // new chain head
    ++size_;
    return true;
}

// ------------------------------------------------------------------- find ---
bool HashMap::find(int key, int& outValue) const {
    for (HashEntry* cur = buckets_[bucketIndex(key)]; cur != nullptr; cur = cur->next) {
        if (cur->key == key) {
            outValue = cur->value;
            return true;
        }
    }
    return false;
}

// --------------------------------------------------------------- contains ---
bool HashMap::contains(int key) const {
    int ignored = 0;
    return find(key, ignored);
}

// ----------------------------------------------------------------- remove ---
// Walk the chain keeping a pointer to the previous entry so the match can be
// unlinked whether it is the chain head or further down.
bool HashMap::remove(int key) {
    int idx = bucketIndex(key);
    HashEntry* prev = nullptr;
    for (HashEntry* cur = buckets_[idx]; cur != nullptr; prev = cur, cur = cur->next) {
        if (cur->key == key) {
            if (prev == nullptr)
                buckets_[idx] = cur->next;   // removing the chain head
            else
                prev->next = cur->next;      // bypass the middle/tail entry
            delete cur;
            --size_;
            return true;
        }
    }
    return false;
}

// ------------------------------------------------------------ bucketIndex ---
int HashMap::bucketIndex(int key) const {
    return hashKey(key, capacity());
}

// ---------------------------------------------------------------- hashKey ---
// C++ '%' keeps the sign of the left operand (-3 % 8 == -3), so a negative
// remainder is shifted up by 'cap' to land in [0, cap).
int HashMap::hashKey(int key, int cap) {
    int h = key % cap;
    return h < 0 ? h + cap : h;
}
