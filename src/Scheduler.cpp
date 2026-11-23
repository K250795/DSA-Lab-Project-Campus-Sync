#include "Scheduler.h"
#include <fstream>
#include <sstream>
#include <stdexcept>

// --------------------------------------------------------------- addEvent ---
// Validate first, snapshot second, edit last: a rejected item never leaves a
// useless snapshot on the undo stack.
bool Scheduler::addEvent(const ScheduleItem& item) {
    if (!canInsert(item))
        return false;

    undo_.push(schedule_);                  // deep copy of the pre-edit state
    redo_.clear();                          // new edit invalidates redo history
    schedule_.insertSorted(item);
    return true;
}

// -------------------------------------------------------------- canInsert ---
// Shared by addEvent and loadFromCsv so both apply exactly the same checks.
bool Scheduler::canInsert(const ScheduleItem& item) const {
    if (item.endMin <= item.startMin)
        return false;                       // empty or reversed time range
    return !schedule_.hasOverlap(item);     // must not clash with an event
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
//
// Bad rows are skipped and counted instead of throwing.  Good rows pass the
// same canInsert() checks as addEvent but are inserted directly, so loading
// pushes no undo snapshots: the initial load cannot be undone.
bool Scheduler::loadFromCsv(const std::string& path, const Graph& g) {
    skippedLines_ = 0;
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
            continue;                       // blank lines are not errors

        ScheduleItem item;
        if (parseCsvRow(line, g, item) && canInsert(item))
            schedule_.insertSorted(item);
        else
            ++skippedLines_;
    }
    return true;
}

// ------------------------------------------------------------ parseCsvRow ---
// Split "title,start,end,locationId" and validate every field.
// Returns false (leaving 'out' unspecified) on any problem.
bool Scheduler::parseCsvRow(const std::string& line, const Graph& g,
                            ScheduleItem& out) {
    std::string fields[4];
    int count = 0;
    std::stringstream ss(line);
    std::string token;
    while (std::getline(ss, token, ',')) {
        if (count == 4)
            return false;                   // too many columns
        fields[count++] = token;
    }
    if (count != 4 || fields[0].empty())
        return false;                       // too few columns / no title

    // locationId: digits only (no sign, no trailing junk), short enough for int
    const std::string& loc = fields[3];
    if (loc.empty() || loc.size() > 9)
        return false;
    int locationId = 0;
    for (char c : loc) {
        if (c < '0' || c > '9')
            return false;
        locationId = locationId * 10 + (c - '0');
    }
    if (g.getNode(locationId) == nullptr)
        return false;                       // no such campus node

    try {
        out.startMin = timeToMinutes(fields[1]);
        out.endMin   = timeToMinutes(fields[2]);
    } catch (const std::invalid_argument&) {
        return false;                       // not a valid "HH:MM"
    }
    out.title      = fields[0];
    out.locationId = locationId;
    return true;
}
