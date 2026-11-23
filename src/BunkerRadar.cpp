#include "BunkerRadar.h"
#include "CampusNode.h"
#include "MaxHeap.h"
#include <limits>

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

// --------------------------------------------------------------- topRooms ---
// 1. Find free rooms (hash-map lookups).
// 2. One Dijkstra from the user gives every room's distance at once.
// 3. Score each reachable room and push (id, score) into the MaxHeap.
// 4. Pop up to 'topN' times: each pop yields the best remaining room.
std::vector<BunkerResult> BunkerRadar::topRooms(int userNode, int hour, int topN,
                                                bool avoidSun) const {
    std::vector<BunkerResult> result;
    if (topN <= 0 || graph_.getNode(userNode) == nullptr)
        return result;

    std::vector<int> freeRooms = findFreeRooms(hour);   // empty if hour invalid
    if (freeRooms.empty())
        return result;

    const double INF = std::numeric_limits<double>::infinity();
    std::vector<double> dist = graph_.dijkstraDistances(userNode, hour, avoidSun);

    MaxHeap heap;
    for (int id : freeRooms) {
        if (dist[id] == INF)
            continue;                                   // cannot walk there
        heap.push(id, utilityScore(consecutiveFreeHours(id, hour), dist[id]));
    }

    while (!heap.empty() && static_cast<int>(result.size()) < topN) {
        MaxHeapNode best = heap.top();
        heap.pop();
        result.push_back({best.id,
                          graph_.getNode(best.id)->getName(),
                          best.score,
                          dist[best.id],
                          consecutiveFreeHours(best.id, hour)});
    }
    return result;
}
