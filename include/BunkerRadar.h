#pragma once
#include <vector>
#include "Graph.h"
#include "RoomAvailability.h"

// "Bunker Radar": finds empty academic rooms a student can sit in.
//
// Reads (never modifies) the campus Graph and the RoomAvailability table
// (room id -> free-slot bitmask in a custom HashMap).
//
// Ownership: holds const references only and owns no memory.  The Graph and
// RoomAvailability passed to the constructor must outlive this object.
class BunkerRadar {
public:
    static constexpr int FIRST_HOUR = 8;    // earliest bookable hour (slot 0)
    static constexpr int LAST_HOUR  = 15;   // latest bookable hour   (slot 7)

    BunkerRadar(const Graph& graph, const RoomAvailability& rooms);

    // Ids of every AcademicRoom that is free at 'hour' (8-15), ascending.
    // Returns an empty vector for an hour outside 8-15.
    std::vector<int> findFreeRooms(int hour) const;

private:
    const Graph&            graph_;
    const RoomAvailability& rooms_;
};
