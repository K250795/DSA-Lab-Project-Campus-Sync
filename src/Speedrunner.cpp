#include "Speedrunner.h"
#include "CampusNode.h"
#include <limits>
#include <utility>

// Pairwise duplicate check: with at most 4 errands it beats a lookup table.
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
            return false;
        for (std::size_t j = i + 1; j < errands.size(); ++j)
            if (errands[i] == errands[j])
                return false;
    }
    return true;
}

// Points: 0 = start, 1..k = errands, k+1 = end. cost[i][j] = minutes from
// point i to point j. One Dijkstra per source fills a whole row, so the
// matrix takes k+1 runs instead of one per leg of every permutation.
// Unreachable pairs stay +infinity.
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
            cost[i][j] = meters[point[j]] / walkMetersPerMin;
    }
    return cost;
}

// Adds each ServiceFacility's queue wait to every leg arriving there, so the
// search only has to sum matrix entries.
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
            cost[i][j] += wait;
    }
}

struct OrderSearch {
    const std::vector<std::vector<double>>& cost;
    int              k;
    double           limit;      // prune partial routes longer than this
    std::vector<int> order;      // errand points 1..k, permuted in place
    std::vector<int> bestOrder;
    double           bestTotal;  // +inf until a complete order is found
};

// Recursive swap permutation: order[0..pos) is fixed, we stand at 'prev'
// after 'soFar' minutes. Costs are >= 0, so a branch can be cut once it
// exceeds the limit or can't beat the best order found so far.
static void searchOrders(OrderSearch& s, int pos, int prev, double soFar) {
    if (soFar > s.limit || soFar >= s.bestTotal)
        return;

    if (pos == s.k) {
        double total = soFar + s.cost[prev][s.k + 1];
        if (total <= s.limit && total < s.bestTotal) {
            s.bestTotal = total;
            s.bestOrder = s.order;
        }
        return;
    }

    for (int i = pos; i < s.k; ++i) {
        std::swap(s.order[pos], s.order[i]);
        int next = s.order[pos];
        searchOrders(s, pos + 1, next, soFar + s.cost[prev][next]);
        std::swap(s.order[pos], s.order[i]);
    }
}

// One Dijkstra per leg; each leg's first node repeats the previous leg's
// last node, so it is skipped.
static std::vector<int> buildFullPath(const Graph& g, int startNode, int endNode,
                                      const std::vector<int>& visitOrder,
                                      int currentHour, bool avoidSun) {
    std::vector<int> stops;
    stops.push_back(startNode);
    for (int id : visitOrder)
        stops.push_back(id);
    stops.push_back(endNode);

    std::vector<int> fullPath{startNode};
    for (std::size_t i = 0; i + 1 < stops.size(); ++i) {
        PathResult leg = g.dijkstra(stops[i], stops[i + 1], currentHour, avoidSun);
        for (std::size_t j = 1; j < leg.path.size(); ++j)
            fullPath.push_back(leg.path[j]);
    }
    return fullPath;
}

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

    // Pass 1: prune anything longer than the break.
    const int    k        = static_cast<int>(errands.size());
    const double INF      = std::numeric_limits<double>::infinity();
    const double breakLen = breakEndMin - breakStartMin;
    OrderSearch search{cost, k, breakLen, std::vector<int>(k), {}, INF};
    for (int i = 0; i < k; ++i)
        search.order[i] = i + 1;
    searchOrders(search, 0, 0, 0.0);

    // Pass 2, only if nothing fit: drop the limit to still find the best
    // order and report how far over the break it is.
    if (search.bestTotal == INF) {
        search.limit = INF;
        searchOrders(search, 0, 0, 0.0);
    }
    if (search.bestTotal == INF)
        return result;                                  // a stop is unreachable

    for (int p : search.bestOrder)
        result.visitOrder.push_back(errands[p - 1]);
    result.totalMinutes = search.bestTotal;
    result.feasible     = search.bestTotal <= breakLen;

    result.fullPath = buildFullPath(g, startNode, endNode, result.visitOrder,
                                    currentHour, avoidSun);
    return result;
}
