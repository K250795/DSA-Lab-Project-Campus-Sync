#include "BunkerRadar.h"
#include "CampusNode.h"

// ----------------------------------------------------------- constructor ---
BunkerRadar::BunkerRadar(const Graph& graph, const RoomAvailability& rooms)
    : graph_(graph), rooms_(rooms) {}

// ---------------------------------------------------------- findFreeRooms ---
// The availability table answers "free at this hour?" in O(1) per room; the
// graph check guards against a table built from a different graph.
std::vector<int> BunkerRadar::findFreeRooms(int hour) const {
    std::vector<int> result;
    if (hour < FIRST_HOUR || hour > LAST_HOUR)
        return result;

    for (int id : rooms_.freeRoomsAtHour(hour)) {
        if (dynamic_cast<const AcademicRoom*>(graph_.getNode(id)) != nullptr)
            result.push_back(id);
    }
    return result;
}

// --------------------------------------------------- consecutiveFreeHours ---
// Walk forward slot by slot from 'hour' while the room stays free.
int BunkerRadar::consecutiveFreeHours(int roomId, int hour) const {
    int count = 0;
    for (int h = hour; h >= FIRST_HOUR && h <= LAST_HOUR; ++h) {
        if (!rooms_.isFreeAtHour(roomId, h))
            break;
        ++count;
    }
    return count;
}

// ----------------------------------------------------------- utilityScore ---
double BunkerRadar::utilityScore(int freeHours, double distanceMeters) {
    return HOUR_POINTS * freeHours - distanceMeters / METERS_PER_POINT;
}
