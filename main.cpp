// main.cpp – sanity-check driver for all data structures and Graph engine.
// All output lives here (UI layer); no cout/cin inside DS/Graph implementations.

#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include <stdexcept>

#include "MinHeap.h"
#include "MaxHeap.h"
#include "DoublyLinkedList.h"
#include "UndoStack.h"
#include "Scheduler.h"
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

// ===================================================================== main ===
int main() {
    std::cout << "========================================================\n";
    std::cout << "        Campus-Sync – Comprehensive System Verification\n";
    std::cout << "========================================================\n";

    // Milestone 1 Core Data Structures
    testMinHeap();
    testMaxHeap();
    testDLLAndUndoStack();
    testScheduler();
    testPrefixTrie();

    // Milestone 2 Campus Data & Spatial Graph Engine
    testCampusGraph();

    std::cout << "\n========================================================\n";
    std::cout << "  All system verification checks passed. Zero crashes.\n";
    std::cout << "========================================================\n";
    return 0;
}
