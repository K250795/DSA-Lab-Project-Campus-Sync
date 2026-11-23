#include "Scheduler.h"
#include <fstream>
#include <sstream>
#include <stdexcept>

// Validate before snapshotting so a rejected event leaves no undo entry.
bool Scheduler::addEvent(const ScheduleItem& item) {
    if (!canInsert(item))
        return false;

    undo_.push(schedule_);
    redo_.clear();
    schedule_.insertSorted(item);
    return true;
}

bool Scheduler::canInsert(const ScheduleItem& item) const {
    if (item.endMin <= item.startMin)
        return false;
    return !schedule_.hasOverlap(item);
}

// Find the node first so a missing title pushes no snapshot.
bool Scheduler::removeEvent(const std::string& title) {
    DLLNode* node = schedule_.findByTitle(title);
    if (node == nullptr)
        return false;

    undo_.push(schedule_);
    redo_.clear();
    schedule_.remove(node);
    return true;
}

bool Scheduler::undo() {
    if (undo_.empty())
        return false;

    redo_.push(schedule_);
    schedule_ = undo_.pop();
    return true;
}

// Unlike an edit, redo keeps the rest of the redo stack so redos can chain.
bool Scheduler::redo() {
    if (redo_.empty())
        return false;

    undo_.push(schedule_);
    schedule_ = redo_.pop();
    return true;
}

// Inserts directly instead of via addEvent so loading pushes no snapshots.
// O(n^2) overall: each insert walks the list.
bool Scheduler::loadFromCsv(const std::string& path, const Graph& g) {
    skippedLines_ = 0;
    std::ifstream file(path);
    if (!file.is_open())
        return false;

    std::string line;
    bool isHeader = true;
    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r')   // Windows line endings
            line.pop_back();
        if (isHeader) {
            isHeader = false;
            continue;
        }
        if (line.empty())
            continue;                               // blank lines aren't errors

        ScheduleItem item;
        if (parseCsvRow(line, g, item) && canInsert(item))
            schedule_.insertSorted(item);
        else
            ++skippedLines_;
    }
    return true;
}

// False on any malformed field; 'out' is then unspecified.
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
        return false;

    // Digits only (rejects "-1", "2x"); at most 9 so it can't overflow an int.
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
        return false;

    try {
        out.startMin = timeToMinutes(fields[1]);
        out.endMin   = timeToMinutes(fields[2]);
    } catch (const std::invalid_argument&) {
        return false;
    }
    out.title      = fields[0];
    out.locationId = locationId;
    return true;
}
