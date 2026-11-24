#include "CampusNode.h"
#include <stdexcept>

// =============================================================== CampusNode ==
CampusNode::CampusNode(int id, const std::string& name, double xVal, double yVal)
    : nodeID(id), nodeName(name), x(xVal), y(yVal) {}

int CampusNode::getId() const {
    return nodeID;
}

const std::string& CampusNode::getName() const {
    return nodeName;
}

double CampusNode::getX() const {
    return x;
}

double CampusNode::getY() const {
    return y;
}

const std::string& CampusNode::getBuilding() const {
    return building_;
}

const std::string& CampusNode::getFloor() const {
    return floor_;
}

void CampusNode::setLocation(const std::string& building, const std::string& floor) {
    building_ = building;
    floor_    = floor;
}

// ============================================================= AcademicRoom ==
AcademicRoom::AcademicRoom(int id, const std::string& name, double xVal, double yVal, const std::string& scheduleStr)
    : CampusNode(id, name, xVal, yVal) {
    for (int i = 0; i < 8; ++i) {
        if (i < static_cast<int>(scheduleStr.size())) {
            schedule[i] = (scheduleStr[i] == '1');
        } else {
            schedule[i] = false;
        }
    }
}

std::string AcademicRoom::getType() const {
    return "ACADEMIC";
}

bool AcademicRoom::isSlotBusy(int slot) const {
    if (slot < 0 || slot >= 8)
        throw std::out_of_range("AcademicRoom slot index out of range (0-7)");
    return schedule[slot];
}

bool AcademicRoom::isSlotFree(int slot) const {
    return !isSlotBusy(slot);
}

void AcademicRoom::setSlot(int slot, bool busy) {
    if (slot < 0 || slot >= 8)
        throw std::out_of_range("AcademicRoom slot index out of range (0-7)");
    schedule[slot] = busy;
}

const bool* AcademicRoom::getSchedule() const {
    return schedule;
}

bool AcademicRoom::isAvailableAtHour(int hour) const {
    int slot = hour - 8; // 8:00 -> slot 0, ..., 15:00 -> slot 7
    if (slot < 0 || slot >= 8)
        return true; // outside monitored hours, assume available
    return isSlotFree(slot);
}

// ========================================================== ServiceFacility ==
ServiceFacility::ServiceFacility(int id, const std::string& name, double xVal, double yVal, int currentQueueVal, int avgServiceMinutesVal)
    : CampusNode(id, name, xVal, yVal),
      currentQueue(currentQueueVal),
      avgServiceMinutes(avgServiceMinutesVal) {}

std::string ServiceFacility::getType() const {
    return "SERVICE";
}

int ServiceFacility::getCurrentQueue() const {
    return currentQueue;
}

void ServiceFacility::setCurrentQueue(int q) {
    currentQueue = q;
}

int ServiceFacility::getAvgServiceMinutes() const {
    return avgServiceMinutes;
}

void ServiceFacility::setAvgServiceMinutes(int m) {
    avgServiceMinutes = m;
}

int ServiceFacility::getEstimatedWaitMinutes() const {
    return currentQueue * avgServiceMinutes;
}

// ============================================================== TransitNode ==
TransitNode::TransitNode(int id, const std::string& name, double xVal, double yVal)
    : CampusNode(id, name, xVal, yVal) {}

std::string TransitNode::getType() const {
    return "TRANSIT";
}
