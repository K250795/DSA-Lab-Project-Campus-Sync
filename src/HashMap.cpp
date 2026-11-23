#include "HashMap.h"

// ----------------------------------------------------------- constructor ---
// Every bucket starts as an empty chain.
HashMap::HashMap(int initialCapacity)
    : buckets_(initialCapacity < 1 ? 1 : initialCapacity, nullptr), size_(0) {}

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
