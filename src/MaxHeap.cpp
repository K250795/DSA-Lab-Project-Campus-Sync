#include "MaxHeap.h"
#include <stdexcept>

// ------------------------------------------------------------------ push ---
void MaxHeap::push(int id, double score) {
    data_.push_back({id, score});
    heapifyUp(static_cast<int>(data_.size()) - 1);
}

// ------------------------------------------------------------------- pop ---
void MaxHeap::pop() {
    if (data_.empty())
        throw std::underflow_error("MaxHeap::pop() called on empty heap");

    // Move the last element to the root position, then shrink.
    data_[0] = data_.back();
    data_.pop_back();

    if (!data_.empty())
        heapifyDown(0);
}

// ------------------------------------------------------------------- top ---
MaxHeapNode MaxHeap::top() const {
    if (data_.empty())
        throw std::underflow_error("MaxHeap::top() called on empty heap");
    return data_[0];
}

// ----------------------------------------------------------------- empty ---
bool MaxHeap::empty() const {
    return data_.empty();
}

// ------------------------------------------------------------------ size ---
int MaxHeap::size() const {
    return static_cast<int>(data_.size());
}

// ------------------------------------------------------------ heapifyUp ---
// Bubble element at index 'i' up until parent score >= child score.
void MaxHeap::heapifyUp(int i) {
    while (i > 0) {
        int p = parent(i);
        if (data_[p].score < data_[i].score) {
            // parent is smaller → swap to maintain max-heap property
            MaxHeapNode tmp = data_[p];
            data_[p]        = data_[i];
            data_[i]        = tmp;
            i = p;
        } else {
            break;
        }
    }
}

// ---------------------------------------------------------- heapifyDown ---
// Sink element at index 'i' down until it is >= both children.
void MaxHeap::heapifyDown(int i) {
    int n = static_cast<int>(data_.size());
    while (true) {
        int largest = i;
        int l = leftChild(i);
        int r = rightChild(i);

        if (l < n && data_[l].score > data_[largest].score)
            largest = l;
        if (r < n && data_[r].score > data_[largest].score)
            largest = r;

        if (largest == i)
            break;   // already in correct position

        MaxHeapNode tmp  = data_[i];
        data_[i]         = data_[largest];
        data_[largest]   = tmp;
        i = largest;
    }
}
