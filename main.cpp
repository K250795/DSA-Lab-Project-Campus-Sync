// main.cpp – sanity-check driver for all four data structures.
// All output lives here (UI layer); no cout/cin inside DS implementations.

#include <iostream>
#include <cassert>
#include <string>

#include "MinHeap.h"
#include "MaxHeap.h"
#include "DoublyLinkedList.h"
#include "UndoStack.h"
#include "PrefixTrie.h"

// ------------------------------------------------------------------ helpers --
static void section(const std::string& title) {
    std::cout << "\n========== " << title << " ==========\n";
}

static void ok(const std::string& msg) {
    std::cout << "  [OK] " << msg << '\n';
}

// ================================================================ MinHeap ===
static void testMinHeap() {
    section("MinHeap");

    MinHeap mh;
    assert(mh.empty());

    mh.push(10, 5.0);
    mh.push(20, 1.0);
    mh.push(30, 3.0);
    mh.push(40, 2.0);
    mh.push(50, 4.0);

    // Expected pop order: 1, 2, 3, 4, 5
    double expected[] = {1.0, 2.0, 3.0, 4.0, 5.0};
    for (double exp : expected) {
        MinHeapNode node = mh.top();
        assert(node.priority == exp);
        std::cout << "  pop id=" << node.id
                  << " priority=" << node.priority << '\n';
        mh.pop();
    }
    assert(mh.empty());
    ok("MinHeap push/pop/top/empty all correct");

    // Edge: single element
    mh.push(99, 0.5);
    assert(mh.top().id == 99);
    mh.pop();
    assert(mh.empty());
    ok("MinHeap single-element edge case correct");
}

// ================================================================ MaxHeap ===
static void testMaxHeap() {
    section("MaxHeap");

    MaxHeap mh;
    assert(mh.empty());

    mh.push(1, 10.0);
    mh.push(2, 50.0);
    mh.push(3, 30.0);
    mh.push(4, 20.0);
    mh.push(5, 40.0);

    // Expected pop order: 50, 40, 30, 20, 10
    double expected[] = {50.0, 40.0, 30.0, 20.0, 10.0};
    for (double exp : expected) {
        MaxHeapNode node = mh.top();
        assert(node.score == exp);
        std::cout << "  pop id=" << node.id
                  << " score=" << node.score << '\n';
        mh.pop();
    }
    assert(mh.empty());
    ok("MaxHeap push/pop/top/empty all correct");

    // Equal priorities: heap must remain stable (not crash).
    mh.push(7, 5.0);
    mh.push(8, 5.0);
    mh.push(9, 5.0);
    assert(mh.size() == 3);
    while (!mh.empty()) mh.pop();
    assert(mh.empty());
    ok("MaxHeap equal-priority edge case correct");
}

// ============================================ DoublyLinkedList + UndoStack ===
static void testDLLAndUndoStack() {
    section("DoublyLinkedList");

    DoublyLinkedList schedule;
    assert(schedule.empty());

    ScheduleItem e1{"CS101 Lecture",    "08:00", "09:30", 101};
    ScheduleItem e2{"Math Tutorial",    "10:00", "11:00", 102};
    ScheduleItem e3{"Lab Session",      "13:00", "15:00", 103};

    DLLNode* n1 = schedule.pushBack(e1);
    DLLNode* n2 = schedule.pushBack(e2);
    schedule.pushBack(e3);
    assert(schedule.size() == 3);
    ok("pushBack 3 items");

    // Insert between n1 and n2.
    ScheduleItem e4{"Office Hours", "09:30", "10:00", 104};
    DLLNode* n4 = schedule.insertAfter(n1, e4);
    assert(schedule.size() == 4);
    // Order: n1 -> n4 -> n2 -> n3
    assert(n1->next == n4);
    assert(n4->prev == n1);
    assert(n4->next == n2);
    assert(n2->prev == n4);
    ok("insertAfter correct (links verified)");

    // Deep copy.
    DoublyLinkedList copy = schedule;
    assert(copy.size() == 4);
    // Pointers should be distinct (deep copy).
    assert(copy.head() != schedule.head());
    ok("Deep copy constructor: size and pointer independence verified");

    // Remove middle node n4 from original; copy should be unaffected.
    schedule.remove(n4);
    assert(schedule.size() == 3);
    assert(copy.size() == 4);
    ok("remove() O(1) node deletion; copy unaffected");

    // Print remaining items.
    std::cout << "  Schedule after remove:\n";
    for (DLLNode* cur = schedule.head(); cur != nullptr; cur = cur->next)
        std::cout << "    " << cur->data.title << " ["
                  << cur->data.startTime << " - " << cur->data.endTime << "]\n";

    // ---- UndoStack ----
    section("UndoStack");

    UndoStack undo;
    assert(undo.empty());

    // Snapshot 1: 1 item.
    DoublyLinkedList s1;
    s1.pushBack(e1);
    undo.push(s1);
    ok("Pushed snapshot 1 (1 item)");

    // Snapshot 2: 2 items.
    s1.pushBack(e2);
    undo.push(s1);
    ok("Pushed snapshot 2 (2 items)");

    // Snapshot 3: 3 items.
    s1.pushBack(e3);
    undo.push(s1);
    ok("Pushed snapshot 3 (3 items)");

    assert(undo.size() == 3);

    // Pop and verify LIFO order.
    DoublyLinkedList restored3 = undo.pop();
    assert(restored3.size() == 3);
    ok("pop() restored snapshot 3 (3 items)");

    DoublyLinkedList restored2 = undo.pop();
    assert(restored2.size() == 2);
    ok("pop() restored snapshot 2 (2 items)");

    DoublyLinkedList restored1 = undo.pop();
    assert(restored1.size() == 1);
    ok("pop() restored snapshot 1 (1 item)");

    assert(undo.empty());
    ok("UndoStack LIFO order and deep-copy semantics verified");

    // Copy-assignment operator test.
    DoublyLinkedList dst;
    dst = copy;   // copy has 4 items
    assert(dst.size() == 4);
    dst.clear();
    assert(copy.size() == 4);  // copy must remain intact
    ok("Copy-assignment operator: independent after assign");
}

// ================================================================ PrefixTrie ===
static void testPrefixTrie() {
    section("PrefixTrie");

    PrefixTrie trie;

    trie.insert("Engineering Block",   1);
    trie.insert("Engineering Lab",     2);
    trie.insert("Admin Office",        3);
    trie.insert("Library",             4);
    trie.insert("Lecture Hall A",      5);
    trie.insert("Lecture Hall B",      6);
    trie.insert("Cafeteria",           7);

    // Exact search.
    assert(trie.searchExact("Library") == 4);
    assert(trie.searchExact("library") == 4);   // case-insensitive
    assert(trie.searchExact("CAFETERIA") == 7);
    assert(trie.searchExact("Unknown Room") == -1);
    ok("searchExact: case-insensitive hits and miss verified");

    // Autocomplete "eng" → should return both engineering entries.
    auto results = trie.autocomplete("eng");
    std::cout << "  autocomplete(\"eng\"):\n";
    for (auto& [name, id] : results)
        std::cout << "    key=\"" << name << "\" id=" << id << '\n';
    assert(results.size() == 2);
    ok("autocomplete(\"eng\") returned exactly 2 matches");

    // Autocomplete "lect" → Lecture Hall A and B.
    results = trie.autocomplete("lect");
    std::cout << "  autocomplete(\"lect\"):\n";
    for (auto& [name, id] : results)
        std::cout << "    key=\"" << name << "\" id=" << id << '\n';
    assert(results.size() == 2);
    ok("autocomplete(\"lect\") returned exactly 2 matches");

    // Autocomplete with no match.
    results = trie.autocomplete("xyz");
    assert(results.empty());
    ok("autocomplete(\"xyz\") correctly returns empty");

    // Autocomplete entire word (should return itself).
    results = trie.autocomplete("Library");
    assert(results.size() == 1);
    assert(results[0].second == 4);
    ok("autocomplete full word returns exactly itself");
}

// ===================================================================== main ===
int main() {
    std::cout << "Campus-Sync – Data Structure Sanity Checks\n";
    std::cout << "===========================================\n";

    testMinHeap();
    testMaxHeap();
    testDLLAndUndoStack();
    testPrefixTrie();

    std::cout << "\n===========================================\n";
    std::cout << "All sanity checks passed. Zero crashes.\n";
    return 0;
}
