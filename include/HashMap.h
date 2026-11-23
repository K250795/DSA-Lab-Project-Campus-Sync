#pragma once
#include <vector>

// A single key/value entry.  Entries that hash to the same bucket are
// chained together through 'next' (separate chaining).
struct HashEntry {
    int        key;
    int        value;
    HashEntry* next;

    HashEntry(int k, int v, HashEntry* n) : key(k), value(v), next(n) {}
};

// Custom hash map from int keys to int values.
//
// Design:
//   - Bucket array: std::vector<HashEntry*> used strictly as backing array
//     storage; each slot is the head of a manual singly linked chain.
//   - Hash function: key mod capacity (made non-negative for negative keys).
//   - No std::unordered_map / std::list used.
class HashMap {
public:
    static constexpr int DEFAULT_CAPACITY = 8;

    // Creates an empty map with 'initialCapacity' buckets (minimum 1).
    explicit HashMap(int initialCapacity = DEFAULT_CAPACITY);

    int  size()     const { return size_; }
    int  capacity() const { return static_cast<int>(buckets_.size()); }
    bool empty()    const { return size_ == 0; }

    // Insert 'key' -> 'value'.  If 'key' already exists its value is
    // overwritten.  Returns true if a new key was added, false if updated.
    bool insert(int key, int value);

    // Look up 'key'.  On success writes the value to 'outValue' and returns
    // true; returns false (outValue untouched) if the key is absent.
    bool find(int key, int& outValue) const;

    // True if 'key' is present.
    bool contains(int key) const;

    // Delete 'key'.  Returns true if it was present, false otherwise.
    bool remove(int key);

    // Index of the bucket 'key' maps to with the current capacity.
    // Public so tests can construct deliberate collisions.
    int bucketIndex(int key) const;

private:
    std::vector<HashEntry*> buckets_;   // bucket heads (nullptr = empty)
    int                     size_;      // number of stored entries

    // Simple modular hash: maps any int key into [0, cap).
    static int hashKey(int key, int cap);
};
