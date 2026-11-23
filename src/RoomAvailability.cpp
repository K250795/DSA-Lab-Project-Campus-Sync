#include "RoomAvailability.h"
#include "CampusNode.h"

// --------------------------------------------------------- buildFromGraph ---
// Walk every node id; for each AcademicRoom pack its 8 busy flags into a
// free-slot bitmask and store it in the hash map.
int RoomAvailability::buildFromGraph(const Graph& graph) {
    availability_.clear();
    roomIds_.clear();

    for (int id = 0; id < graph.getNodeCount(); ++id) {
        const AcademicRoom* room = dynamic_cast<const AcademicRoom*>(graph.getNode(id));
        if (room == nullptr)
            continue;                       // missing id or not an academic room

        int mask = 0;
        for (int slot = 0; slot < SLOT_COUNT; ++slot)
            if (room->isSlotFree(slot))
                mask |= (1 << slot);

        availability_.insert(room->getId(), mask);
        roomIds_.push_back(room->getId());  // ids visited in ascending order
    }
    return roomCount();
}

// ---------------------------------------------------------------- hasRoom ---
bool RoomAvailability::hasRoom(int roomId) const {
    return availability_.contains(roomId);
}

// ---------------------------------------------------------------- getMask ---
int RoomAvailability::getMask(int roomId) const {
    int mask = 0;
    return availability_.find(roomId, mask) ? mask : -1;
}

// ------------------------------------------------------------- isFreeSlot ---
bool RoomAvailability::isFreeSlot(int roomId, int slot) const {
    if (slot < 0 || slot >= SLOT_COUNT)
        return false;
    int mask = getMask(roomId);
    return mask != -1 && (mask & (1 << slot)) != 0;
}

// ----------------------------------------------------------- isFreeAtHour ---
bool RoomAvailability::isFreeAtHour(int roomId, int hour) const {
    return isFreeSlot(roomId, hour - FIRST_HOUR);
}

// ---------------------------------------------------------- freeSlotCount ---
// Count set bits by clearing the lowest one each pass (Kernighan's method).
int RoomAvailability::freeSlotCount(int roomId) const {
    int mask = getMask(roomId);
    if (mask == -1)
        return -1;

    int count = 0;
    while (mask != 0) {
        mask &= (mask - 1);
        ++count;
    }
    return count;
}

// -------------------------------------------------------- freeRoomsAtHour ---
std::vector<int> RoomAvailability::freeRoomsAtHour(int hour) const {
    std::vector<int> result;
    for (int id : roomIds_)
        if (isFreeAtHour(id, hour))
            result.push_back(id);
    return result;
}
