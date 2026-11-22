#pragma once
#include <string>
#include "DoublyLinkedList.h"
#include "UndoStack.h"

// Day-schedule manager.
//
// Owns the live schedule (a DoublyLinkedList kept in start-time order) and an
// UndoStack of deep-copy snapshots.  Every successful edit pushes a snapshot
// of the state *before* the edit, so the caller never has to remember to.
// Failed edits change nothing and push nothing.
//
// No raw pointers are owned directly: the members manage their own memory,
// so the default destructor is correct.  Copying is disabled because
// UndoStack is non-copyable.
class Scheduler {
public:
    Scheduler() = default;

    Scheduler(const Scheduler&)            = delete;
    Scheduler& operator=(const Scheduler&) = delete;

    // Insert 'item' in start-time order.
    // Returns false (and changes nothing) if the time range is invalid
    // (endMin <= startMin) or it overlaps an existing event.
    bool addEvent(const ScheduleItem& item);

    // Remove the first event whose title matches exactly.
    // Returns false (and changes nothing) if no event has that title.
    bool removeEvent(const std::string& title);

    // Restore the schedule to how it was before the most recent edit.
    // Returns false (and changes nothing) if there is nothing to undo.
    bool undo();

    // Re-apply the most recently undone edit.
    // Returns false (and changes nothing) if there is nothing to redo.
    // Any new successful addEvent/removeEvent clears the redo history.
    bool redo();

    // Read-only view of the live schedule, in start-time order.
    const DoublyLinkedList& getSchedule() const { return schedule_; }

private:
    DoublyLinkedList schedule_;   // live, start-time ordered schedule
    UndoStack        undo_;       // snapshots taken before each edit
    UndoStack        redo_;       // snapshots taken before each undo
};
