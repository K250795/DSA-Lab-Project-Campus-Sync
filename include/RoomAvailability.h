#pragma once
#include <vector>
#include "HashMap.h"
#include "Graph.h"

// Room availability lookup table (room id -> 8-slot availability).
//
// Each AcademicRoom's bool schedule[8] (true = busy) is packed into one int
// bitmask stored in a HashMap:  bit i set  <=>  slot i is FREE.
// Slot i covers hour 8 + i (slot 0 = 08:00-09:00, ..., slot 7 = 15:00-16:00).
//
// Example: schedule "11010011" (busy) -> free slots 2, 4, 5 -> mask 0b00110100.
//
// Owns no raw pointers (HashMap and std::vector manage their own memory),
// so the compiler-generated copy operations and destructor are correct.
class RoomAvailability {
public:
    static constexpr int SLOT_COUNT = 8;
    static constexpr int FIRST_HOUR = 8;     // slot 0 starts at 08:00

    RoomAvailability() = default;

    // Rebuild the table from every AcademicRoom in 'graph'.
    // Non-academic nodes and empty id slots are skipped.
    // Returns the number of rooms loaded.
    int buildFromGraph(const Graph& graph);

    // Number of rooms in the table.
    int roomCount() const { return availability_.size(); }

    // True if 'roomId' is an academic room in the table.
    bool hasRoom(int roomId) const;

    // Free-slot bitmask for 'roomId' (bit i = slot i free), or -1 if the
    // room is not in the table.
    int getMask(int roomId) const;

    // True if 'roomId' is known and slot 'slot' (0-7) is free.
    // Unknown rooms and out-of-range slots return false.
    bool isFreeSlot(int roomId, int slot) const;

    // Same as isFreeSlot for a clock hour (8-15).
    bool isFreeAtHour(int roomId, int hour) const;

    // Number of free slots (0-8) for 'roomId', or -1 if unknown.
    int freeSlotCount(int roomId) const;

    // Ids of every room that is free at 'hour', in ascending id order.
    std::vector<int> freeRoomsAtHour(int hour) const;

private:
    HashMap          availability_;   // room id -> free-slot bitmask
    std::vector<int> roomIds_;        // loaded room ids, ascending (for iteration)
};
