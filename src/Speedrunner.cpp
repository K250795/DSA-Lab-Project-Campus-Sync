#include "Speedrunner.h"

// -------------------------------------------------------------- validInput ---
// True if every rule in the header holds.  Duplicates are found with a pairwise
// scan: with at most MAX_ERRANDS items this is cheaper than any lookup table.
static bool validInput(const Graph& g, int startNode, int endNode,
                       const std::vector<int>& errands,
                       int breakStartMin, int breakEndMin,
                       double walkMetersPerMin) {
    if (breakEndMin <= breakStartMin || walkMetersPerMin <= 0.0)
        return false;
    if (static_cast<int>(errands.size()) > MAX_ERRANDS)
        return false;
    if (g.getNode(startNode) == nullptr || g.getNode(endNode) == nullptr)
        return false;

    for (std::size_t i = 0; i < errands.size(); ++i) {
        if (g.getNode(errands[i]) == nullptr)
            return false;                           // unknown node
        for (std::size_t j = i + 1; j < errands.size(); ++j)
            if (errands[i] == errands[j])
                return false;                       // duplicate errand
    }
    return true;
}

// --------------------------------------------------------------- planBreak ---
SpeedrunResult planBreak(const Graph& g, int startNode, int endNode,
                         const std::vector<int>& errands,
                         int breakStartMin, int breakEndMin,
                         int currentHour, bool avoidSun,
                         double walkMetersPerMin) {
    SpeedrunResult result{false, {}, {}, -1.0};
    if (!validInput(g, startNode, endNode, errands,
                    breakStartMin, breakEndMin, walkMetersPerMin))
        return result;

    // Planning is added in the next steps.
    (void)currentHour;
    (void)avoidSun;
    return result;
}
