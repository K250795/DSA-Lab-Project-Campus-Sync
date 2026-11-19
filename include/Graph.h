#pragma once

#include <vector>
#include <string>
#include "CampusNode.h"
#include "MinHeap.h"

// Represents a directed edge in the weighted adjacency list
struct Edge {
    int    to;
    double distance;          // base distance in meters
    bool   isOutdoor;         // true if walkway is outdoors (sun exposed)
    double rushMultiplier;    // multiplier applied during peak hours (11:00 - 14:00)
};

// Result of a single-source or point-to-point Dijkstra query
struct PathResult {
    std::vector<int> path;     // ordered sequence of node IDs from start to end
    double           totalCost; // total weighted travel cost / effective distance
};

// Result of a squad meetup query
struct MeetupResult {
    int                 meetupNodeId;      // vertex that minimizes collective cost
    double              totalCollectiveCost; // sum of travel costs across all users
    std::vector<double> userCosts;         // individual cost for each user in the squad
};

// Result of an errand tour optimization
struct ErrandResult {
    std::vector<int> visitOrder; // [startNode, errand_1, ..., errand_k, endNode]
    std::vector<int> fullPath;   // complete node-by-node path
    double           totalCost;  // total cost of the complete errand journey
};

// Spatial Graph Engine
// Implements a custom weighted adjacency list, CSV loaders, Dijkstra pathfinding
// using MinHeap, Squad-Sync meetup point solver, and Errand trip optimizer.
class Graph {
public:
    Graph();
    ~Graph();

    // Prevent accidental shallow copying to protect raw pointer ownership
    Graph(const Graph&) = delete;
    Graph& operator=(const Graph&) = delete;
    Graph(Graph&&) = delete;
    Graph& operator=(Graph&&) = delete;

    // Loaders
    bool loadNodes(const std::string& filepath);
    bool loadEdges(const std::string& filepath);
    bool loadGraph(const std::string& nodesFile, const std::string& edgesFile);

    // Queries
    int getNodeCount() const;
    CampusNode* getNode(int id) const;
    const std::vector<Edge>& getNeighbors(int id) const;

    // Weight calculation considering hour rush and sun avoidance
    double calculateWeight(const Edge& edge, int currentHour, bool avoidSun) const;

    // Dijkstra: single-source to single-destination shortest path
    PathResult dijkstra(int startId, int endId, int currentHour, bool avoidSun) const;

    // Dijkstra: single-source to all reachable nodes (returns distance array)
    std::vector<double> dijkstraDistances(int startId, int currentHour, bool avoidSun) const;

    // Squad-Sync: finds the vertex minimizing total collective travel cost for all users
    MeetupResult findSquadMeetup(const std::vector<int>& userNodes, int currentHour, bool avoidSun) const;

    // Errand Optimization: tests permutations of errands (up to 4) to find optimal route
    ErrandResult optimizeErrands(int startNode, int endNode, const std::vector<int>& errandNodes, int currentHour, bool avoidSun) const;

private:
    std::vector<CampusNode*>        nodes_; // node storage (owns raw pointers)
    std::vector<std::vector<Edge>>  adj_;   // custom weighted adjacency list

    // Helper: generate permutations manually without forbidden STL algorithms
    static void generatePermutations(std::vector<int>& items, int l, int r, std::vector<std::vector<int>>& out);
};
