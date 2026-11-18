#pragma once
#include <vector>

// Node stored in the Max-Heap.
// 'score' determines heap order: higher value = higher priority.
struct MaxHeapNode {
    int    id;
    double score;
};

// Custom array-backed Max-Heap.
// Uses std::vector strictly as the underlying dynamic array storage.
// All heap logic (heapifyUp / heapifyDown) is implemented manually
// with 0-based index arithmetic.
class MaxHeap {
public:
    MaxHeap() = default;

    // Insert a new node and restore the heap property.
    void   push(int id, double score);

    // Remove the maximum element (root) and restore the heap property.
    void   pop();

    // Return (without removing) the maximum element.
    MaxHeapNode top() const;

    // Returns true when the heap contains no elements.
    bool   empty() const;

    // Number of elements currently in the heap.
    int    size() const;

private:
    std::vector<MaxHeapNode> data_;   // backing storage

    // Bubble the element at index 'i' upward until the heap property holds.
    void heapifyUp(int i);

    // Sink the element at index 'i' downward until the heap property holds.
    void heapifyDown(int i);

    // 0-based parent / child index helpers.
    static int parent(int i)      { return (i - 1) / 2; }
    static int leftChild(int i)   { return 2 * i + 1;   }
    static int rightChild(int i)  { return 2 * i + 2;   }
};
