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

    // Utility score formula (higher = better bunker):
    //
    //     score = HOUR_POINTS * freeHours  -  distanceMeters / METERS_PER_POINT
    //
    //   freeHours      = consecutive free hours starting at the query hour
    //                    (1-8), i.e. how long you can stay before the room
    //                    is needed for a class.
    //   distanceMeters = shortest walking cost from the user's node
    //                    (Graph::dijkstraDistances, same rush/sun rules).
    //
    // With 10 points per hour and 1 point per 10 m, one extra free hour is
    // worth exactly 100 m of extra walking.
    static constexpr double HOUR_POINTS      = 10.0;
    static constexpr double METERS_PER_POINT = 10.0;

    BunkerRadar(const Graph& graph, const RoomAvailability& rooms);

    // Ids of every AcademicRoom that is free at 'hour' (8-15), ascending.
    // Returns an empty vector for an hour outside 8-15.
    std::vector<int> findFreeRooms(int hour) const;

    // Consecutive free hours for 'roomId' starting at 'hour' and running
    // until the first busy slot or 16:00.  Returns 0 if the room is unknown,
    // busy at 'hour', or 'hour' is outside 8-15.
    int consecutiveFreeHours(int roomId, int hour) const;

    // Apply the documented formula to raw inputs.
    static double utilityScore(int freeHours, double distanceMeters);

private:
    const Graph&            graph_;
    const RoomAvailability& rooms_;
};
