#pragma once
#include <vector>
#include "Graph.h"

// Result of planning errands inside a class break.
struct SpeedrunResult {
    bool             feasible;      // true if the best order fits in the break
    std::vector<int> visitOrder;    // errand node ids in the chosen order
    std::vector<int> fullPath;      // every node walked, start to end
    double           totalMinutes;  // walking + service waits for that order
                                    // (filled in even when it does not fit);
                                    // -1 if no plan could be made
};

// "Break Speedrunner": choose the order of up to MAX_ERRANDS errands that gets
// a student from 'startNode' to 'endNode' in the fewest minutes, and say
// whether it fits between breakStartMin and breakEndMin (minutes after
// midnight).
//
// Walking cost comes from Graph::dijkstraDistances (same rush-hour and sun
// rules, via 'currentHour' and 'avoidSun') divided by walkMetersPerMin.
// ServiceFacility errands also add their estimated queue wait.
//
// Invalid input (more than MAX_ERRANDS errands, an unknown or duplicate node,
// or breakEndMin <= breakStartMin) returns feasible = false and
// totalMinutes = -1 with empty vectors.  The same happens if some stop cannot
// be reached at all.
//
// If even the best order is longer than the break, feasible = false but
// visitOrder, fullPath and totalMinutes still describe that best order, so
// the caller can report "you're (totalMinutes - break length) minutes short".
static constexpr int MAX_ERRANDS = 4;

SpeedrunResult planBreak(const Graph& g, int startNode, int endNode,
                         const std::vector<int>& errands,
                         int breakStartMin, int breakEndMin,
                         int currentHour, bool avoidSun,
                         double walkMetersPerMin = 80.0);
