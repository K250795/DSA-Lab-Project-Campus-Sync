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
    redo_.clear();                          // new edit invalidates redo history
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
    redo_.clear();                          // new edit invalidates redo history
    schedule_.remove(node);                 // O(1) unlink; node is in schedule_
    return true;
}

// ------------------------------------------------------------------- undo ---
// Save the current state for redo, then restore the most recent pre-edit
// snapshot.
bool Scheduler::undo() {
    if (undo_.empty())
        return false;

    redo_.push(schedule_);                  // so this undo can be redone
    schedule_ = undo_.pop();                // deep-copy assignment
    return true;
}

// ------------------------------------------------------------------- redo ---
// Mirror of undo(): save the current state for undo, then restore the most
// recently undone state.  Does not clear redo_, so several redos can chain.
bool Scheduler::redo() {
    if (redo_.empty())
        return false;

    undo_.push(schedule_);                  // so this redo can be undone
    schedule_ = redo_.pop();                // deep-copy assignment
    return true;
}
