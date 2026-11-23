#include "Speedrunner.h"
#include "CampusNode.h"
#include <limits>
#include <utility>   // std::swap

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

// Shared state for the permutation search, so the recursion only passes what
// changes from call to call.
struct OrderSearch {
    const std::vector<std::vector<double>>& cost;
    int              k;          // number of errands; end point is k+1
    double           limit;      // prune any partial route above this
    std::vector<int> order;      // order[0..k-1] = errand points, permuted
    std::vector<int> bestOrder;  // best complete order found so far
    double           bestTotal;  // its total minutes (+inf if none yet)
};

// ---------------------------------------------------------- searchOrders ---
// Recursive swap permutation: positions [0, pos) are fixed, 'prev' is the
// point we are standing at and 'soFar' the minutes spent getting there.
// For each candidate in [pos, k): swap it into 'pos', recurse, swap back.
//
// Every cost is >= 0, so a partial route can only get longer.  A branch is
// cut as soon as it exceeds the break ('limit') or can no longer beat the
// best complete order found so far.
static void searchOrders(OrderSearch& s, int pos, int prev, double soFar) {
    if (soFar > s.limit || soFar >= s.bestTotal)
        return;                                         // prune this branch

    if (pos == s.k) {                                   // all errands placed
        double total = soFar + s.cost[prev][s.k + 1];   // walk to the end
        if (total <= s.limit && total < s.bestTotal) {
            s.bestTotal = total;
            s.bestOrder = s.order;
        }
        return;
    }

    for (int i = pos; i < s.k; ++i) {
        std::swap(s.order[pos], s.order[i]);            // choose order[i] next
        int next = s.order[pos];
        searchOrders(s, pos + 1, next, soFar + s.cost[prev][next]);
        std::swap(s.order[pos], s.order[i]);            // undo the choice
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

    // Try every order of the errands, pruning past the break length.
    const int k = static_cast<int>(errands.size());
    OrderSearch search{cost, k, double(breakEndMin - breakStartMin),
                       std::vector<int>(k), {},
                       std::numeric_limits<double>::infinity()};
    for (int i = 0; i < k; ++i)
        search.order[i] = i + 1;                        // errand points 1..k
    searchOrders(search, 0, 0, 0.0);                    // start at point 0

    if (search.bestTotal == std::numeric_limits<double>::infinity())
        return result;                                  // nothing fits

    for (int p : search.bestOrder)
        result.visitOrder.push_back(errands[p - 1]);    // point -> node id
    result.totalMinutes = search.bestTotal;
    result.feasible     = true;
    return result;
}
