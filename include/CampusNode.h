#pragma once

#include <string>

// Abstract base class for all nodes on campus
class CampusNode {
public:
    CampusNode(int id, const std::string& name, double x, double y);
    virtual ~CampusNode() = default;

    int getId() const;
    const std::string& getName() const;
    double getX() const;
    double getY() const;

    // Building and floor ("-" for outdoor places), set by the CSV loader.
    const std::string& getBuilding() const;
    const std::string& getFloor() const;
    void setLocation(const std::string& building, const std::string& floor);

    virtual std::string getType() const = 0;

protected:
    int nodeID;
    std::string nodeName;
    double x;
    double y;
    std::string building_ = "-";
    std::string floor_    = "-";
};

// Derived class: AcademicRoom
// Contains an 8-slot daily schedule (e.g. 8:00 AM to 4:00 PM, slots 0-7)
// where true = busy/occupied, false = free/available.
class AcademicRoom : public CampusNode {
public:
    AcademicRoom(int id, const std::string& name, double x, double y, const std::string& scheduleStr = "00000000");

    std::string getType() const override;

    bool isSlotBusy(int slot) const;
    bool isSlotFree(int slot) const;
    void setSlot(int slot, bool busy);
    const bool* getSchedule() const;

    // Helper: checks slot corresponding to hour (e.g. hour 8-15 maps to slot 0-7)
    bool isAvailableAtHour(int hour) const;

private:
    bool schedule[8];
};

// Derived class: ServiceFacility
// Tracks queue length and average processing time
class ServiceFacility : public CampusNode {
public:
    ServiceFacility(int id, const std::string& name, double x, double y, int currentQueue = 0, int avgServiceMinutes = 5);

    std::string getType() const override;

    int getCurrentQueue() const;
    void setCurrentQueue(int q);

    int getAvgServiceMinutes() const;
    void setAvgServiceMinutes(int m);

    int getEstimatedWaitMinutes() const;

private:
    int currentQueue;
    int avgServiceMinutes;
};

// Derived class: TransitNode
// Hallways, corridors, stairs, gates, plazas
class TransitNode : public CampusNode {
public:
    TransitNode(int id, const std::string& name, double x, double y);

    std::string getType() const override;
};
