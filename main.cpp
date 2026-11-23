// main.cpp – sanity-check driver for all data structures and Graph engine.
// All output lives here (UI layer); no cout/cin inside DS/Graph implementations.

#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include <stdexcept>
#include <fstream>   // temporary CSV in testSchedulerCsv
#include <cstdio>    // std::remove
#include <cmath>     // std::abs
#include <iomanip>   // std::setw for aligned menu columns
#include <utility>   // std::pair
#include <cctype>    // std::tolower

#include "MinHeap.h"
#include "MaxHeap.h"
#include "DoublyLinkedList.h"
#include "UndoStack.h"
#include "Scheduler.h"
#include "HashMap.h"
#include "RoomAvailability.h"
#include "BunkerRadar.h"
#include "Speedrunner.h"
#include "PrefixTrie.h"
#include "CampusNode.h"
#include "Graph.h"

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

    // Time conversion helpers.
    assert(timeToMinutes("00:00") == 0);
    assert(timeToMinutes("08:30") == 510);
    assert(timeToMinutes("23:59") == 1439);
    assert(minutesToTime(510) == "08:30");
    assert(minutesToTime(0) == "00:00");
    assert(minutesToTime(timeToMinutes("13:05")) == "13:05");
    bool threw = false;
    try { timeToMinutes("24:00"); } catch (const std::invalid_argument&) { threw = true; }
    assert(threw);
    threw = false;
    try { timeToMinutes("8:30"); } catch (const std::invalid_argument&) { threw = true; }
    assert(threw);
    threw = false;
    try { minutesToTime(1440); } catch (const std::invalid_argument&) { threw = true; }
    assert(threw);
    ok("timeToMinutes / minutesToTime round-trip and reject bad input");

    ScheduleItem e1{"CS101 Lecture", timeToMinutes("08:00"), timeToMinutes("09:30"), 2};   // Academic Block A
    ScheduleItem e2{"Math Tutorial", timeToMinutes("10:00"), timeToMinutes("11:00"), 3};   // Academic Block B
    ScheduleItem e3{"Lab Session",   timeToMinutes("13:00"), timeToMinutes("15:00"), 5};   // CS Lab One

    DLLNode* n1 = schedule.pushBack(e1);
    DLLNode* n2 = schedule.pushBack(e2);
    schedule.pushBack(e3);
    assert(schedule.size() == 3);
    ok("pushBack 3 items");

    // Insert between n1 and n2.
    ScheduleItem e4{"Office Hours", timeToMinutes("09:30"), timeToMinutes("10:00"), 4};   // EE Block
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

    // insertSorted: insert out of order, expect ascending start times.
    DoublyLinkedList sorted;
    sorted.insertSorted(e3);                                    // 13:00 (empty list)
    sorted.insertSorted(e1);                                    // 08:00 (new head)
    sorted.insertSorted(e2);                                    // 10:00 (middle)
    ScheduleItem late{"Evening Study", timeToMinutes("16:00"), timeToMinutes("17:00"), 7};   // Library
    sorted.insertSorted(late);                                  // 16:00 (new tail)
    ScheduleItem tie{"Quiz", timeToMinutes("10:00"), timeToMinutes("10:15"), 6};   // CS Lab Two
    DLLNode* tieNode = sorted.insertSorted(tie);                // equal start -> after e2
    assert(sorted.size() == 5);
    assert(sorted.head()->data.title == "CS101 Lecture");
    assert(sorted.tail()->data.title == "Evening Study");
    assert(tieNode->prev->data.title == "Math Tutorial");
    int prevStart = -1;
    int count = 0;
    for (DLLNode* cur = sorted.head(); cur != nullptr; cur = cur->next) {
        assert(cur->data.startMin >= prevStart);
        assert(cur->next == nullptr || cur->next->prev == cur);   // back links intact
        prevStart = cur->data.startMin;
        ++count;
    }
    assert(count == sorted.size());
    ok("insertSorted keeps ascending order (head, middle, tail, ties stable)");

    // pushFront / insertBefore.
    DoublyLinkedList front;
    DLLNode* f2 = front.pushFront(e2);           // empty list: head == tail
    assert(front.head() == f2 && front.tail() == f2);
    DLLNode* f1 = front.pushFront(e1);           // new head
    assert(front.head() == f1 && f1->next == f2 && f2->prev == f1);
    assert(f1->prev == nullptr);
    DLLNode* fb = front.insertBefore(f2, e4);    // middle: f1 -> fb -> f2
    assert(f1->next == fb && fb->prev == f1 && fb->next == f2 && f2->prev == fb);
    DLLNode* fh = front.insertBefore(f1, e3);    // before head -> new head
    assert(front.head() == fh && fh->prev == nullptr && fh->next == f1 && f1->prev == fh);
    assert(front.tail() == f2 && front.size() == 4);
    threw = false;
    try { front.insertBefore(nullptr, e1); } catch (const std::invalid_argument&) { threw = true; }
    assert(threw);
    ok("pushFront and insertBefore (empty, head, middle, null guard)");

    // findByTitle / removeByTitle on the sorted list
    // (CS101 Lecture, Math Tutorial, Quiz, Lab Session, Evening Study).
    assert(sorted.findByTitle("Quiz") == tieNode);
    assert(sorted.findByTitle("quiz") == nullptr);              // case-sensitive
    assert(sorted.findByTitle("Missing Event") == nullptr);
    assert(sorted.removeByTitle("Quiz"));                       // middle
    assert(sorted.findByTitle("Quiz") == nullptr && sorted.size() == 4);
    assert(sorted.removeByTitle("CS101 Lecture"));              // head
    assert(sorted.head()->data.title == "Math Tutorial" && sorted.head()->prev == nullptr);
    assert(sorted.removeByTitle("Evening Study"));              // tail
    assert(sorted.tail()->data.title == "Lab Session" && sorted.tail()->next == nullptr);
    assert(!sorted.removeByTitle("Missing Event"));             // no match
    assert(sorted.size() == 2);
    assert(sorted.removeByTitle("Math Tutorial") && sorted.removeByTitle("Lab Session"));
    assert(sorted.empty() && sorted.head() == nullptr && sorted.tail() == nullptr);
    ok("findByTitle / removeByTitle (head, middle, tail, miss, empty)");

    // Overlap check against 'schedule' (Office Hours was removed above, so it is
    // CS101 08:00-09:30, Math Tutorial 10:00-11:00, Lab Session 13:00-15:00).
    auto mk = [](const char* s, const char* e) {
        return ScheduleItem{"Probe", timeToMinutes(s), timeToMinutes(e), 0};
    };
    assert(schedule.findOverlap(mk("09:00", "09:45")) == n1);  // partial overlap
    assert(schedule.hasOverlap(mk("13:30", "14:00")));          // fully inside
    assert(schedule.hasOverlap(mk("12:00", "16:00")));          // fully covers
    assert(!schedule.hasOverlap(mk("11:00", "13:00")));         // fills the gap exactly
    assert(!schedule.hasOverlap(mk("07:00", "08:00")));         // ends as first starts
    assert(!schedule.hasOverlap(mk("15:00", "16:00")));         // starts as last ends
    assert(!DoublyLinkedList().hasOverlap(mk("08:00", "09:00"))); // empty list
    assert(itemsOverlap(mk("08:00", "09:00"), mk("08:30", "10:00")));
    assert(!itemsOverlap(mk("08:00", "09:00"), mk("09:00", "10:00")));
    ok("Overlap check: partial / inside / covering clash; touching edges do not");

    // Print remaining items.
    std::cout << "  Schedule after remove:\n";
    for (DLLNode* cur = schedule.head(); cur != nullptr; cur = cur->next)
        std::cout << "    " << cur->data.title << " ["
                  << minutesToTime(cur->data.startMin) << " - "
                  << minutesToTime(cur->data.endMin) << "]\n";

    // ---- UndoStack ----
    section("UndoStack");

    UndoStack undo;
    assert(undo.empty());

    // Build the day out of order with insertSorted, snapshotting after each edit.
    // Snapshot 1: 1 item.
    DoublyLinkedList s1;
    s1.insertSorted(e3);                         // Lab Session 13:00
    undo.push(s1);
    ok("Pushed snapshot 1 (1 item)");

    // Snapshot 2: 2 items.
    s1.insertSorted(e1);                         // CS101 Lecture 08:00 -> new head
    undo.push(s1);
    ok("Pushed snapshot 2 (2 items)");

    // Snapshot 3: 3 items.
    s1.insertSorted(e2);                         // Math Tutorial 10:00 -> middle
    undo.push(s1);
    ok("Pushed snapshot 3 (3 items)");

    assert(undo.size() == 3);

    // Editing the live list after pushing must not change any snapshot.
    assert(s1.removeByTitle("CS101 Lecture"));
    assert(s1.size() == 2);
    assert(undo.top().size() == 3);
    assert(undo.top().findByTitle("CS101 Lecture") != nullptr);
    ok("Snapshots are independent of later edits to the live list");

    // Pop and verify LIFO order and the exact contents of each snapshot.
    DoublyLinkedList restored3 = undo.pop();
    assert(restored3.size() == 3);
    assert(restored3.head()->data.title == "CS101 Lecture");
    assert(restored3.head()->next->data.title == "Math Tutorial");
    assert(restored3.tail()->data.title == "Lab Session");
    for (DLLNode* cur = restored3.head(); cur != nullptr; cur = cur->next)
        assert(cur->data.locationId >= 0 && cur->data.locationId <= 14);  // real node IDs
    ok("pop() restored snapshot 3 (3 items, sorted, valid node IDs)");

    DoublyLinkedList restored2 = undo.pop();
    assert(restored2.size() == 2);
    assert(restored2.head()->data.title == "CS101 Lecture");
    assert(restored2.head()->data.startMin == timeToMinutes("08:00"));
    assert(restored2.tail()->data.locationId == 5);             // CS Lab One
    assert(restored2.findByTitle("Math Tutorial") == nullptr);
    ok("pop() restored snapshot 2 (2 items, contents match)");

    DoublyLinkedList restored1 = undo.pop();
    assert(restored1.size() == 1);
    assert(restored1.head() == restored1.tail());
    assert(restored1.head()->data.title == "Lab Session");
    assert(minutesToTime(restored1.head()->data.endMin) == "15:00");
    ok("pop() restored snapshot 1 (1 item, contents match)");

    assert(undo.empty());
    ok("UndoStack LIFO order and deep-copy semantics verified");

    // clear(): empties a non-empty stack, is safe on an empty one, and the
    // stack is reusable afterwards.
    undo.push(restored1);
    undo.push(restored2);
    undo.push(restored3);
    assert(undo.size() == 3);
    undo.clear();
    assert(undo.empty() && undo.size() == 0);
    undo.clear();                                   // no-op on empty stack
    assert(undo.empty());
    undo.push(restored2);                           // reusable after clear
    assert(undo.size() == 1 && undo.top().size() == 2);
    undo.clear();
    ok("clear() empties the stack, is idempotent, and allows reuse");

    // Copy-assignment operator test.
    DoublyLinkedList dst;
    dst = copy;   // copy has 4 items
    assert(dst.size() == 4);
    dst.clear();
    assert(copy.size() == 4);  // copy must remain intact
    ok("Copy-assignment operator: independent after assign");
}

// ================================================================ Scheduler ===
// Collect titles in list order, e.g. "CS101 Lecture|Lab Session".
static std::string titlesOf(const DoublyLinkedList& list) {
    std::string out;
    for (DLLNode* cur = list.head(); cur != nullptr; cur = cur->next) {
        if (!out.empty()) out += '|';
        out += cur->data.title;
    }
    return out;
}

static void testScheduler() {
    section("Scheduler (auto-snapshot, undo, redo)");

    Scheduler sched;
    assert(sched.getSchedule().empty());
    assert(!sched.undo() && !sched.redo());
    ok("Empty scheduler: undo()/redo() return false");

    ScheduleItem lecture{"CS101 Lecture", timeToMinutes("08:00"), timeToMinutes("09:30"), 2};  // Academic Block A
    ScheduleItem lab    {"Lab Session",   timeToMinutes("13:00"), timeToMinutes("15:00"), 5};  // CS Lab One
    ScheduleItem math   {"Math Tutorial", timeToMinutes("10:00"), timeToMinutes("11:00"), 3};  // Academic Block B
    ScheduleItem clash  {"Group Meeting", timeToMinutes("09:00"), timeToMinutes("10:30"), 7};  // Library
    ScheduleItem library{"Library Visit", timeToMinutes("11:00"), timeToMinutes("12:00"), 7};  // Library

    // Adds keep start-time order regardless of insertion order.
    assert(sched.addEvent(lab));
    assert(sched.addEvent(lecture));
    assert(sched.addEvent(math));
    assert(titlesOf(sched.getSchedule()) == "CS101 Lecture|Math Tutorial|Lab Session");
    ok("addEvent inserts in start-time order");

    // Overlap and invalid ranges are rejected and leave everything untouched.
    assert(!sched.addEvent(clash));                              // overlaps lecture + math
    ScheduleItem empty{"Zero Length", timeToMinutes("12:00"), timeToMinutes("12:00"), 7};
    ScheduleItem reversed{"Backwards", timeToMinutes("12:30"), timeToMinutes("12:15"), 7};
    assert(!sched.addEvent(empty));
    assert(!sched.addEvent(reversed));
    assert(!sched.removeEvent("No Such Event"));
    assert(sched.getSchedule().size() == 3);
    assert(sched.getSchedule().findByTitle("Group Meeting") == nullptr);
    ok("Overlapping / zero-length / reversed adds and unknown removes rejected");

    // Back-to-back events touch but do not overlap.
    assert(sched.addEvent(library));                             // 11:00, right after math
    assert(sched.removeEvent("Math Tutorial"));
    assert(titlesOf(sched.getSchedule()) == "CS101 Lecture|Library Visit|Lab Session");
    ok("Back-to-back add accepted; removeEvent removes by title");

    // Undo walks back one successful edit at a time; rejected edits left no snapshots.
    assert(sched.undo());                                        // undo remove math
    assert(titlesOf(sched.getSchedule()) == "CS101 Lecture|Math Tutorial|Library Visit|Lab Session");
    assert(sched.undo());                                        // undo add library
    assert(titlesOf(sched.getSchedule()) == "CS101 Lecture|Math Tutorial|Lab Session");
    assert(sched.undo());                                        // undo add math
    assert(sched.undo());                                        // undo add lecture
    assert(titlesOf(sched.getSchedule()) == "Lab Session");
    assert(sched.undo());                                        // undo add lab
    assert(sched.getSchedule().empty());
    assert(!sched.undo());                                       // exactly 5 snapshots
    ok("undo() restores each prior state; only successful edits were snapshotted");

    // Redo replays forward through the same states.
    assert(sched.redo());
    assert(titlesOf(sched.getSchedule()) == "Lab Session");
    assert(sched.redo() && sched.redo());
    assert(titlesOf(sched.getSchedule()) == "CS101 Lecture|Math Tutorial|Lab Session");
    assert(sched.undo());                                        // undo/redo interleave
    assert(sched.redo());
    assert(titlesOf(sched.getSchedule()) == "CS101 Lecture|Math Tutorial|Lab Session");
    ok("redo() re-applies undone edits; undo/redo interleave correctly");

    // A rejected edit keeps the redo history.
    assert(!sched.addEvent(clash));
    assert(sched.redo());                                        // re-add library
    assert(titlesOf(sched.getSchedule()) == "CS101 Lecture|Math Tutorial|Library Visit|Lab Session");
    ok("Rejected edit does not clear redo history");

    // A new successful edit clears the redo history.
    assert(sched.undo());                                        // redo now available
    ScheduleItem gym{"Sports Practice", timeToMinutes("16:00"), timeToMinutes("17:00"), 10};  // Sports Room
    assert(sched.addEvent(gym));                                 // new timeline
    assert(!sched.redo());
    assert(titlesOf(sched.getSchedule()) == "CS101 Lecture|Math Tutorial|Lab Session|Sports Practice");
    assert(sched.undo());                                        // the new edit is undoable
    assert(titlesOf(sched.getSchedule()) == "CS101 Lecture|Math Tutorial|Lab Session");
    ok("New edit after undo clears redo stack; the new edit is itself undoable");

    std::cout << "  Final schedule:\n";
    for (DLLNode* cur = sched.getSchedule().head(); cur != nullptr; cur = cur->next)
        std::cout << "    " << cur->data.title << " ["
                  << minutesToTime(cur->data.startMin) << " - "
                  << minutesToTime(cur->data.endMin) << "] @ node "
                  << cur->data.locationId << '\n';
}

// ===================================================== Scheduler CSV loading ===
static void testSchedulerCsv() {
    section("Scheduler CSV loading (validation, no undo)");

    Graph graph;
    assert(graph.loadGraph("data/nodes.csv", "data/edges.csv"));

    // Missing file: the only case that returns false.
    Scheduler none;
    assert(!none.loadFromCsv("data/no_such_schedule.csv", graph));
    assert(none.getSchedule().empty());
    ok("Missing file returns false and leaves the schedule empty");

    // The real student day: every row is valid.
    Scheduler day;
    assert(day.loadFromCsv("data/schedule.csv", graph));
    assert(day.getSkippedCount() == 0);
    assert(day.getSchedule().size() == 5);
    assert(titlesOf(day.getSchedule()) ==
           "Data Structures Lecture|Linear Algebra|Lunch Break|"
           "Data Structures Lab|Digital Logic Design");
    int prevStart = -1;
    for (DLLNode* cur = day.getSchedule().head(); cur != nullptr; cur = cur->next) {
        assert(cur->data.startMin > prevStart);                  // sorted
        assert(graph.getNode(cur->data.locationId) != nullptr);  // real node
        prevStart = cur->data.startMin;
    }
    DLLNode* first = day.getSchedule().head();
    assert(first->data.startMin == timeToMinutes("08:00"));
    assert(first->data.endMin   == timeToMinutes("09:30"));
    assert(first->data.locationId == 2);
    ok("data/schedule.csv: 5 events, start-time order, 0 skipped");

    // Loading pushes no snapshots; later edits are still undoable.
    assert(!day.undo());
    assert(day.getSchedule().size() == 5);
    ScheduleItem gym{"Sports Practice", timeToMinutes("16:00"), timeToMinutes("17:00"), 10};
    assert(day.addEvent(gym));
    assert(day.undo());
    assert(day.getSchedule().size() == 5);
    assert(!day.undo());                                         // load stays put
    ok("Initial load cannot be undone; later edits still can");

    // Bad rows: write a temporary CSV, load it, then delete it.
    const std::string tmpPath = "data/tmp_bad_schedule.csv";
    {
        std::ofstream out(tmpPath);
        assert(out.is_open());
        out << "title,start,end,locationId\n"
            << "Good Early,08:00,09:00,2\n"
            << "Too Few,09:00,10:00\n"                // 3 columns
            << "Too Many,10:00,11:00,2,extra\n"       // 5 columns
            << "Bad Time,9:00,10:00,2\n"              // not zero-padded
            << "Bad Minutes,10:75,11:00,2\n"          // minutes > 59
            << "Reversed,12:00,11:00,2\n"             // end < start
            << "Zero Length,12:00,12:00,2\n"          // end == start
            << "No Node,10:00,11:00,99\n"             // id not in graph
            << "Negative Node,10:00,11:00,-1\n"       // not digits
            << "Clash,08:30,09:30,3\n"                // overlaps Good Early
            << "\n"                                   // blank: ignored
            << "Good Late,10:00,11:00,3\r\n";         // Windows ending
    }
    Scheduler bad;
    bool loaded = bad.loadFromCsv(tmpPath, graph);
    std::remove(tmpPath.c_str());
    assert(loaded);
    assert(bad.getSkippedCount() == 9);
    assert(bad.getSchedule().size() == 2);
    assert(titlesOf(bad.getSchedule()) == "Good Early|Good Late");
    assert(!bad.undo());
    std::ifstream gone(tmpPath);
    assert(!gone.is_open());                                     // cleaned up
    ok("Bad rows skipped and counted (9), good rows kept, temp file deleted");

    // A second load adds to the existing schedule and resets the counter:
    // the Lecture (08:00) clashes with Good Early and Linear Algebra (09:45)
    // clashes with Good Late; the other 3 rows fit.
    assert(bad.loadFromCsv("data/schedule.csv", graph));
    assert(bad.getSkippedCount() == 2);
    assert(bad.getSchedule().size() == 5);
    ok("Reload resets the skipped count; overlaps with existing events skipped");
}

// ================================================================== HashMap ===
static void testHashMap() {
    section("HashMap (separate chaining, rehash at 0.75)");

    HashMap map;                                    // 8 buckets
    int value = -1;
    assert(map.empty() && map.capacity() == HashMap::DEFAULT_CAPACITY);
    assert(!map.find(42, value) && value == -1);    // missing key: outValue untouched
    assert(!map.contains(42) && !map.remove(42));
    ok("Empty map: find/contains/remove on missing key return false");

    // Insert, find, and overwrite.
    assert(map.insert(1, 100));
    assert(map.insert(2, 200));
    assert(map.find(1, value) && value == 100);
    assert(map.find(2, value) && value == 200);
    assert(!map.insert(1, 111));                    // existing key -> update
    assert(map.size() == 2);
    assert(map.find(1, value) && value == 111);
    assert(map.insert(-7, 70) && map.find(-7, value) && value == 70);   // negative key
    ok("insert / find / overwrite (size unchanged on update, negative keys)");

    // Collisions: 3, 11, 19 all land in the same bucket of an 8-bucket map.
    HashMap chain;
    assert(chain.bucketIndex(3) == chain.bucketIndex(11));
    assert(chain.bucketIndex(3) == chain.bucketIndex(19));
    assert(chain.insert(3, 30) && chain.insert(11, 110) && chain.insert(19, 190));
    assert(chain.find(3, value) && value == 30);
    assert(chain.find(11, value) && value == 110);
    assert(chain.find(19, value) && value == 190);
    assert(!chain.contains(27));                    // same bucket, never inserted
    ok("Colliding keys 3 / 11 / 19 share a bucket and stay distinct");

    // Remove from the middle, head and tail of that chain.
    assert(chain.remove(11));                       // middle
    assert(!chain.contains(11) && chain.contains(3) && chain.contains(19));
    assert(chain.remove(19));                       // head (last inserted)
    assert(chain.remove(3));                        // only remaining entry
    assert(chain.empty() && !chain.remove(3));
    assert(chain.insert(11, 1) && chain.find(11, value) && value == 1);  // reusable
    ok("remove from middle / head / last of a chain; double remove returns false");

    // Rehash: capacity doubles as soon as load factor exceeds 0.75.
    HashMap grow(4);
    grow.insert(10, 1); grow.insert(20, 2); grow.insert(30, 3);      // 3/4 == 0.75
    assert(grow.capacity() == 4);
    grow.insert(40, 4);                                              // 4/4  > 0.75
    assert(grow.capacity() == 8 && grow.loadFactor() <= 0.75);
    for (int k = 0; k < 200; ++k) grow.insert(1000 + k, k);
    assert(grow.size() == 204);
    assert(grow.loadFactor() <= 0.75);
    for (int k = 0; k < 200; ++k)
        assert(grow.find(1000 + k, value) && value == k);
    assert(grow.find(10, value) && value == 1 && grow.find(40, value) && value == 4);
    std::cout << "  204 keys -> capacity " << grow.capacity()
              << ", load factor " << grow.loadFactor() << '\n';
    ok("Rehash keeps load factor <= 0.75 and every key findable");

    // Rule of Three: deep copies are independent.
    HashMap copy = grow;
    assert(copy.size() == grow.size() && copy.capacity() == grow.capacity());
    copy.remove(10);
    copy.insert(20, -2);
    assert(grow.find(10, value) && value == 1);
    assert(grow.find(20, value) && value == 2);
    HashMap assigned(2);
    assigned.insert(5, 5);
    assigned = map;
    assert(assigned.size() == map.size() && !assigned.contains(5));
    assigned = assigned;                            // self-assignment is safe
    assert(assigned.find(1, value) && value == 111);
    assigned.clear();
    assert(assigned.empty() && map.contains(1));
    ok("Copy ctor / copy assignment are deep; clear() leaves the source intact");

    // ---- Room availability built from the campus graph ----
    section("RoomAvailability (room id -> free-slot bitmask)");

    Graph graph;
    assert(graph.loadGraph("data/nodes.csv", "data/edges.csv"));
    RoomAvailability rooms;
    assert(rooms.buildFromGraph(graph) == 6);       // ids 2, 3, 4, 5, 6, 11
    assert(rooms.hasRoom(2) && rooms.hasRoom(11));
    assert(!rooms.hasRoom(0) && !rooms.hasRoom(7)); // transit / service nodes
    assert(!rooms.hasRoom(99) && rooms.getMask(99) == -1);
    ok("Loaded 6 AcademicRooms; non-academic and unknown ids are absent");

    // Academic Block A (2): busy "11010011" -> free slots 2, 4, 5.
    assert(rooms.getMask(2) == 0b00110100);
    assert(rooms.freeSlotCount(2) == 3);
    assert(rooms.isFreeAtHour(2, 10) && !rooms.isFreeAtHour(2, 8));
    assert(!rooms.isFreeAtHour(2, 16) && !rooms.isFreeSlot(2, -1));   // out of range
    for (int id = 0; id < graph.getNodeCount(); ++id) {
        AcademicRoom* room = dynamic_cast<AcademicRoom*>(graph.getNode(id));
        if (room == nullptr) continue;
        for (int slot = 0; slot < RoomAvailability::SLOT_COUNT; ++slot)
            assert(rooms.isFreeSlot(id, slot) == room->isSlotFree(slot));
    }
    ok("Bitmask matches every AcademicRoom schedule slot");

    std::vector<int> freeAt8 = rooms.freeRoomsAtHour(8);
    assert(freeAt8.size() == 2 && freeAt8[0] == 3 && freeAt8[1] == 6);
    std::cout << "  Free at 08:00:";
    for (int id : freeAt8)
        std::cout << " [" << id << "] " << graph.getNode(id)->getName() << ';';
    std::cout << '\n';
    assert(rooms.buildFromGraph(graph) == 6);       // rebuild does not duplicate
    ok("freeRoomsAtHour lists free rooms; rebuilding does not duplicate");
}

// ============================================================== BunkerRadar ===
static void testBunkerRadar() {
    section("Bunker Radar (free rooms ranked by utility score)");

    Graph graph;
    assert(graph.loadGraph("data/nodes.csv", "data/edges.csv"));
    RoomAvailability rooms;
    rooms.buildFromGraph(graph);
    BunkerRadar radar(graph, rooms);

    // Free-room lookup: at 08:00 only Academic Block B (3) and CS Lab Two (6).
    std::vector<int> free8 = radar.findFreeRooms(8);
    assert(free8.size() == 2 && free8[0] == 3 && free8[1] == 6);
    ok("findFreeRooms(8) returns exactly rooms 3 and 6");

    // Consecutive free hours and the score formula.
    assert(radar.consecutiveFreeHours(6, 8) == 2);    // CS Lab Two free 08-10
    assert(radar.consecutiveFreeHours(3, 14) == 2);   // free 14:00 and 15:00 (to 16:00)
    assert(radar.consecutiveFreeHours(3, 9) == 0);    // busy at 09:00
    assert(radar.consecutiveFreeHours(99, 8) == 0);   // unknown room
    assert(BunkerRadar::utilityScore(2, 50.0) == 15.0);
    assert(BunkerRadar::utilityScore(3, 100.0) == BunkerRadar::utilityScore(2, 0.0));  // 1h == 100m
    ok("consecutiveFreeHours and utilityScore follow the documented formula");

    // An hour with free rooms: user at Central Plaza (1), 08:00.
    //   CS Lab Two       2h, 95m -> 20 - 9.5 = 10.5
    //   Academic Block B 1h, 65m -> 10 - 6.5 =  3.5
    std::vector<BunkerResult> top = radar.topRooms(1, 8, 5);
    assert(top.size() == 2);                           // only 2 free, even though N = 5
    assert(top[0].roomId == 6 && top[0].name == "CS Lab Two");
    assert(top[0].score == 10.5 && top[0].distance == 95.0 && top[0].freeHours == 2);
    assert(top[1].roomId == 3 && top[1].score == 3.5 && top[1].distance == 65.0);
    ok("08:00 from Central Plaza: closer room loses to longer free window");

    // Ranking order with more candidates: 11:00 (rush hour) has 4 free rooms.
    top = radar.topRooms(1, 11, 4);
    assert(top.size() == 4);
    for (size_t i = 1; i < top.size(); ++i)
        assert(top[i - 1].score > top[i].score);       // strictly descending here
    assert(top[0].roomId == 5 && top[1].roomId == 11 && top[2].roomId == 3 && top[3].roomId == 4);
    std::cout << "  Top rooms at 11:00 from Central Plaza:\n";
    for (const BunkerResult& b : top)
        std::cout << "    [" << b.roomId << "] " << b.name << "  score=" << b.score
                  << "  dist=" << b.distance << "m  free=" << b.freeHours << "h\n";
    ok("11:00 ranking is strictly descending: 5 > 11 > 3 > 4");

    // Top-N truncation returns the best N, in the same order.
    std::vector<BunkerResult> top2 = radar.topRooms(1, 11, 2);
    assert(top2.size() == 2 && top2[0].roomId == 5 && top2[1].roomId == 11);
    ok("topN = 2 returns the two highest-scoring rooms");

    // No free rooms: hours outside 08-15, and a campus with no academic rooms.
    assert(radar.findFreeRooms(7).empty() && radar.topRooms(1, 7, 3).empty());
    assert(radar.findFreeRooms(16).empty() && radar.topRooms(1, 16, 3).empty());
    Graph noRooms;                                     // empty campus
    RoomAvailability noAvailability;
    assert(noAvailability.buildFromGraph(noRooms) == 0);
    BunkerRadar emptyRadar(noRooms, noAvailability);
    for (int hour = BunkerRadar::FIRST_HOUR; hour <= BunkerRadar::LAST_HOUR; ++hour) {
        assert(emptyRadar.findFreeRooms(hour).empty());
        assert(emptyRadar.topRooms(0, hour, 3).empty());
    }
    ok("No free rooms (hour outside 08-15 / no academic rooms) -> empty result");

    // Invalid inputs.
    assert(radar.topRooms(1, 8, 0).empty());           // N = 0
    assert(radar.topRooms(99, 8, 3).empty());          // unknown user node
    assert(radar.topRooms(-1, 8, 3).empty());
    ok("topN <= 0 and invalid user node return empty");
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

    // Digits and spaces are part of the key: "Lab 1" and "Lab 2" stay distinct.
    PrefixTrie labs;
    labs.insert("Lab 1", 21);
    labs.insert("Lab 2", 22);
    labs.insert("Lab 12", 23);
    labs.insert("Lab1", 24);                         // no space -> different key
    assert(labs.searchExact("Lab 1") == 21);
    assert(labs.searchExact("Lab 2") == 22);
    assert(labs.searchExact("Lab 12") == 23);
    assert(labs.searchExact("Lab1") == 24);
    assert(labs.searchExact("Lab 3") == -1);
    assert(labs.searchExact("LAB 1") == 21);         // still case-insensitive
    assert(labs.searchExact("  lab   2 ") == 22);    // extra whitespace collapses
    assert(labs.searchExact("Lab") == -1);           // prefix only, not a key
    results = labs.autocomplete("lab 1");            // "lab 1" and "lab 12"
    assert(results.size() == 2);
    results = labs.autocomplete("Lab ");             // trailing space trimmed -> "lab"
    assert(results.size() == 4);
    std::cout << "  autocomplete(\"lab 1\"):\n";
    for (auto& [name, id] : labs.autocomplete("lab 1"))
        std::cout << "    key=\"" << name << "\" id=" << id << '\n';
    ok("\"Lab 1\" / \"Lab 2\" / \"Lab 12\" / \"Lab1\" stored and found separately");
}

// ============================================== CampusNode OOP & Spatial Graph ===
static void testCampusGraph() {
    section("CampusNode OOP Hierarchy & CSV Loading");

    Graph graph;
    bool loaded = graph.loadGraph("data/nodes.csv", "data/edges.csv");
    assert(loaded);
    ok("Loaded data/nodes.csv and data/edges.csv successfully");

    int nodeCount = graph.getNodeCount();
    assert(nodeCount == 15);
    std::cout << "  Total campus nodes loaded: " << nodeCount << '\n';
    ok("Verified 15 campus locations");

    // Verify OOP hierarchy on loaded nodes
    CampusNode* n0 = graph.getNode(0);
    assert(n0 != nullptr);
    assert(n0->getType() == "TRANSIT");
    assert(n0->getName() == "Main Gate");
    ok("Node 0 is TransitNode (\"Main Gate\")");

    CampusNode* n2 = graph.getNode(2);
    assert(n2 != nullptr);
    assert(n2->getType() == "ACADEMIC");
    AcademicRoom* roomA = dynamic_cast<AcademicRoom*>(n2);
    assert(roomA != nullptr);
    // Schedule for node 2 is 11010011
    assert(roomA->isSlotBusy(0) == true);
    assert(roomA->isSlotBusy(1) == true);
    assert(roomA->isSlotFree(2) == true);
    assert(roomA->isSlotBusy(3) == true);
    assert(roomA->isSlotFree(4) == true);
    assert(roomA->isSlotFree(5) == true);
    assert(roomA->isSlotBusy(6) == true);
    assert(roomA->isSlotBusy(7) == true);
    // Hour 10 is slot 2 (10 - 8 = 2) -> free
    assert(roomA->isAvailableAtHour(10) == true);
    // Hour 8 is slot 0 (8 - 8 = 0) -> busy
    assert(roomA->isAvailableAtHour(8) == false);
    ok("Node 2 is AcademicRoom with 8-slot schedule & availability helpers verified");

    CampusNode* n7 = graph.getNode(7);
    assert(n7 != nullptr);
    assert(n7->getType() == "SERVICE");
    ServiceFacility* lib = dynamic_cast<ServiceFacility*>(n7);
    assert(lib != nullptr);
    assert(lib->getCurrentQueue() >= 0);
    assert(lib->getAvgServiceMinutes() > 0);
    int wait = lib->getEstimatedWaitMinutes();
    assert(wait == lib->getCurrentQueue() * lib->getAvgServiceMinutes());
    ok("Node 7 is ServiceFacility with queue and wait time helpers verified");

    // ---- PrefixTrie Integration ----
    section("PrefixTrie Integrated with Campus Nodes");
    PrefixTrie campusTrie;
    for (int i = 0; i < nodeCount; ++i) {
        CampusNode* node = graph.getNode(i);
        if (node) {
            campusTrie.insert(node->getName(), node->getId());
        }
    }
    assert(campusTrie.searchExact("Academic Block A") == 2);
    assert(campusTrie.searchExact("library") == 7);
    auto labMatches = campusTrie.autocomplete("cs");
    assert(labMatches.size() == 2); // CS Lab One and CS Lab Two
    std::cout << "  Prefix autocomplete(\"cs\") on loaded campus:\n";
    for (auto& [k, id] : labMatches) {
        std::cout << "    [" << id << "] " << graph.getNode(id)->getName() << '\n';
    }
    ok("PrefixTrie correctly indexes and autocompletes real campus locations");

    // ---- Dijkstra Pathfinding: Normal vs AvoidSun ----
    section("Dijkstra: Normal vs Sun Avoidance (Heatwave Routing)");
    // Route from Central Plaza (1) to Academic Block A (2)
    // Direct outdoor edge: 1 -> 2 (distance = 60.0, isOutdoor = 1)
    // Indoor route: 1 -> Main Corridor (12, 40m) -> Academic Block A (2, 25m) = 65m total
    
    // Normal query (avoidSun = false, morning hour 9)
    PathResult normalRes = graph.dijkstra(1, 2, 9, false);
    std::cout << "  Normal route (1 -> 2, avoidSun=false, hour=9):\n    Path: ";
    for (int id : normalRes.path) std::cout << graph.getNode(id)->getName() << " (" << id << ") -> ";
    std::cout << "Total Cost = " << normalRes.totalCost << "m\n";
    assert(normalRes.path.size() == 2);
    assert(normalRes.path[0] == 1 && normalRes.path[1] == 2);
    assert(normalRes.totalCost == 60.0);
    ok("Normal Dijkstra chooses direct outdoor path (60m)");

    // Sun avoidance query (avoidSun = true, morning hour 9)
    // Direct path gets 2.5x penalty: 60 * 2.5 = 150m
    // Indoor path via Main Corridor (12) stays 40 + 25 = 65m
    PathResult shadedRes = graph.dijkstra(1, 2, 9, true);
    std::cout << "  Shaded route (1 -> 2, avoidSun=true, hour=9):\n    Path: ";
    for (int id : shadedRes.path) std::cout << graph.getNode(id)->getName() << " (" << id << ") -> ";
    std::cout << "Total Cost = " << shadedRes.totalCost << "m\n";
    assert(shadedRes.path.size() == 3);
    assert(shadedRes.path[0] == 1 && shadedRes.path[1] == 12 && shadedRes.path[2] == 2);
    assert(shadedRes.totalCost == 65.0);
    ok("avoidSun=true reroutes via indoor corridors (Main Corridor, 65m vs 150m penalized)");

    // Rush hour query (currentHour = 12, peak 11-14)
    // Direct path: 60 * 1.5 rushMultiplier = 90m
    // Indoor path: 40 * 1.3 (52m) + 25 * 1.3 (32.5m) = 84.5m
    PathResult rushRes = graph.dijkstra(1, 2, 12, false);
    std::cout << "  Peak-hour route (1 -> 2, avoidSun=false, hour=12):\n    Path: ";
    for (int id : rushRes.path) std::cout << graph.getNode(id)->getName() << " (" << id << ") -> ";
    std::cout << "Total Cost = " << rushRes.totalCost << "m\n";
    assert(rushRes.totalCost == 84.5);
    ok("Peak hour multiplier (11-14) correctly adjusts edge weights");

    // ---- Squad-Sync Meetup Point ----
    section("Squad-Sync (3-User Collective Travel Optimization)");
    // 3 students at different campus points:
    // User 1: CS Lab One (5)
    // User 2: EE Block (4)
    // User 3: Cafeteria (9)
    std::vector<int> squad = {5, 4, 9};
    MeetupResult meetup = graph.findSquadMeetup(squad, 10, false);
    assert(meetup.meetupNodeId >= 0);
    std::cout << "  Squad members at:\n";
    for (size_t i = 0; i < squad.size(); ++i) {
        std::cout << "    User " << (i + 1) << ": " << graph.getNode(squad[i])->getName()
                  << " (Cost to meetup: " << meetup.userCosts[i] << "m)\n";
    }
    std::cout << "  Optimal Meetup Node: [" << meetup.meetupNodeId << "] "
              << graph.getNode(meetup.meetupNodeId)->getName() << '\n';
    std::cout << "  Total Collective Cost: " << meetup.totalCollectiveCost << "m\n";
    ok("Squad-Sync identified optimal collective meetup vertex");

    // ---- Errand Optimization (Permutations) ----
    section("Errand Tour Optimization (Permutations)");
    // Student starts at Main Gate (0), needs to visit Dhaba (8), Library (7), and Sports Room (10),
    // and finish class at CS Lab Two (6).
    int startNode = 0;
    int endNode = 6;
    std::vector<int> errands = {8, 7, 10};

    ErrandResult errandTour = graph.optimizeErrands(startNode, endNode, errands, 10, false);
    assert(!errandTour.visitOrder.empty());
    assert(errandTour.visitOrder.front() == startNode);
    assert(errandTour.visitOrder.back() == endNode);

    std::cout << "  Start: " << graph.getNode(startNode)->getName() << " (0)\n";
    std::cout << "  End:   " << graph.getNode(endNode)->getName() << " (6)\n";
    std::cout << "  Errands to visit: Dhaba (8), Library (7), Sports Room (10)\n";
    std::cout << "  Optimal Visit Order:\n    ";
    for (size_t i = 0; i < errandTour.visitOrder.size(); ++i) {
        std::cout << graph.getNode(errandTour.visitOrder[i])->getName()
                  << " (" << errandTour.visitOrder[i] << ")";
        if (i + 1 < errandTour.visitOrder.size()) std::cout << " -> ";
    }
    std::cout << "\n  Complete Step-by-Step Path:\n    ";
    for (size_t i = 0; i < errandTour.fullPath.size(); ++i) {
        std::cout << graph.getNode(errandTour.fullPath[i])->getName();
        if (i + 1 < errandTour.fullPath.size()) std::cout << " -> ";
    }
    std::cout << "\n  Total Tour Cost: " << errandTour.totalCost << "m\n";
    ok("Errand tour tested permutations and produced shortest complete journey");
}

// ============================================================ Speedrunner ===
// True if every consecutive pair in 'path' is joined by an edge.
static bool isWalkablePath(const Graph& g, const std::vector<int>& path) {
    for (std::size_t i = 0; i + 1 < path.size(); ++i) {
        bool linked = false;
        for (const Edge& e : g.getNeighbors(path[i]))
            if (e.to == path[i + 1])
                linked = true;
        if (!linked)
            return false;
    }
    return true;
}

// Minutes for a fixed order, computed independently with one Dijkstra per
// leg plus 15 min (queue 5 x 3 min) at each ServiceFacility errand.
static double minutesForOrder(const Graph& g, int start, int end,
                              const std::vector<int>& order, int hour) {
    double total = 0.0;
    int    prev  = start;
    for (int id : order) {
        total += g.dijkstraDistances(prev, hour, false)[id] / 80.0;
        if (dynamic_cast<const ServiceFacility*>(g.getNode(id)) != nullptr)
            total += 15.0;
        prev = id;
    }
    return total + g.dijkstraDistances(prev, hour, false)[end] / 80.0;
}

static void testSpeedrunner() {
    section("Break Speedrunner (k+1 Dijkstra matrix, pruned permutations)");

    Graph graph;
    assert(graph.loadGraph("data/nodes.csv", "data/edges.csv"));
    const int BREAK_START = timeToMinutes("10:00");
    const int HOUR        = 10;
    const double EPS      = 1e-9;

    // Feasible: Main Gate -> {Library, Cafeteria, CS Lab One, Sports Room}
    // -> Academic Block A in a 60-minute break.
    std::vector<int> errands{7, 9, 5, 10};
    SpeedrunResult fit = planBreak(graph, 0, 2, errands,
                                   BREAK_START, BREAK_START + 60, HOUR, false);
    assert(fit.feasible);
    assert(fit.visitOrder.size() == 4);
    for (int id : errands) {                                     // each once
        int seen = 0;
        for (int v : fit.visitOrder)
            if (v == id) ++seen;
        assert(seen == 1);
    }
    assert(fit.totalMinutes <= 60.0);
    assert(std::abs(fit.totalMinutes -
                    minutesForOrder(graph, 0, 2, fit.visitOrder, HOUR)) < EPS);
    assert(fit.fullPath.front() == 0 && fit.fullPath.back() == 2);
    assert(isWalkablePath(graph, fit.fullPath));
    std::size_t at = 0;                                          // errands appear
    for (int id : fit.visitOrder) {                              // in visit order
        while (at < fit.fullPath.size() && fit.fullPath[at] != id) ++at;
        assert(at < fit.fullPath.size());
    }
    ok("4 errands fit in 60 min; time matches an independent recount");

    // Optimality: no other order of the same errands is faster.
    std::vector<int> order = errands;
    for (int a = 0; a < 4; ++a)
        for (int b = 0; b < 4; ++b)
            for (int c = 0; c < 4; ++c)
                for (int d = 0; d < 4; ++d) {
                    if (a == b || a == c || a == d || b == c || b == d || c == d)
                        continue;
                    order = {errands[a], errands[b], errands[c], errands[d]};
                    assert(fit.totalMinutes <=
                           minutesForOrder(graph, 0, 2, order, HOUR) + EPS);
                }
    ok("Chosen order is no slower than any of the 24 orders");

    std::cout << "  Plan (" << fit.totalMinutes << " min):";
    for (int id : fit.fullPath)
        std::cout << ' ' << id;
    std::cout << '\n';

    // Infeasible: Dhaba errand needs 15 min of queueing; break is 5 min.
    SpeedrunResult shortBreak = planBreak(graph, 0, 2, {8},
                                          BREAK_START, BREAK_START + 5, HOUR, false);
    assert(!shortBreak.feasible);
    assert(shortBreak.totalMinutes > 5.0);                       // real time kept
    assert(shortBreak.visitOrder.size() == 1 && shortBreak.visitOrder[0] == 8);
    assert(shortBreak.fullPath.front() == 0 && shortBreak.fullPath.back() == 2);
    assert(isWalkablePath(graph, shortBreak.fullPath));
    std::cout << "  5-min break: " << shortBreak.totalMinutes - 5.0
              << " minutes short\n";
    ok("Too-short break: feasible=false but best order, path and time returned");

    // Invalid input: feasible=false, totalMinutes=-1, empty vectors.
    SpeedrunResult bad[] = {
        planBreak(graph, 0, 2, {3, 4, 5, 6, 7}, BREAK_START, BREAK_START + 60, HOUR, false),
        planBreak(graph, 0, 2, {99},            BREAK_START, BREAK_START + 60, HOUR, false),
        planBreak(graph, 0, 2, {-1},            BREAK_START, BREAK_START + 60, HOUR, false),
        planBreak(graph, 42, 2, {},             BREAK_START, BREAK_START + 60, HOUR, false),
        planBreak(graph, 0, 2, {5, 5},          BREAK_START, BREAK_START + 60, HOUR, false),
        planBreak(graph, 0, 2, {},              BREAK_START, BREAK_START,      HOUR, false),
    };
    for (const SpeedrunResult& r : bad) {
        assert(!r.feasible);
        assert(r.totalMinutes == -1.0);
        assert(r.visitOrder.empty() && r.fullPath.empty());
    }
    ok("Rejected: 5 errands, unknown/negative node, duplicate, empty break");

    // Zero errands: walk straight from start to end.
    SpeedrunResult direct = planBreak(graph, 0, 2, {},
                                      BREAK_START, BREAK_START + 60, HOUR, false);
    PathResult straight = graph.dijkstra(0, 2, HOUR, false);
    assert(direct.feasible);
    assert(direct.visitOrder.empty());
    assert(direct.fullPath == straight.path);
    assert(std::abs(direct.totalMinutes - straight.totalCost / 80.0) < EPS);
    SpeedrunResult stay = planBreak(graph, 0, 0, {},
                                    BREAK_START, BREAK_START + 60, HOUR, false);
    assert(stay.feasible && stay.totalMinutes == 0.0);
    assert(stay.fullPath.size() == 1 && stay.fullPath[0] == 0);
    ok("Zero errands: direct Dijkstra route; start == end costs 0 min");
}

// ============================================================== self-tests ===
// Runs every testX() above.  Each one asserts, so reaching the end means all
// checks passed; a failed assert stops the program with the failing line.
static void runSelfTests() {
    std::cout << "========================================================\n";
    std::cout << "        Campus-Sync – Comprehensive System Verification\n";
    std::cout << "========================================================\n";

    // Milestone 1 Core Data Structures
    testMinHeap();
    testMaxHeap();
    testDLLAndUndoStack();
    testScheduler();
    testSchedulerCsv();
    testHashMap();
    testBunkerRadar();
    testPrefixTrie();

    // Milestone 2 Campus Data & Spatial Graph Engine
    testCampusGraph();
    testSpeedrunner();

    std::cout << "\n========================================================\n";
    std::cout << "  All system verification checks passed. Zero crashes.\n";
    std::cout << "========================================================\n";
}

// ============================================================ console input ===
// Every prompt reads a whole line with getline, so stray text never stays in
// the buffer for the next prompt.  Returns false on end of input (Ctrl+Z /
// closed pipe), which the menu treats as Exit.
static bool readLine(const std::string& prompt, std::string& out) {
    std::cout << prompt;
    if (!std::getline(std::cin, out))
        return false;
    if (!out.empty() && out.back() == '\r')
        out.pop_back();
    std::size_t first = out.find_first_not_of(" \t");
    std::size_t last  = out.find_last_not_of(" \t");
    out = (first == std::string::npos) ? "" : out.substr(first, last - first + 1);
    return true;
}

// Ask until the user types a whole number in [lo, hi].  Rejects empty input,
// letters, "3abc", signs on their own and numbers too long to fit an int.
static bool readInt(const std::string& prompt, int lo, int hi, int& out) {
    std::string line;
    while (readLine(prompt, line)) {
        bool digitsOnly = !line.empty() && line.size() <= 9;
        std::size_t start = (digitsOnly && line[0] == '-') ? 1 : 0;
        if (start == line.size())
            digitsOnly = false;
        for (std::size_t i = start; digitsOnly && i < line.size(); ++i)
            if (line[i] < '0' || line[i] > '9')
                digitsOnly = false;

        if (!digitsOnly) {
            std::cout << "  Please enter a number.\n";
            continue;
        }
        int value = std::stoi(line);
        if (value < lo || value > hi) {
            std::cout << "  Please enter a number from " << lo << " to " << hi << ".\n";
            continue;
        }
        out = value;
        return true;
    }
    return false;
}

// ============================================================ dashboard UI ===
static void printBanner(const Graph& graph, int roomCount) {
    std::cout << "\n========================================================\n"
              << "                 Campus-Sync Dashboard\n"
              << "========================================================\n"
              << "  Loaded " << graph.getNodeCount() << " campus locations, "
              << roomCount << " bookable rooms.\n";
}

// ---------------------------------------------------------------- pickNode ---
// Shared by every option that needs a location.  The user types part of a
// name; PrefixTrie::autocomplete lists the matches, numbered, and the user
// picks one.  Typing nothing cancels.
// Returns the chosen node id, or -1 if the user cancelled or input ended.
static int pickNode(const Graph& graph, const PrefixTrie& names,
                    const std::string& what) {
    std::string prefix;
    while (readLine("  " + what + " - type part of a name (Enter to cancel): ",
                    prefix)) {
        if (prefix.empty())
            return -1;

        std::vector<std::pair<std::string, int>> matches = names.autocomplete(prefix);
        if (matches.empty()) {
            std::cout << "    No place starts with \"" << prefix << "\". Try again.\n";
            continue;
        }

        const int count = static_cast<int>(matches.size());
        for (int i = 0; i < count; ++i) {
            const CampusNode* node = graph.getNode(matches[i].second);
            std::cout << "    " << std::setw(2) << (i + 1) << ". "
                      << std::left << std::setw(20) << node->getName()
                      << std::right << " (" << node->getType() << ")\n";
        }
        if (count == 1) {                               // nothing to choose
            std::cout << "    -> " << graph.getNode(matches[0].second)->getName() << '\n';
            return matches[0].second;
        }

        int pick;
        if (!readInt("  Pick 1-" + std::to_string(count) + " (0 to search again): ",
                     0, count, pick))
            return -1;
        if (pick != 0)
            return matches[pick - 1].second;
    }
    return -1;
}

// ------------------------------------------------------------- findPlace ---
// Menu option: look a place up by name and show what we know about it.
static void findPlace(const Graph& graph, const PrefixTrie& names,
                      const RoomAvailability& rooms) {
    std::cout << "\n==================== Find a place ======================\n";
    int id = pickNode(graph, names, "Place");
    if (id < 0)
        return;

    const CampusNode* node = graph.getNode(id);
    std::cout << "--------------------------------------------------------\n"
              << "  Name     : " << node->getName() << '\n'
              << "  Type     : " << node->getType() << '\n'
              << "  Node id  : " << id << '\n'
              << "  Position : (" << node->getX() << ", " << node->getY() << ")\n"
              << "  Paths to : " << graph.getNeighbors(id).size() << " places\n";

    if (rooms.hasRoom(id)) {
        std::cout << "  Free at  :";
        int freeCount = 0;
        for (int hour = 8; hour <= 15; ++hour)
            if (rooms.isFreeAtHour(id, hour)) {
                std::cout << ' ' << (hour < 10 ? "0" : "") << hour << ":00";
                ++freeCount;
            }
        std::cout << (freeCount == 0 ? " never (booked all day)" : "") << '\n';
    }
    if (const ServiceFacility* facility = dynamic_cast<const ServiceFacility*>(node))
        std::cout << "  Queue    : about " << facility->getEstimatedWaitMinutes()
                  << " min wait\n";
}

// --------------------------------------------------------- readHour/readYesNo ---
static const double WALK_METERS_PER_MIN = 80.0;   // same pace as Speedrunner

static bool readHour(int& hour) {
    return readInt("  Hour of day (8-15): ", 8, 15, hour);
}

// Ask until the user types y/yes or n/no (any case).
static bool readYesNo(const std::string& prompt, bool& answer) {
    std::string line;
    while (readLine(prompt, line)) {
        for (char& c : line)
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        if (line == "y" || line == "yes") { answer = true;  return true; }
        if (line == "n" || line == "no")  { answer = false; return true; }
        std::cout << "  Please answer y or n.\n";
    }
    return false;
}

// The edge from 'from' to 'to', or nullptr if they are not adjacent.
static const Edge* findEdge(const Graph& graph, int from, int to) {
    for (const Edge& e : graph.getNeighbors(from))
        if (e.to == to)
            return &e;
    return nullptr;
}

// ---------------------------------------------------------------- showRoute ---
// Menu option: shortest route between two places with Graph::dijkstra.
//
// Dijkstra minimises the *weighted* cost (rush-hour crowding, plus a 2.5x
// penalty on outdoor paths when avoiding sun).  The table shows each leg's
// real length, and the time estimate uses the rush-hour weight only: the sun
// penalty changes which way you go, not how fast you walk.
static void showRoute(const Graph& graph, const PrefixTrie& names) {
    std::cout << "\n============ Route between two places ==================\n";
    int start = pickNode(graph, names, "Start");
    if (start < 0) return;
    int end = pickNode(graph, names, "Destination");
    if (end < 0) return;
    int hour;
    bool avoidSun;
    if (!readHour(hour) || !readYesNo("  Avoid sun-exposed paths? (y/n): ", avoidSun))
        return;

    PathResult route = graph.dijkstra(start, end, hour, avoidSun);
    std::cout << "--------------------------------------------------------\n";
    if (route.path.empty()) {
        std::cout << "  No walkable route from " << graph.getNode(start)->getName()
                  << " to " << graph.getNode(end)->getName() << ".\n";
        return;
    }

    std::cout << "  " << std::left << std::setw(4) << "#"
              << std::setw(22) << "From" << std::setw(22) << "To"
              << std::right << std::setw(8) << "Meters" << "\n"
              << "  " << std::string(56, '-') << '\n';
    double meters = 0.0, walkCost = 0.0;
    for (std::size_t i = 0; i + 1 < route.path.size(); ++i) {
        const Edge* e = findEdge(graph, route.path[i], route.path[i + 1]);
        meters   += e->distance;
        walkCost += graph.calculateWeight(*e, hour, false);
        std::cout << "  " << std::left << std::setw(4) << (i + 1)
                  << std::setw(22) << graph.getNode(route.path[i])->getName()
                  << std::setw(22) << graph.getNode(route.path[i + 1])->getName()
                  << std::right << std::setw(8) << e->distance
                  << (e->isOutdoor ? "  (outdoor)" : "") << '\n';
    }

    std::cout << "  " << std::string(56, '-') << '\n'
              << std::fixed << std::setprecision(1)
              << "  Path     : ";
    for (std::size_t i = 0; i < route.path.size(); ++i)
        std::cout << (i ? " -> " : "") << graph.getNode(route.path[i])->getName();
    std::cout << "\n  Distance : " << meters << " m\n"
              << "  Time     : about " << walkCost / WALK_METERS_PER_MIN
              << " min at " << WALK_METERS_PER_MIN << " m/min"
              << (hour >= 11 && hour <= 14 ? " (rush hour)" : "") << '\n';
    std::cout.unsetf(std::ios::fixed);
    std::cout << std::setprecision(6);
}

// -------------------------------------------------------------- measurePath ---
// Real meters and estimated walking minutes along a Dijkstra path (same rules
// as showRoute: rush-hour weight counts toward time, sun penalty does not).
static void measurePath(const Graph& graph, const std::vector<int>& path, int hour,
                        double& meters, double& minutes) {
    meters = 0.0;
    double walkCost = 0.0;
    for (std::size_t i = 0; i + 1 < path.size(); ++i) {
        const Edge* e = findEdge(graph, path[i], path[i + 1]);
        meters   += e->distance;
        walkCost += graph.calculateWeight(*e, hour, false);
    }
    minutes = walkCost / WALK_METERS_PER_MIN;
}

// ---------------------------------------------------------------- squadSync ---
// Menu option: 2-4 friends at different places; Graph::findSquadMeetup picks
// the node with the lowest total travel cost for the whole group.
static void squadSync(const Graph& graph, const PrefixTrie& names) {
    std::cout << "\n===================== Squad-Sync =======================\n";
    int count;
    if (!readInt("  How many people (2-4)? ", 2, 4, count))
        return;

    std::vector<int> squad;
    for (int i = 1; i <= count; ++i) {
        int id = pickNode(graph, names, "Person " + std::to_string(i));
        if (id < 0) return;
        squad.push_back(id);
    }
    int hour;
    bool avoidSun;
    if (!readHour(hour) || !readYesNo("  Avoid sun-exposed paths? (y/n): ", avoidSun))
        return;

    MeetupResult meet = graph.findSquadMeetup(squad, hour, avoidSun);
    std::cout << "--------------------------------------------------------\n";
    if (meet.meetupNodeId < 0) {
        std::cout << "  No place can be reached by everyone.\n";
        return;
    }

    const std::string meetName = graph.getNode(meet.meetupNodeId)->getName();
    std::cout << "  Meet at  : " << meetName << '\n'
              << "  " << std::string(56, '-') << '\n'
              << "  " << std::left << std::setw(10) << "Person"
              << std::setw(22) << "Starts at"
              << std::right << std::setw(10) << "Meters" << std::setw(12) << "Minutes" << '\n'
              << "  " << std::string(56, '-') << '\n'
              << std::fixed << std::setprecision(1);

    double totalMeters = 0.0, slowest = 0.0;
    for (int i = 0; i < count; ++i) {
        PathResult walk = graph.dijkstra(squad[i], meet.meetupNodeId, hour, avoidSun);
        double meters, minutes;
        measurePath(graph, walk.path, hour, meters, minutes);
        totalMeters += meters;
        if (minutes > slowest) slowest = minutes;
        std::cout << "  " << std::left << std::setw(10) << ("#" + std::to_string(i + 1))
                  << std::setw(22) << graph.getNode(squad[i])->getName()
                  << std::right << std::setw(10) << meters
                  << std::setw(12) << minutes << '\n';
    }
    std::cout << "  " << std::string(56, '-') << '\n'
              << "  Total distance : " << totalMeters << " m\n"
              << "  Everyone there : in about " << slowest << " min\n";
    std::cout.unsetf(std::ios::fixed);
    std::cout << std::setprecision(6);
}

// -------------------------------------------------------------- bunkerRadar ---
// Menu option: the 3 best empty rooms to sit in at a given hour, ranked by
// BunkerRadar's utility score (10 points per consecutive free hour, minus
// 1 point per 10 m of walking).
static void bunkerRadar(const Graph& graph, const PrefixTrie& names,
                        const RoomAvailability& rooms) {
    std::cout << "\n==================== Bunker Radar ======================\n";
    int hour;
    if (!readHour(hour))
        return;
    int here = pickNode(graph, names, "You are at");
    if (here < 0)
        return;

    BunkerRadar radar(graph, rooms);
    std::vector<BunkerResult> top = radar.topRooms(here, hour, 3);
    std::cout << "--------------------------------------------------------\n";
    if (top.empty()) {
        std::cout << "  No free rooms at " << (hour < 10 ? "0" : "") << hour
                  << ":00 that you can walk to.\n";
        return;
    }

    std::cout << "  Free rooms at " << (hour < 10 ? "0" : "") << hour << ":00 near "
              << graph.getNode(here)->getName() << ":\n"
              << "  " << std::string(56, '-') << '\n'
              << "  " << std::left << std::setw(4) << "#" << std::setw(20) << "Room"
              << std::right << std::setw(8) << "Score" << std::setw(12) << "Distance"
              << std::setw(12) << "Free for" << '\n'
              << "  " << std::string(56, '-') << '\n'
              << std::fixed << std::setprecision(1);
    for (std::size_t i = 0; i < top.size(); ++i) {
        const BunkerResult& r = top[i];
        int untilHour = hour + r.freeHours;
        std::cout << "  " << std::left << std::setw(4) << (i + 1)
                  << std::setw(20) << r.name
                  << std::right << std::setw(8) << r.score
                  << std::setw(10) << r.distance << " m"
                  << std::setw(7) << r.freeHours << " h"
                  << "  (until " << (untilHour < 10 ? "0" : "") << untilHour << ":00)\n";
    }
    std::cout << "  " << std::string(56, '-') << '\n';
    if (hour >= 11 && hour <= 14)
        std::cout << "  Distances include the rush-hour crowd penalty.\n";
    std::cout.unsetf(std::ios::fixed);
    std::cout << std::setprecision(6);
}

// ----------------------------------------------------------------- readTime ---
// Ask until the user types a valid "HH:MM" (00:00-23:59, zero-padded).
static bool readTime(const std::string& prompt, int& minutes) {
    std::string line;
    while (readLine(prompt, line)) {
        try {
            minutes = timeToMinutes(line);
            return true;
        } catch (const std::invalid_argument&) {
            std::cout << "  Please enter a time like 09:30 (24-hour HH:MM).\n";
        }
    }
    return false;
}

// One schedule line: "HH:MM - HH:MM  Title  @ Location".
static void printEvent(const Graph& graph, const ScheduleItem& item) {
    std::cout << minutesToTime(item.startMin) << " - " << minutesToTime(item.endMin)
              << "  " << std::left << std::setw(26) << item.title << std::right
              << " @ " << graph.getNode(item.locationId)->getName() << '\n';
}

// ------------------------------------------------------------- mySchedule ---
// Submenu: view, add, remove, undo and redo on the day's Scheduler.
static void showDay(const Graph& graph, const Scheduler& day) {
    std::cout << "\n  ------------------- Your day -------------------------\n";
    if (day.getSchedule().empty()) {
        std::cout << "  (no events)\n";
        return;
    }
    int n = 1;
    for (DLLNode* cur = day.getSchedule().head(); cur != nullptr; cur = cur->next, ++n) {
        std::cout << "  " << std::setw(2) << n << ". ";
        printEvent(graph, cur->data);
    }
}

static void addToDay(const Graph& graph, const PrefixTrie& names, Scheduler& day) {
    std::string title;
    if (!readLine("  Title (Enter to cancel): ", title) || title.empty())
        return;
    int start, end;
    if (!readTime("  Start (HH:MM): ", start) || !readTime("  End   (HH:MM): ", end))
        return;
    if (end <= start) {
        std::cout << "  Not added: the end time must be after the start time.\n";
        return;
    }
    int where = pickNode(graph, names, "Location");
    if (where < 0)
        return;

    ScheduleItem item{title, start, end, where};
    if (day.addEvent(item)) {
        std::cout << "  Added: ";
        printEvent(graph, item);
        return;
    }
    DLLNode* clash = day.getSchedule().findOverlap(item);
    std::cout << "  Not added: it overlaps ";
    if (clash != nullptr)
        printEvent(graph, clash->data);
    else
        std::cout << "an existing event.\n";
}

static void removeFromDay(Scheduler& day) {
    std::string title;
    if (!readLine("  Title to remove (Enter to cancel): ", title) || title.empty())
        return;
    if (day.removeEvent(title))
        std::cout << "  Removed \"" << title << "\".\n";
    else
        std::cout << "  No event is called \"" << title << "\" (titles must match exactly).\n";
}

static void mySchedule(const Graph& graph, const PrefixTrie& names, Scheduler& day) {
    while (true) {
        std::cout << "\n==================== My Schedule =======================\n"
                  << "  1. View the day\n"
                  << "  2. Add an event\n"
                  << "  3. Remove an event\n"
                  << "  4. Undo\n"
                  << "  5. Redo\n"
                  << "  0. Back\n"
                  << "--------------------------------------------------------\n";
        int choice;
        if (!readInt("Choose an option: ", 0, 5, choice) || choice == 0)
            return;

        switch (choice) {
            case 1: showDay(graph, day);              break;
            case 2: addToDay(graph, names, day);      break;
            case 3: removeFromDay(day);               break;
            case 4:
                if (day.undo()) { std::cout << "  Undone.";  showDay(graph, day); }
                else            std::cout << "  Nothing to undo.\n";
                break;
            case 5:
                if (day.redo()) { std::cout << "  Redone.";  showDay(graph, day); }
                else            std::cout << "  Nothing to redo.\n";
                break;
        }
    }
}

// ---------------------------------------------------------------- nextClass ---
// Menu option: find the first event starting at or after the current time
// and route there with Graph::dijkstra.  The rush-hour rules use the current
// hour; sun avoidance is off.
static void nextClass(const Graph& graph, const PrefixTrie& names, const Scheduler& day) {
    std::cout << "\n===================== Next class =======================\n";
    int now;
    if (!readTime("  Current time (HH:MM): ", now))
        return;

    // The list is in start-time order, so the first match is the next event.
    const ScheduleItem* next = nullptr;
    for (DLLNode* cur = day.getSchedule().head(); cur != nullptr; cur = cur->next)
        if (cur->data.startMin >= now) {
            next = &cur->data;
            break;
        }
    if (next == nullptr) {
        std::cout << "  Nothing else on your schedule after " << minutesToTime(now) << ".\n";
        return;
    }

    std::cout << "  Next up : ";
    printEvent(graph, *next);
    int here = pickNode(graph, names, "You are at");
    if (here < 0)
        return;

    const int hour = now / 60;
    PathResult route = graph.dijkstra(here, next->locationId, hour, false);
    std::cout << "--------------------------------------------------------\n";
    if (route.path.empty()) {
        std::cout << "  No walkable route to " << graph.getNode(next->locationId)->getName() << ".\n";
        return;
    }

    double meters, minutes;
    measurePath(graph, route.path, hour, meters, minutes);
    std::cout << "  Path     : ";
    for (std::size_t i = 0; i < route.path.size(); ++i)
        std::cout << (i ? " -> " : "") << graph.getNode(route.path[i])->getName();
    std::cout << std::fixed << std::setprecision(1)
              << "\n  Distance : " << meters << " m\n"
              << "  Walk     : about " << minutes << " min\n";

    double spare = next->startMin - now - minutes;
    if (spare < 0)
        std::cout << "  Warning  : you'll be about " << -spare << " min late!\n";
    else
        std::cout << "  On time  : " << spare << " min to spare before "
                  << minutesToTime(next->startMin) << ".\n";
    std::cout.unsetf(std::ios::fixed);
    std::cout << std::setprecision(6);
}

static void printMenu() {
    std::cout << "\n--------------------- Main Menu ------------------------\n"
              << "  1. Run self-tests\n"
              << "  2. Find a place\n"
              << "  3. Route between two places\n"
              << "  4. Squad-Sync (meeting point)\n"
              << "  5. Bunker Radar (free rooms)\n"
              << "  6. My Schedule\n"
              << "  7. Next class\n"
              << "  0. Exit\n"
              << "--------------------------------------------------------\n";
}

// ===================================================================== main ===
int main() {
    // ---- Startup: graph, name trie and room availability table ----
    Graph graph;
    if (!graph.loadGraph("data/nodes.csv", "data/edges.csv")) {
        std::cerr << "Error: could not load data/nodes.csv and data/edges.csv.\n"
                  << "Run campus_sync from the project root folder.\n";
        return 1;
    }

    PrefixTrie names;
    for (int id = 0; id < graph.getNodeCount(); ++id) {
        CampusNode* node = graph.getNode(id);
        if (node != nullptr)
            names.insert(node->getName(), id);
    }

    RoomAvailability rooms;
    int roomCount = rooms.buildFromGraph(graph);

    printBanner(graph, roomCount);

    // ---- The student's day: loaded once, not undoable ----
    Scheduler day;
    if (!day.loadFromCsv("data/schedule.csv", graph)) {
        std::cout << "  Warning: data/schedule.csv not found; starting with an empty day.\n";
    } else {
        std::cout << "  Loaded " << day.getSchedule().size() << " events from data/schedule.csv.\n";
        if (day.getSkippedCount() > 0)
            std::cout << "  Warning: skipped " << day.getSkippedCount()
                      << " invalid or overlapping line(s) in data/schedule.csv.\n";
    }

    // ---- Main menu loop ----
    const int MAX_CHOICE = 7;
    while (true) {
        printMenu();
        int choice;
        if (!readInt("Choose an option: ", 0, MAX_CHOICE, choice))
            break;                                      // end of input
        if (choice == 0)
            break;

        switch (choice) {
            case 1: runSelfTests();                  break;
            case 2: findPlace(graph, names, rooms);  break;
            case 3: showRoute(graph, names);         break;
            case 4: squadSync(graph, names);         break;
            case 5: bunkerRadar(graph, names, rooms); break;
            case 6: mySchedule(graph, names, day);   break;
            case 7: nextClass(graph, names, day);    break;
        }
    }

    std::cout << "\nGoodbye!\n";
    return 0;
}
