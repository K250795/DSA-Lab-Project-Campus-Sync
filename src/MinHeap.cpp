#include "MinHeap.h"
#include <stdexcept>

// ------------------------------------------------------------------ push ---
void MinHeap::push(int id, double priority) {
    data_.push_back({id, priority});
    heapifyUp(static_cast<int>(data_.size()) - 1);
}

// ------------------------------------------------------------------- pop ---
void MinHeap::pop() {
    if (data_.empty())
        throw std::underflow_error("MinHeap::pop() called on empty heap");

    // Move the last element to the root position, then shrink.
    data_[0] = data_.back();
    data_.pop_back();

    if (!data_.empty())
        heapifyDown(0);
}

// ------------------------------------------------------------------- top ---
MinHeapNode MinHeap::top() const {
    if (data_.empty())
        throw std::underflow_error("MinHeap::top() called on empty heap");
    return data_[0];
}

// ----------------------------------------------------------------- empty ---
bool MinHeap::empty() const {
    return data_.empty();
}

// ------------------------------------------------------------------ size ---
int MinHeap::size() const {
    return static_cast<int>(data_.size());
}

// ------------------------------------------------------------ heapifyUp ---
// Bubble element at index 'i' up until parent priority <= child priority.
void MinHeap::heapifyUp(int i) {
    while (i > 0) {
        int p = parent(i);
        if (data_[p].priority > data_[i].priority) {
            // parent is larger → swap to maintain min-heap property
            MinHeapNode tmp = data_[p];
            data_[p]        = data_[i];
            data_[i]        = tmp;
            i = p;
        } else {
            break;
        }
    }
}

// ---------------------------------------------------------- heapifyDown ---
// Sink element at index 'i' down until it is <= both children.
void MinHeap::heapifyDown(int i) {
    int n = static_cast<int>(data_.size());
    while (true) {
        int smallest = i;
        int l = leftChild(i);
        int r = rightChild(i);

        if (l < n && data_[l].priority < data_[smallest].priority)
            smallest = l;
        if (r < n && data_[r].priority < data_[smallest].priority)
            smallest = r;

        if (smallest == i)
            break;   // already in correct position

        MinHeapNode tmp   = data_[i];
        data_[i]          = data_[smallest];
        data_[smallest]   = tmp;
        i = smallest;
    }
}
