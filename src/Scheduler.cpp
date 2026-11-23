#include "Scheduler.h"
#include <fstream>
#include <sstream>

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

// ------------------------------------------------------------ loadFromCsv ---
// Same reading style as Graph's loaders: skip the header, strip a trailing
// '\r' (Windows line endings), split each row on commas.
bool Scheduler::loadFromCsv(const std::string& path, const Graph& g) {
    std::ifstream file(path);
    if (!file.is_open())
        return false;

    std::string line;
    bool isHeader = true;
    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        if (isHeader) {
            isHeader = false;
            continue;
        }
        if (line.empty())
            continue;

        std::stringstream ss(line);
        std::string title, start, end, loc;
        std::getline(ss, title, ',');
        std::getline(ss, start, ',');
        std::getline(ss, end, ',');
        std::getline(ss, loc, ',');

        ScheduleItem item{title, timeToMinutes(start), timeToMinutes(end),
                          std::stoi(loc)};
        addEvent(item);
    }
    (void)g;   // location validation is added in the next step
    return true;
}
