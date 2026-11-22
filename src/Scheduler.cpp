#include "Scheduler.h"

// --------------------------------------------------------------- addEvent ---
// Validate first, snapshot second, edit last: a rejected item never leaves a
// useless snapshot on the undo stack.
bool Scheduler::addEvent(const ScheduleItem& item) {
    if (item.endMin <= item.startMin)
        return false;                       // empty or reversed time range
    if (schedule_.hasOverlap(item))
        return false;                       // clashes with an existing event

    undo_.push(schedule_);                  // deep copy of the pre-edit state
    schedule_.insertSorted(item);
    return true;
}

// ------------------------------------------------------------ removeEvent ---
// Look up the node first so a missing title does not push a snapshot.
bool Scheduler::removeEvent(const std::string& title) {
    DLLNode* node = schedule_.findByTitle(title);
    if (node == nullptr)
        return false;

    undo_.push(schedule_);                  // deep copy of the pre-edit state
    schedule_.remove(node);                 // O(1) unlink; node is in schedule_
    return true;
}

// ------------------------------------------------------------------- undo ---
// Replace the live list with the most recent pre-edit snapshot.
bool Scheduler::undo() {
    if (undo_.empty())
        return false;

    schedule_ = undo_.pop();                // deep-copy assignment
    return true;
}
