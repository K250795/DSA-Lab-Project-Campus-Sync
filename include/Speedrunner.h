#pragma once
#include <vector>
#include "Graph.h"

struct SpeedrunResult {
    bool             feasible;      // best order fits in the break
    std::vector<int> visitOrder;    // errand node ids, best order
    std::vector<int> fullPath;      // every node walked, start to end
    double           totalMinutes;  // walking + queues; set even if it doesn't fit; -1 if no plan
};

static constexpr int MAX_ERRANDS = 4;

// Finds the fastest order to visit up to MAX_ERRANDS errands between
// startNode and endNode, and whether it fits in [breakStartMin, breakEndMin].
// Walking minutes = Dijkstra cost (rush/sun rules) / walkMetersPerMin;
// ServiceFacility errands add their queue wait.
//
// Returns totalMinutes = -1 with empty vectors for invalid input (too many
// errands, unknown or duplicate node, end <= start) or an unreachable stop.
// If the best order is too long, feasible = false but the other fields still
// describe it, so the caller can say how many minutes short it is.
//
// Cost: 2(k+1) Dijkstra runs plus at most k! orders of O(k) lookups each.
SpeedrunResult planBreak(const Graph& g, int startNode, int endNode,
                         const std::vector<int>& errands,
                         int breakStartMin, int breakEndMin,
                         int currentHour, bool avoidSun,
                         double walkMetersPerMin = 80.0);
