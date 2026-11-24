// main.cpp – sanity-check driver for all data structures and Graph engine.
// All output lives here (UI layer); no cout/cin inside DS/Graph implementations.

#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include <stdexcept>
#include <fstream>   // temporary CSV in testSchedulerCsv
#include <cstdio>    // std::remove
#include <limits>    // std::numeric_limits
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

// ===================================================== campus test helpers ===
// Node id by exact name; the tests look places up by name, never by raw id.
static int idOf(const Graph& g, const std::string& name) {
    for (int i = 0; i < g.getNodeCount(); ++i)
        if (g.getNode(i) != nullptr && g.getNode(i)->getName() == name)
            return i;
    assert(false && "unknown campus node name");
    return -1;
}

// The edge a -> b, or nullptr if they are not adjacent.
static const Edge* edgeBetween(const Graph& g, int a, int b) {
    for (const Edge& e : g.getNeighbors(a))
        if (e.to == b)
            return &e;
    return nullptr;
}

// True if every consecutive pair in 'path' is joined by an edge.
static bool isWalkablePath(const Graph& g, const std::vector<int>& path) {
    for (std::size_t i = 0; i + 1 < path.size(); ++i)
        if (edgeBetween(g, path[i], path[i + 1]) == nullptr)
            return false;
    return true;
}

// Sum of Graph::calculateWeight over the path's edges.
static double pathCost(const Graph& g, const std::vector<int>& path, int hour, bool sun) {
    double total = 0.0;
    for (std::size_t i = 0; i + 1 < path.size(); ++i)
        total += g.calculateWeight(*edgeBetween(g, path[i], path[i + 1]), hour, sun);
    return total;
}

// Number of stairs flights on the path (same building, different floor).
static int stairsIn(const Graph& g, const std::vector<int>& path) {
    int count = 0;
    for (std::size_t i = 0; i + 1 < path.size(); ++i) {
        const CampusNode* a = g.getNode(path[i]);
        const CampusNode* b = g.getNode(path[i + 1]);
        if (a->getBuilding() != "-" && a->getBuilding() == b->getBuilding()
            && a->getFloor() != b->getFloor())
            ++count;
    }
    return count;
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
           "Digital Logic Design|Linear Algebra|Data Structures|"
           "Lunch Break|Data Structures Lab");
    int prevStart = -1;
    for (DLLNode* cur = day.getSchedule().head(); cur != nullptr; cur = cur->next) {
        assert(cur->data.startMin > prevStart);                  // sorted
        assert(graph.getNode(cur->data.locationId) != nullptr);  // real node
        prevStart = cur->data.startMin;
    }
    DLLNode* first = day.getSchedule().head();
    assert(first->data.startMin == timeToMinutes("08:00"));
    assert(first->data.endMin   == timeToMinutes("08:50"));
    assert(first->data.locationId == idOf(graph, "C15"));
    assert(day.getSchedule().tail()->data.locationId == idOf(graph, "Lab 2"));
    ok("data/schedule.csv: 5 events, start-time order, 0 skipped");

    // Loading pushes no snapshots; later edits are still undoable.
    assert(!day.undo());
    assert(day.getSchedule().size() == 5);
    ScheduleItem gym{"Sports Practice", timeToMinutes("16:00"), timeToMinutes("17:00"),
                     idOf(graph, "Sports Room")};
    assert(day.addEvent(gym));
    assert(day.undo());
    assert(day.getSchedule().size() == 5);
    assert(!day.undo());                                         // load stays put
    ok("Initial load cannot be undone; later edits still can");

    // Bad rows: write a temporary CSV, load it, then delete it.
    // Ids 2 and 3 exist (junctions); 99 is past the last node.
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
    assert(graph.getNode(99) == nullptr);
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
    // Digital Logic Design (08:00) clashes with Good Early and Data Structures
    // (10:00) clashes with Good Late; the other 3 rows fit.
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
    int academic = 0;
    for (int id = 0; id < graph.getNodeCount(); ++id)
        if (dynamic_cast<AcademicRoom*>(graph.getNode(id)) != nullptr)
            ++academic;
    RoomAvailability rooms;
    assert(rooms.buildFromGraph(graph) == academic);
    const int d24 = idOf(graph, "D24");
    assert(rooms.hasRoom(d24) && rooms.hasRoom(idOf(graph, "Lab 5")));
    assert(!rooms.hasRoom(idOf(graph, "Main Gate")));            // transit
    assert(!rooms.hasRoom(idOf(graph, "Library")));              // service
    assert(!rooms.hasRoom(999) && rooms.getMask(999) == -1);
    ok("Loaded every AcademicRoom; non-academic and unknown ids are absent");

    // D24 busy "00111001" -> busy slots 2,3,4,7 -> free slots 0,1,5,6
    // -> mask = 1 + 2 + 32 + 64 = 99.
    assert(rooms.getMask(d24) == 0b01100011);
    assert(rooms.freeSlotCount(d24) == 4);
    assert(rooms.isFreeAtHour(d24, 8) && !rooms.isFreeAtHour(d24, 10));
    assert(!rooms.isFreeAtHour(d24, 16) && !rooms.isFreeSlot(d24, -1));   // out of range
    for (int id = 0; id < graph.getNodeCount(); ++id) {
        AcademicRoom* room = dynamic_cast<AcademicRoom*>(graph.getNode(id));
        if (room == nullptr) continue;
        for (int slot = 0; slot < RoomAvailability::SLOT_COUNT; ++slot)
            assert(rooms.isFreeSlot(id, slot) == room->isSlotFree(slot));
    }
    ok("Bitmask matches every AcademicRoom schedule slot");

    // freeRoomsAtHour(h) lists exactly the academic rooms free at h, ascending.
    for (int hour = 8; hour <= 15; ++hour) {
        std::vector<int> freeNow = rooms.freeRoomsAtHour(hour);
        int expected = 0;
        for (int id = 0; id < graph.getNodeCount(); ++id) {
            AcademicRoom* room = dynamic_cast<AcademicRoom*>(graph.getNode(id));
            if (room != nullptr && room->isAvailableAtHour(hour)) ++expected;
        }
        assert(static_cast<int>(freeNow.size()) == expected && expected > 0);
        for (std::size_t i = 0; i < freeNow.size(); ++i) {
            assert(rooms.isFreeAtHour(freeNow[i], hour));
            assert(i == 0 || freeNow[i - 1] < freeNow[i]);
        }
    }
    std::cout << "  Free at 08:00: " << rooms.freeRoomsAtHour(8).size()
              << " of " << academic << " rooms\n";
    assert(rooms.buildFromGraph(graph) == academic);  // rebuild does not duplicate
    ok("freeRoomsAtHour matches the schedules at every hour; rebuilding does not duplicate");
}

// ============================================================== BunkerRadar ===
static void testBunkerRadar() {
    section("Bunker Radar (free rooms ranked by utility score)");

    Graph graph;
    assert(graph.loadGraph("data/nodes.csv", "data/edges.csv"));
    RoomAvailability rooms;
    rooms.buildFromGraph(graph);
    BunkerRadar radar(graph, rooms);

    // findFreeRooms agrees with the availability table at every hour.
    for (int hour = 8; hour <= 15; ++hour)
        assert(radar.findFreeRooms(hour) == rooms.freeRoomsAtHour(hour));
    ok("findFreeRooms(h) == freeRoomsAtHour(h) for every hour 08-15");

    // Consecutive free hours (D24 free 08, 09, 13, 14) and the score formula.
    const int d24 = idOf(graph, "D24");
    assert(radar.consecutiveFreeHours(d24, 8) == 2);
    assert(radar.consecutiveFreeHours(d24, 13) == 2);  // busy again at 15:00
    assert(radar.consecutiveFreeHours(d24, 10) == 0);  // busy
    assert(radar.consecutiveFreeHours(999, 8) == 0);   // unknown room
    assert(BunkerRadar::utilityScore(2, 50.0) == 15.0);
    assert(BunkerRadar::utilityScore(3, 100.0) == BunkerRadar::utilityScore(2, 0.0));  // 1h == 100m
    ok("consecutiveFreeHours and utilityScore follow the documented formula");

    // 11:00 from the CS Canteen: check every field against its definition and
    // that no free room left out scores higher than the ones returned.
    const int canteen = idOf(graph, "CS Canteen");
    const double EPS  = 1e-9;
    std::vector<double> dist = graph.dijkstraDistances(canteen, 11, false);
    std::vector<BunkerResult> top = radar.topRooms(canteen, 11, 3);
    assert(top.size() == 3);
    for (std::size_t i = 0; i < top.size(); ++i) {
        const BunkerResult& r = top[i];
        assert(rooms.isFreeAtHour(r.roomId, 11));
        assert(r.name == graph.getNode(r.roomId)->getName());
        assert(std::abs(r.distance - dist[r.roomId]) < EPS);
        assert(r.freeHours == radar.consecutiveFreeHours(r.roomId, 11));
        assert(std::abs(r.score - BunkerRadar::utilityScore(r.freeHours, r.distance)) < EPS);
        assert(i == 0 || top[i - 1].score >= top[i].score);  // descending
    }
    for (int id : radar.findFreeRooms(11)) {
        bool chosen = false;
        for (const BunkerResult& r : top)
            if (r.roomId == id) chosen = true;
        if (!chosen)
            assert(BunkerRadar::utilityScore(radar.consecutiveFreeHours(id, 11), dist[id])
                   <= top.back().score + EPS);
    }
    std::cout << "  Top rooms at 11:00 from CS Canteen:\n";
    for (const BunkerResult& b : top)
        std::cout << "    " << b.name << " [" << graph.getNode(b.roomId)->getBuilding()
                  << ", " << graph.getNode(b.roomId)->getFloor() << "]  score=" << b.score
                  << "  dist=" << b.distance << "m  free=" << b.freeHours << "h\n";
    ok("11:00 from CS Canteen: fields match their definitions, nothing better left out");

    // Top-N truncation returns the best N scores, in the same order.
    std::vector<BunkerResult> top2 = radar.topRooms(canteen, 11, 2);
    assert(top2.size() == 2);
    assert(std::abs(top2[0].score - top[0].score) < EPS);
    assert(std::abs(top2[1].score - top[1].score) < EPS);
    ok("topN = 2 returns the two highest scores");

    // No free rooms: hours outside 08-15, and a campus with no academic rooms.
    assert(radar.findFreeRooms(7).empty() && radar.topRooms(canteen, 7, 3).empty());
    assert(radar.findFreeRooms(16).empty() && radar.topRooms(canteen, 16, 3).empty());
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
    assert(radar.topRooms(canteen, 8, 0).empty());     // N = 0
    assert(radar.topRooms(999, 8, 3).empty());         // unknown user node
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

    auto results = trie.autocomplete("eng");
    assert(results.size() == 2);
    results = trie.autocomplete("lect");
    assert(results.size() == 2);
    assert(trie.autocomplete("xyz").empty());
    results = trie.autocomplete("Library");
    assert(results.size() == 1 && results[0].second == 4);
    ok("autocomplete: prefix matches, no match, and full word");

    // Digits, spaces and hyphens are part of the key.
    PrefixTrie labs;
    labs.insert("Lab 1", 21);
    labs.insert("Lab 2", 22);
    labs.insert("Lab 12", 23);
    labs.insert("Lab1", 24);                         // no space -> different key
    labs.insert("Lab 10", 25);
    labs.insert("LLC-1", 26);
    labs.insert("LLC-2", 27);
    labs.insert("R109", 28);
    assert(labs.searchExact("Lab 1") == 21);
    assert(labs.searchExact("Lab 10") == 25);        // not confused with "Lab 1"
    assert(labs.searchExact("Lab 12") == 23);
    assert(labs.searchExact("Lab1") == 24);
    assert(labs.searchExact("Lab 3") == -1);
    assert(labs.searchExact("LAB 1") == 21);         // case-insensitive
    assert(labs.searchExact("  lab   2 ") == 22);    // extra whitespace collapses
    assert(labs.searchExact("Lab") == -1);           // prefix only, not a key
    assert(labs.searchExact("llc-1") == 26 && labs.searchExact("LLC-2") == 27);
    assert(labs.searchExact("LLC1") == -1);          // hyphen is significant
    assert(labs.searchExact("r109") == 28);
    results = labs.autocomplete("lab 1");            // "lab 1", "lab 10", "lab 12"
    assert(results.size() == 3);
    results = labs.autocomplete("Lab ");             // trailing space trimmed -> "lab"
    assert(results.size() == 5);
    results = labs.autocomplete("llc-");
    assert(results.size() == 2 && results[0].first == "llc-1");
    std::cout << "  autocomplete(\"lab 1\"):\n";
    for (auto& [name, id] : labs.autocomplete("lab 1"))
        std::cout << "    key=\"" << name << "\" id=" << id << '\n';
    ok("\"Lab 1\" / \"Lab 10\" / \"LLC-1\" / \"R109\" stored and found separately");
}

// ============================================== CampusNode OOP & Spatial Graph ===
static void testCampusGraph() {
    section("CampusNode OOP Hierarchy & CSV Loading");

    Graph graph;
    bool loaded = graph.loadGraph("data/nodes.csv", "data/edges.csv");
    assert(loaded);
    ok("Loaded data/nodes.csv and data/edges.csv successfully");

    // 11 outdoor places + 11 floor hubs (EE 5, CS 3, MPB 3)
    // + 61 rooms (EE 45, CS 13, MPB 3) = 83.
    const int nodeCount = graph.getNodeCount();
    assert(nodeCount == 83);
    for (int i = 0; i < nodeCount; ++i)
        assert(graph.getNode(i) != nullptr && graph.getNode(i)->getId() == i);
    std::cout << "  Total campus nodes loaded: " << nodeCount << '\n';
    ok("Verified 83 campus locations with dense ids");

    CampusNode* gate = graph.getNode(idOf(graph, "Main Gate"));
    assert(gate->getType() == "TRANSIT");
    assert(gate->getBuilding() == "-" && gate->getFloor() == "-");
    ok("Main Gate is an outdoor TransitNode");

    AcademicRoom* d24 = dynamic_cast<AcademicRoom*>(graph.getNode(idOf(graph, "D24")));
    assert(d24 != nullptr);
    assert(d24->getBuilding() == "EE" && d24->getFloor() == "D");
    assert(d24->isAvailableAtHour(8) && !d24->isAvailableAtHour(10));
    assert(d24->isSlotBusy(7) && d24->isSlotFree(6));
    ok("D24 is an AcademicRoom on EE Floor D with its schedule loaded");

    ServiceFacility* lib = dynamic_cast<ServiceFacility*>(graph.getNode(idOf(graph, "Library")));
    assert(lib != nullptr);
    assert(lib->getBuilding() == "MPB" && lib->getFloor() == "Basement");
    assert(lib->getEstimatedWaitMinutes() == lib->getCurrentQueue() * lib->getAvgServiceMinutes());
    ok("Library is a ServiceFacility in the MPB basement with queue helpers");

    // ---- Structure of the 3D graph ----
    section("Campus structure (floors, stairs, rooms)");
    int stairs = 0;
    for (int a = 0; a < nodeCount; ++a) {
        const CampusNode* na = graph.getNode(a);
        const bool indoorRoom = na->getBuilding() != "-" && na->getType() != "TRANSIT";
        if (indoorRoom) {
            // A room links only to its own floor hub.
            assert(graph.getNeighbors(a).size() == 1);
            const CampusNode* hub = graph.getNode(graph.getNeighbors(a)[0].to);
            assert(hub->getType() == "TRANSIT");
            assert(hub->getBuilding() == na->getBuilding() && hub->getFloor() == na->getFloor());
            double d = graph.getNeighbors(a)[0].distance;
            assert(d >= 5.0 && d <= 15.0);
        }
        for (const Edge& e : graph.getNeighbors(a)) {
            const CampusNode* nb = graph.getNode(e.to);
            const bool sameBuilding = na->getBuilding() != "-" && na->getBuilding() == nb->getBuilding();
            assert(e.isOutdoor == !sameBuilding);           // indoor iff inside one building
            if (sameBuilding && na->getFloor() != nb->getFloor()) {
                assert(e.distance == 20.0 && e.rushMultiplier == 1.5);  // identical stairs
                ++stairs;
            }
        }
    }
    assert(stairs / 2 == 8);       // EE A-B-C-D-E: 4, CS 2, MPB 2
    for (int hour : {9, 12}) {
        std::vector<double> d = graph.dijkstraDistances(idOf(graph, "Main Gate"), hour, false);
        for (int i = 0; i < nodeCount; ++i)
            assert(d[i] < std::numeric_limits<double>::infinity());
    }
    ok("Rooms hang off their floor hub (5-15 m); 8 identical 20 m stairs; all reachable");

    // ---- PrefixTrie Integration ----
    section("PrefixTrie Integrated with Campus Nodes");
    PrefixTrie campusTrie;
    for (int i = 0; i < nodeCount; ++i)
        campusTrie.insert(graph.getNode(i)->getName(), i);
    for (int i = 0; i < nodeCount; ++i)
        assert(campusTrie.searchExact(graph.getNode(i)->getName()) == i);  // every name unique
    assert(campusTrie.searchExact("d24") == idOf(graph, "D24"));
    assert(campusTrie.autocomplete("lab 1").size() == 6);   // Lab 1, Lab 10-14
    assert(campusTrie.autocomplete("llc").size() == 2);     // LLC-1, LLC-2
    assert(campusTrie.autocomplete("ee floor").size() == 5);
    std::cout << "  autocomplete(\"lab 1\") on the campus:\n";
    for (auto& [k, id] : campusTrie.autocomplete("lab 1"))
        std::cout << "    " << graph.getNode(id)->getName() << " ["
                  << graph.getNode(id)->getBuilding() << ", " << graph.getNode(id)->getFloor() << "]\n";
    ok("Every campus name is found exactly; digit, hyphen and floor prefixes work");

    // ---- Dijkstra properties ----
    section("Dijkstra on the 3D campus");
    const double EPS = 1e-9;
    const std::pair<const char*, const char*> trips[] = {
        {"Main Gate", "Lab 2"}, {"D24", "Lab 4"}, {"Lab 14", "Cafe"},
        {"Futsal Court", "Lab 6"}, {"R109", "Masjid"}};
    for (const auto& [from, to] : trips)
        for (int hour : {9, 12})
            for (bool sun : {false, true}) {
                int s = idOf(graph, from), t = idOf(graph, to);
                PathResult r = graph.dijkstra(s, t, hour, sun);
                assert(r.path.front() == s && r.path.back() == t);
                assert(isWalkablePath(graph, r.path));
                assert(std::abs(r.totalCost - pathCost(graph, r.path, hour, sun)) < EPS);
                assert(std::abs(r.totalCost - graph.dijkstraDistances(s, hour, sun)[t]) < EPS);
                if (sun)
                    assert(r.totalCost >= graph.dijkstra(s, t, hour, false).totalCost - EPS);
            }
    ok("Paths start/end correctly, use real edges, cost = sum of edge weights");

    PathResult down = graph.dijkstra(idOf(graph, "D24"), idOf(graph, "Lab 4"), 9, false);
    assert(stairsIn(graph, down.path) == 4);    // EE D -> A (3) + CS Ground -> Basement (1)
    PathResult up = graph.dijkstra(idOf(graph, "Main Gate"), idOf(graph, "Lab 2"), 9, false);
    assert(stairsIn(graph, up.path) == 1);      // CS Ground -> Floor 1
    ok("Stairs count: D24 -> Lab 4 uses 4 flights, Main Gate -> Lab 2 uses 1");

    // Hand calculation, Main Gate -> D24 (the only route):
    //   Main Gate -> South Junction 150 m, -> EE Floor A 45 m,
    //   3 flights of stairs A -> D 3 x 20 m, -> D24 7 m (3rd of 11 rooms: 5 + 10*2/10).
    //   09:00: 150 + 45 + 60 + 7                       = 262.0
    //   12:00: 150*1.2 + 45*1.2 + 60*1.5 + 7*1.4 = 180 + 54 + 90 + 9.8 = 333.8
    PathResult toD24 = graph.dijkstra(idOf(graph, "Main Gate"), idOf(graph, "D24"), 9, false);
    assert(std::abs(toD24.totalCost - 262.0) < EPS);
    PathResult rushD24 = graph.dijkstra(idOf(graph, "Main Gate"), idOf(graph, "D24"), 12, false);
    assert(std::abs(rushD24.totalCost - 333.8) < EPS);
    std::cout << "  Main Gate -> D24: " << toD24.totalCost << " (09:00), "
              << rushD24.totalCost << " (12:00 rush)\n";
    ok("Main Gate -> D24 matches the hand calculation (262.0 / 333.8)");

    // ---- Squad-Sync Meetup Point ----
    section("Squad-Sync (3-User Collective Travel Optimization)");
    std::vector<int> squad = {idOf(graph, "D24"), idOf(graph, "Lab 2"), idOf(graph, "CS Canteen")};
    MeetupResult meetup = graph.findSquadMeetup(squad, 10, false);
    assert(meetup.meetupNodeId >= 0 && meetup.userCosts.size() == squad.size());
    std::vector<std::vector<double>> fromUser;
    double sum = 0.0;
    for (std::size_t i = 0; i < squad.size(); ++i) {
        fromUser.push_back(graph.dijkstraDistances(squad[i], 10, false));
        assert(std::abs(meetup.userCosts[i] - fromUser[i][meetup.meetupNodeId]) < EPS);
        sum += meetup.userCosts[i];
    }
    assert(std::abs(sum - meetup.totalCollectiveCost) < EPS);
    for (int v = 0; v < nodeCount; ++v) {                     // no better meeting point
        double total = 0.0;
        for (const auto& d : fromUser) total += d[v];
        assert(total >= meetup.totalCollectiveCost - EPS);
    }
    std::cout << "  D24 + Lab 2 + CS Canteen meet at "
              << graph.getNode(meetup.meetupNodeId)->getName()
              << " (total " << meetup.totalCollectiveCost << ")\n";
    ok("Squad-Sync total = sum of user costs and no node is cheaper");

    // ---- Errand Optimization (Permutations) ----
    section("Errand Tour Optimization (Permutations)");
    const int startNode = idOf(graph, "Main Gate");
    const int endNode   = idOf(graph, "Lab 2");
    std::vector<int> errands = {idOf(graph, "Masjid"), idOf(graph, "Library"),
                                idOf(graph, "Sports Room")};
    ErrandResult errandTour = graph.optimizeErrands(startNode, endNode, errands, 10, false);
    assert(errandTour.visitOrder.size() == errands.size() + 2);
    assert(errandTour.visitOrder.front() == startNode && errandTour.visitOrder.back() == endNode);
    assert(errandTour.fullPath.front() == startNode && errandTour.fullPath.back() == endNode);
    assert(isWalkablePath(graph, errandTour.fullPath));
    double best = std::numeric_limits<double>::infinity();
    for (int a = 0; a < 3; ++a)
        for (int b = 0; b < 3; ++b)
            for (int c = 0; c < 3; ++c) {
                if (a == b || a == c || b == c) continue;
                const int stops[] = {startNode, errands[a], errands[b], errands[c], endNode};
                double total = 0.0;
                for (int i = 0; i + 1 < 5; ++i)
                    total += graph.dijkstraDistances(stops[i], 10, false)[stops[i + 1]];
                if (total < best) best = total;
            }
    assert(std::abs(errandTour.totalCost - best) < EPS);
    std::cout << "  Optimal Visit Order:\n    ";
    for (size_t i = 0; i < errandTour.visitOrder.size(); ++i) {
        std::cout << graph.getNode(errandTour.visitOrder[i])->getName();
        if (i + 1 < errandTour.visitOrder.size()) std::cout << " -> ";
    }
    std::cout << "\n  Total Tour Cost: " << errandTour.totalCost << "m\n";
    ok("Errand tour cost equals the best of all 6 orders; path is walkable");
}

// ============================================================ Speedrunner ===
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
    const int start = idOf(graph, "D24");
    const int end   = idOf(graph, "Lab 2");

    // Feasible: D24 -> {Masjid, CS Canteen, Library, Lab 5} -> Lab 2 in 90 min.
    std::vector<int> errands{idOf(graph, "Masjid"), idOf(graph, "CS Canteen"),
                             idOf(graph, "Library"), idOf(graph, "Lab 5")};
    SpeedrunResult fit = planBreak(graph, start, end, errands,
                                   BREAK_START, BREAK_START + 90, HOUR, false);
    assert(fit.feasible);
    assert(fit.visitOrder.size() == 4);
    for (int id : errands) {                                     // each once
        int seen = 0;
        for (int v : fit.visitOrder)
            if (v == id) ++seen;
        assert(seen == 1);
    }
    assert(fit.totalMinutes <= 90.0);
    assert(std::abs(fit.totalMinutes -
                    minutesForOrder(graph, start, end, fit.visitOrder, HOUR)) < EPS);
    assert(fit.fullPath.front() == start && fit.fullPath.back() == end);
    assert(isWalkablePath(graph, fit.fullPath));
    std::size_t at = 0;                                          // errands appear
    for (int id : fit.visitOrder) {                              // in visit order
        while (at < fit.fullPath.size() && fit.fullPath[at] != id) ++at;
        assert(at < fit.fullPath.size());
    }
    ok("4 errands fit in 90 min; time matches an independent recount");

    // Optimality: no other order of the same errands is faster.
    for (int a = 0; a < 4; ++a)
        for (int b = 0; b < 4; ++b)
            for (int c = 0; c < 4; ++c)
                for (int d = 0; d < 4; ++d) {
                    if (a == b || a == c || a == d || b == c || b == d || c == d)
                        continue;
                    std::vector<int> order{errands[a], errands[b], errands[c], errands[d]};
                    assert(fit.totalMinutes <=
                           minutesForOrder(graph, start, end, order, HOUR) + EPS);
                }
    ok("Chosen order is no slower than any of the 24 orders");

    std::cout << "  Plan (" << fit.totalMinutes << " min):";
    for (int id : fit.visitOrder)
        std::cout << ' ' << graph.getNode(id)->getName();
    std::cout << '\n';

    // Infeasible: the Masjid errand alone needs 15 min of queueing; break is 5 min.
    const int masjid = idOf(graph, "Masjid");
    SpeedrunResult shortBreak = planBreak(graph, start, end, {masjid},
                                          BREAK_START, BREAK_START + 5, HOUR, false);
    assert(!shortBreak.feasible);
    assert(shortBreak.totalMinutes > 15.0);                      // real time kept
    assert(shortBreak.visitOrder.size() == 1 && shortBreak.visitOrder[0] == masjid);
    assert(shortBreak.fullPath.front() == start && shortBreak.fullPath.back() == end);
    assert(isWalkablePath(graph, shortBreak.fullPath));
    std::cout << "  5-min break: " << shortBreak.totalMinutes - 5.0
              << " minutes short\n";
    ok("Too-short break: feasible=false but best order, path and time returned");

    // Invalid input: feasible=false, totalMinutes=-1, empty vectors.
    SpeedrunResult bad[] = {
        planBreak(graph, start, end, {3, 4, 5, 6, 7}, BREAK_START, BREAK_START + 60, HOUR, false),
        planBreak(graph, start, end, {999},           BREAK_START, BREAK_START + 60, HOUR, false),
        planBreak(graph, start, end, {-1},            BREAK_START, BREAK_START + 60, HOUR, false),
        planBreak(graph, 999, end, {},                BREAK_START, BREAK_START + 60, HOUR, false),
        planBreak(graph, start, end, {masjid, masjid}, BREAK_START, BREAK_START + 60, HOUR, false),
        planBreak(graph, start, end, {},              BREAK_START, BREAK_START,      HOUR, false),
    };
    for (const SpeedrunResult& r : bad) {
        assert(!r.feasible);
        assert(r.totalMinutes == -1.0);
        assert(r.visitOrder.empty() && r.fullPath.empty());
    }
    ok("Rejected: 5 errands, unknown/negative node, duplicate, empty break");

    // Zero errands: walk straight from start to end.
    SpeedrunResult direct = planBreak(graph, start, end, {},
                                      BREAK_START, BREAK_START + 60, HOUR, false);
    PathResult straight = graph.dijkstra(start, end, HOUR, false);
    assert(direct.feasible);
    assert(direct.visitOrder.empty());
    assert(direct.fullPath == straight.path);
    assert(std::abs(direct.totalMinutes - straight.totalCost / 80.0) < EPS);
    SpeedrunResult stay = planBreak(graph, start, start, {},
                                    BREAK_START, BREAK_START + 60, HOUR, false);
    assert(stay.feasible && stay.totalMinutes == 0.0);
    assert(stay.fullPath.size() == 1 && stay.fullPath[0] == start);
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
        bool digitsOnly = !line.empty();
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
        // A real number that is too long for stoi is simply out of range.
        bool tooLong = line.size() - start > 9;
        int value = tooLong ? 0 : std::stoi(line);
        if (tooLong || value < lo || value > hi) {
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

// ------------------------------------------------------------- locationTag ---
// "[EE, Floor D]", "[CS, Basement]", "[MPB, Ground]" or "[Outdoor]".
static std::string locationTag(const CampusNode* node) {
    const std::string& b = node->getBuilding();
    const std::string& f = node->getFloor();
    if (b == "-")
        return "[Outdoor]";
    if (f == "Basement" || f == "Ground")
        return "[" + b + ", " + f + "]";
    return "[" + b + ", Floor " + f + "]";
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

        // The trie drops most punctuation, so "!!!" would search for "" and
        // list every place.  Require at least one letter or digit.
        bool hasAlnum = false;
        for (char c : prefix)
            if (std::isalnum(static_cast<unsigned char>(c)))
                hasAlnum = true;
        if (!hasAlnum) {
            std::cout << "    Type at least one letter or digit of the name.\n";
            continue;
        }

        std::vector<std::pair<std::string, int>> matches = names.autocomplete(prefix);
        if (matches.empty()) {
            std::cout << "    No place starts with \"" << prefix << "\". Try again.\n";
            continue;
        }

        const int count = static_cast<int>(matches.size());
        for (int i = 0; i < count; ++i) {
            const CampusNode* node = graph.getNode(matches[i].second);
            std::cout << "    " << std::setw(2) << (i + 1) << ". "
                      << std::left << std::setw(18) << node->getName()
                      << std::setw(18) << locationTag(node)
                      << std::right << node->getType() << '\n';
        }
        if (count == 1) {                               // nothing to choose
            const CampusNode* only = graph.getNode(matches[0].second);
            std::cout << "    -> " << only->getName() << ' ' << locationTag(only) << '\n';
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
              << "  Location : " << locationTag(node) << '\n'
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

// ----------------------------------------------------------------- readHour ---
static const double WALK_METERS_PER_MIN = 80.0;   // same pace as Speedrunner

static bool readHour(int& hour) {
    return readInt("  Hour of day (8-15): ", 8, 15, hour);
}

// The edge from 'from' to 'to', or nullptr if they are not adjacent.
static const Edge* findEdge(const Graph& graph, int from, int to) {
    for (const Edge& e : graph.getNeighbors(from))
        if (e.to == to)
            return &e;
    return nullptr;
}

// ------------------------------------------------------- floors and stairs ---
// Height order of a floor: Basement < Ground/A < 1/B < ...
static int floorRank(const std::string& floor) {
    if (floor == "Basement") return -1;
    if (floor == "Ground")   return 0;
    if (floor.size() == 1 && floor[0] >= 'A' && floor[0] <= 'Z')
        return floor[0] - 'A';                      // EE: A is ground
    bool digits = !floor.empty();
    for (char c : floor)
        if (c < '0' || c > '9') digits = false;
    return digits ? std::stoi(floor) : 0;
}

// Rooms only link to their own floor hub, so an edge between two nodes of the
// same building on different floors is a flight of stairs.
static bool isStairs(const Graph& graph, int a, int b) {
    const CampusNode* na = graph.getNode(a);
    const CampusNode* nb = graph.getNode(b);
    return na->getBuilding() != "-" && na->getBuilding() == nb->getBuilding()
        && na->getFloor() != nb->getFloor();
}

// Path as names, with each run of stairs collapsed, e.g.
// "EE Floor D -> (stairs down 3) -> EE Floor A".  Counts the floors changed.
static std::string describePath(const Graph& graph, const std::vector<int>& path,
                                int& floorChanges) {
    floorChanges = 0;
    if (path.empty())
        return "";
    std::string out = graph.getNode(path[0])->getName();
    std::size_t i = 0;
    while (i + 1 < path.size()) {
        if (!isStairs(graph, path[i], path[i + 1])) {
            out += " -> " + graph.getNode(path[i + 1])->getName();
            ++i;
            continue;
        }
        std::size_t j = i;
        while (j + 1 < path.size() && isStairs(graph, path[j], path[j + 1]))
            ++j;
        int delta = floorRank(graph.getNode(path[j])->getFloor())
                  - floorRank(graph.getNode(path[i])->getFloor());
        floorChanges += static_cast<int>(j - i);
        out += std::string(" -> (stairs ") + (delta > 0 ? "up " : "down ")
             + std::to_string(delta > 0 ? delta : -delta) + ") -> "
             + graph.getNode(path[j])->getName();
        i = j;
    }
    return out;
}

// ---------------------------------------------------------------- showRoute ---
// Menu option: shortest route between two places with Graph::dijkstra.
//
// Dijkstra minimises the rush-hour weighted cost; sun avoidance is off.
// Distance is the real metres (stairs count as 20 m each); time uses the
// rush-hour weight at 80 m/min.
static void showRoute(const Graph& graph, const PrefixTrie& names) {
    std::cout << "\n============ Route between two places ==================\n";
    int start = pickNode(graph, names, "Start");
    if (start < 0) return;
    int end = pickNode(graph, names, "Destination");
    if (end < 0) return;
    if (start == end) {                         // nothing to route
        std::cout << "  You're already at " << graph.getNode(start)->getName() << ".\n";
        return;
    }
    int hour;
    if (!readHour(hour))
        return;

    PathResult route = graph.dijkstra(start, end, hour, false);
    std::cout << "--------------------------------------------------------\n";
    if (route.path.empty()) {
        std::cout << "  No walkable route from " << graph.getNode(start)->getName()
                  << " to " << graph.getNode(end)->getName() << ".\n";
        return;
    }

    double meters = 0.0, walkCost = 0.0;
    for (std::size_t i = 0; i + 1 < route.path.size(); ++i) {
        const Edge* e = findEdge(graph, route.path[i], route.path[i + 1]);
        meters   += e->distance;
        walkCost += graph.calculateWeight(*e, hour, false);
    }
    int floorChanges;
    std::string summary = describePath(graph, route.path, floorChanges);

    std::cout << "  From     : " << graph.getNode(start)->getName() << ' '
              << locationTag(graph.getNode(start)) << '\n'
              << "  To       : " << graph.getNode(end)->getName() << ' '
              << locationTag(graph.getNode(end)) << '\n'
              << "  Path     : " << summary << '\n'
              << "  Floors   : " << floorChanges
              << (floorChanges == 1 ? " floor change\n" : " floor changes\n")
              << std::fixed << std::setprecision(1)
              << "  Distance : " << meters << " m\n"
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
    const bool avoidSun = false;                 // sun avoidance is not offered
    if (!readHour(hour))
        return;

    MeetupResult meet = graph.findSquadMeetup(squad, hour, avoidSun);
    std::cout << "--------------------------------------------------------\n";
    if (meet.meetupNodeId < 0) {
        std::cout << "  No place can be reached by everyone.\n";
        return;
    }

    const std::string meetName = graph.getNode(meet.meetupNodeId)->getName();
    std::cout << "  Meet at  : " << meetName << ' '
              << locationTag(graph.getNode(meet.meetupNodeId)) << '\n'
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
              << "  " << std::left << std::setw(4) << "#" << std::setw(8) << "Room"
              << std::setw(16) << "Where"
              << std::right << std::setw(7) << "Score" << std::setw(12) << "Distance"
              << std::setw(10) << "Free for" << '\n'
              << "  " << std::string(56, '-') << '\n'
              << std::fixed << std::setprecision(1);
    for (std::size_t i = 0; i < top.size(); ++i) {
        const BunkerResult& r = top[i];
        int untilHour = hour + r.freeHours;
        std::cout << "  " << std::left << std::setw(4) << (i + 1)
                  << std::setw(8) << r.name
                  << std::setw(16) << locationTag(graph.getNode(r.roomId))
                  << std::right << std::setw(7) << r.score
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

// One schedule line: "HH:MM - HH:MM  Title  @ Location [Building, Floor]".
static void printEvent(const Graph& graph, const ScheduleItem& item) {
    const CampusNode* where = graph.getNode(item.locationId);
    std::cout << minutesToTime(item.startMin) << " - " << minutesToTime(item.endMin)
              << "  " << std::left << std::setw(22) << item.title << std::right
              << " @ " << where->getName() << ' ' << locationTag(where) << '\n';
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
    if (day.getSchedule().empty()) {
        std::cout << "  Your schedule is empty. Add events in My Schedule (option 6).\n";
        return;
    }
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
    int floorChanges;
    std::cout << "  Path     : " << describePath(graph, route.path, floorChanges) << '\n'
              << "  Floors   : " << floorChanges
              << (floorChanges == 1 ? " floor change" : " floor changes")
              << std::fixed << std::setprecision(1)
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

// --------------------------------------------------------------- planABreak ---
// Menu option: Speedrunner.  Collect the break window, start/end places and
// up to MAX_ERRANDS errands, then show planBreak's best order and path.
static void planABreak(const Graph& graph, const PrefixTrie& names) {
    std::cout << "\n============== Plan a break (Speedrunner) ==============\n";
    int breakStart, breakEnd;
    if (!readTime("  Break starts (HH:MM): ", breakStart))
        return;
    while (true) {
        if (!readTime("  Break ends   (HH:MM): ", breakEnd))
            return;
        if (breakEnd > breakStart)
            break;
        std::cout << "  The break must end after " << minutesToTime(breakStart) << ".\n";
    }

    int start = pickNode(graph, names, "Start at");
    if (start < 0) return;
    int end = pickNode(graph, names, "Finish at");
    if (end < 0) return;

    int count;
    if (!readInt("  How many errands (0-" + std::to_string(MAX_ERRANDS) + ")? ",
                 0, MAX_ERRANDS, count))
        return;
    std::vector<int> errands;
    while (static_cast<int>(errands.size()) < count) {
        int id = pickNode(graph, names, "Errand " + std::to_string(errands.size() + 1));
        if (id < 0) return;
        bool duplicate = false;
        for (int e : errands)
            if (e == id) duplicate = true;
        if (duplicate) {
            std::cout << "    " << graph.getNode(id)->getName()
                      << " is already on the list. Pick a different place.\n";
            continue;
        }
        errands.push_back(id);
    }

    int hour;
    const bool avoidSun = false;                 // sun avoidance is not offered
    if (!readHour(hour))
        return;

    SpeedrunResult plan = planBreak(graph, start, end, errands,
                                    breakStart, breakEnd, hour, avoidSun);
    std::cout << "--------------------------------------------------------\n";
    if (plan.totalMinutes < 0) {
        std::cout << "  Could not plan this break: some place cannot be reached.\n";
        return;
    }

    std::cout << "  Visit order :";
    if (plan.visitOrder.empty())
        std::cout << " (no errands, go straight there)";
    for (std::size_t i = 0; i < plan.visitOrder.size(); ++i) {
        const CampusNode* stop = graph.getNode(plan.visitOrder[i]);
        std::cout << "\n    " << (i + 1) << ". " << stop->getName() << ' ' << locationTag(stop);
    }
    int floorChanges;
    std::cout << "\n  Full path   : " << describePath(graph, plan.fullPath, floorChanges)
              << "\n  Floors      : " << floorChanges
              << (floorChanges == 1 ? " floor change" : " floor changes");

    const int breakLen = breakEnd - breakStart;
    std::cout << std::fixed << std::setprecision(1)
              << "\n  Total time  : " << plan.totalMinutes << " min (walking + queues) of a "
              << breakLen << " min break\n";
    if (plan.feasible)
        std::cout << "  It fits! Done by about "
                  << minutesToTime(breakStart + static_cast<int>(plan.totalMinutes + 0.999))
                  << ", " << breakLen - plan.totalMinutes << " min to spare.\n";
    else
        std::cout << "  You're " << plan.totalMinutes - breakLen
                  << " minutes short. Drop an errand or find a longer break.\n";
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
              << "  8. Plan a break (Speedrunner)\n"
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
    const int MAX_CHOICE = 8;
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
            case 8: planABreak(graph, names);        break;
        }
    }

    std::cout << "\nGoodbye!\n";
    return 0;
}
