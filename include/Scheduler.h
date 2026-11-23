#pragma once
#include <string>
#include "DoublyLinkedList.h"
#include "UndoStack.h"
#include "Graph.h"

// The student's day: a start-time ordered list plus undo/redo stacks.
// Each successful edit snapshots the previous state; failed edits change
// nothing and push nothing. Not copyable because UndoStack isn't.
class Scheduler {
public:
    Scheduler() = default;

    Scheduler(const Scheduler&)            = delete;
    Scheduler& operator=(const Scheduler&) = delete;

    // False if endMin <= startMin or the event overlaps another.
    bool addEvent(const ScheduleItem& item);

    // Removes the first exact title match; false if none.
    bool removeEvent(const std::string& title);

    // False if there is nothing to undo/redo. A new edit clears the redo history.
    bool undo();
    bool redo();

    // Reads "title,start,end,locationId" rows (times as "HH:MM").
    // Bad rows (column count, time format, end <= start, unknown node,
    // overlap) are skipped and counted, never thrown. Pushes no undo
    // snapshots, so the load itself can't be undone.
    // Returns false only if the file can't be opened.
    bool loadFromCsv(const std::string& path, const Graph& g);

    // Rows skipped by the most recent loadFromCsv.
    int getSkippedCount() const { return skippedLines_; }

    const DoublyLinkedList& getSchedule() const { return schedule_; }

private:
    // Checks shared by addEvent and loadFromCsv.
    bool canInsert(const ScheduleItem& item) const;

    static bool parseCsvRow(const std::string& line, const Graph& g,
                            ScheduleItem& out);

    DoublyLinkedList schedule_;
    UndoStack        undo_;
    UndoStack        redo_;
    int              skippedLines_ = 0;
};
