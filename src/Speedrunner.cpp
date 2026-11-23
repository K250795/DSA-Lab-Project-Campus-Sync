#include "Speedrunner.h"
#include "CampusNode.h"

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

// --------------------------------------------------------- buildCostMatrix ---
// Points are numbered: 0 = start, 1..k = errands[0..k-1], k+1 = end.
// cost[i][j] = walking minutes from point i to point j, for i in 0..k (every
// point we ever leave from) and j in 0..k+1 (every point we can walk to).
//
// One dijkstraDistances call per source gives that row in full, so the whole
// matrix needs k+1 Dijkstra runs.  Unreachable pairs stay +infinity.
static std::vector<std::vector<double>>
buildCostMatrix(const Graph& g, int startNode, int endNode,
                const std::vector<int>& errands,
                int currentHour, bool avoidSun, double walkMetersPerMin) {
    const int k = static_cast<int>(errands.size());

    std::vector<int> point(k + 2);
    point[0] = startNode;
    for (int i = 0; i < k; ++i)
        point[i + 1] = errands[i];
    point[k + 1] = endNode;

    std::vector<std::vector<double>> cost(k + 1, std::vector<double>(k + 2));
    for (int i = 0; i <= k; ++i) {
        std::vector<double> meters =
            g.dijkstraDistances(point[i], currentHour, avoidSun);
        for (int j = 0; j <= k + 1; ++j)
            cost[i][j] = meters[point[j]] / walkMetersPerMin;   // inf / x = inf
    }
    return cost;
}

// ---------------------------------------------------------- addServiceWait ---
// Arriving at an errand that is a ServiceFacility means queueing there too,
// so its estimated wait is added to every leg that ends at it (column j for
// point j = 1..k).  Other node types (rooms, corridors) add nothing.
static void addServiceWait(const Graph& g, const std::vector<int>& errands,
                           std::vector<std::vector<double>>& cost) {
    const int k = static_cast<int>(errands.size());
    for (int j = 1; j <= k; ++j) {
        const ServiceFacility* facility =
            dynamic_cast<const ServiceFacility*>(g.getNode(errands[j - 1]));
        if (facility == nullptr)
            continue;
        double wait = facility->getEstimatedWaitMinutes();
        for (int i = 0; i <= k; ++i)
            cost[i][j] += wait;                         // inf + wait = inf
    }
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

    std::vector<std::vector<double>> cost =
        buildCostMatrix(g, startNode, endNode, errands,
                        currentHour, avoidSun, walkMetersPerMin);
    addServiceWait(g, errands, cost);

    // Choosing the order is added in the next steps.
    (void)cost;
    return result;
}
