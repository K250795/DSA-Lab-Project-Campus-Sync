#include "Graph.h"
#include <fstream>
#include <sstream>
#include <limits>
#include <algorithm>

// ======================================================= Lifecycle & Memory ==
Graph::Graph() = default;

Graph::~Graph() {
    for (CampusNode* ptr : nodes_) {
        delete ptr;
    }
    nodes_.clear();
}

// ============================================================== CSV Loaders ==
bool Graph::loadNodes(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open())
        return false;

    std::string line;
    bool isHeader = true;

    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();

        if (line.empty())
            continue;

        if (isHeader) {
            isHeader = false;
            // Check if first line is indeed a header
            if (line.rfind("id,", 0) == 0 || line.rfind("ID,", 0) == 0)
                continue;
        }

        std::stringstream ss(line);
        std::string token;

        // format: id,name,type,x,y,schedule
        int id = -1;
        std::string name;
        std::string type;
        double x = 0.0;
        double y = 0.0;
        std::string schedule = "00000000";

        if (std::getline(ss, token, ',')) id = std::stoi(token);
        if (std::getline(ss, name, ',')) {}
        if (std::getline(ss, type, ',')) {}
        if (std::getline(ss, token, ',')) x = std::stod(token);
        if (std::getline(ss, token, ',')) y = std::stod(token);
        if (std::getline(ss, schedule, ',')) {}

        if (id < 0) continue;

        if (id >= static_cast<int>(nodes_.size())) {
            nodes_.resize(id + 1, nullptr);
            adj_.resize(id + 1);
        }

        CampusNode* node = nullptr;
        if (type == "ACADEMIC") {
            node = new AcademicRoom(id, name, x, y, schedule);
        } else if (type == "SERVICE") {
            node = new ServiceFacility(id, name, x, y, 5, 3);
        } else {
            node = new TransitNode(id, name, x, y);
        }

        if (nodes_[id] != nullptr) {
            delete nodes_[id];
        }
        nodes_[id] = node;
    }

    return true;
}

bool Graph::loadEdges(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open())
        return false;

    std::string line;
    bool isHeader = true;

    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();

        if (line.empty())
            continue;

        if (isHeader) {
            isHeader = false;
            if (line.rfind("from,", 0) == 0 || line.rfind("FROM,", 0) == 0)
                continue;
        }

        std::stringstream ss(line);
        std::string token;

        // format: from,to,distance,is_outdoor,rush_multiplier
        int u = -1;
        int v = -1;
        double dist = 0.0;
        bool isOutdoor = false;
        double rushMult = 1.0;

        if (std::getline(ss, token, ',')) u = std::stoi(token);
        if (std::getline(ss, token, ',')) v = std::stoi(token);
        if (std::getline(ss, token, ',')) dist = std::stod(token);
        if (std::getline(ss, token, ',')) isOutdoor = (std::stoi(token) != 0);
        if (std::getline(ss, token, ',')) rushMult = std::stod(token);

        if (u < 0 || v < 0) continue;

        int maxId = std::max(u, v);
        if (maxId >= static_cast<int>(adj_.size())) {
            adj_.resize(maxId + 1);
            nodes_.resize(maxId + 1, nullptr);
        }

        // Bidirectional walkways
        adj_[u].push_back({v, dist, isOutdoor, rushMult});
        adj_[v].push_back({u, dist, isOutdoor, rushMult});
    }

    return true;
}

bool Graph::loadGraph(const std::string& nodesFile, const std::string& edgesFile) {
    return loadNodes(nodesFile) && loadEdges(edgesFile);
}

// ================================================================= Queries ==
int Graph::getNodeCount() const {
    return static_cast<int>(nodes_.size());
}

CampusNode* Graph::getNode(int id) const {
    if (id < 0 || id >= static_cast<int>(nodes_.size()))
        return nullptr;
    return nodes_[id];
}

const std::vector<Edge>& Graph::getNeighbors(int id) const {
    static const std::vector<Edge> emptyList;
    if (id < 0 || id >= static_cast<int>(adj_.size()))
        return emptyList;
    return adj_[id];
}

double Graph::calculateWeight(const Edge& edge, int currentHour, bool avoidSun) const {
    double weight = edge.distance;

    // Rush hours: 11:00 AM to 2:00 PM (11–14 inclusive)
    if (currentHour >= 11 && currentHour <= 14) {
        weight *= edge.rushMultiplier;
    }

    // Direct sun avoidance: 2.5x penalty on outdoor paths
    if (avoidSun && edge.isOutdoor) {
        weight *= 2.5;
    }

    return weight;
}

// ================================================================ Dijkstra ==
PathResult Graph::dijkstra(int startId, int endId, int currentHour, bool avoidSun) const {
    const double INF = std::numeric_limits<double>::infinity();
    int n = static_cast<int>(nodes_.size());

    if (startId < 0 || startId >= n || nodes_[startId] == nullptr ||
        endId < 0 || endId >= n || nodes_[endId] == nullptr) {
        return {{}, INF};
    }

    if (startId == endId) {
        return {{startId}, 0.0};
    }

    std::vector<double> dist(n, INF);
    std::vector<int> parent(n, -1);
    dist[startId] = 0.0;

    MinHeap pq;
    pq.push(startId, 0.0);

    while (!pq.empty()) {
        MinHeapNode topNode = pq.top();
        pq.pop();

        int u = topNode.id;
        double d = topNode.priority;

        if (d > dist[u])
            continue;

        if (u == endId)
            break;

        if (u >= static_cast<int>(adj_.size()))
            continue;

        for (const auto& edge : adj_[u]) {
            int v = edge.to;
            double w = calculateWeight(edge, currentHour, avoidSun);

            if (dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                parent[v] = u;
                pq.push(v, dist[v]);
            }
        }
    }

    if (dist[endId] == INF) {
        return {{}, INF};
    }

    // Backtrack path
    std::vector<int> path;
    for (int cur = endId; cur != -1; cur = parent[cur]) {
        path.push_back(cur);
    }
    std::reverse(path.begin(), path.end());

    return {path, dist[endId]};
}

std::vector<double> Graph::dijkstraDistances(int startId, int currentHour, bool avoidSun) const {
    const double INF = std::numeric_limits<double>::infinity();
    int n = static_cast<int>(nodes_.size());

    std::vector<double> dist(n, INF);
    if (startId < 0 || startId >= n || nodes_[startId] == nullptr) {
        return dist;
    }

    dist[startId] = 0.0;
    MinHeap pq;
    pq.push(startId, 0.0);

    while (!pq.empty()) {
        MinHeapNode topNode = pq.top();
        pq.pop();

        int u = topNode.id;
        double d = topNode.priority;

        if (d > dist[u])
            continue;

        if (u >= static_cast<int>(adj_.size()))
            continue;

        for (const auto& edge : adj_[u]) {
            int v = edge.to;
            double w = calculateWeight(edge, currentHour, avoidSun);

            if (dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                pq.push(v, dist[v]);
            }
        }
    }

    return dist;
}

// ============================================================== Squad-Sync ==
MeetupResult Graph::findSquadMeetup(const std::vector<int>& userNodes, int currentHour, bool avoidSun) const {
    const double INF = std::numeric_limits<double>::infinity();
    if (userNodes.empty()) {
        return {-1, 0.0, {}};
    }

    int userCount = static_cast<int>(userNodes.size());
    std::vector<std::vector<double>> userDists(userCount);

    for (int i = 0; i < userCount; ++i) {
        userDists[i] = dijkstraDistances(userNodes[i], currentHour, avoidSun);
    }

    int n = static_cast<int>(nodes_.size());
    int bestNode = -1;
    double bestCollectiveCost = INF;
    std::vector<double> bestUserCosts;

    for (int candidate = 0; candidate < n; ++candidate) {
        if (nodes_[candidate] == nullptr)
            continue;

        double sum = 0.0;
        bool reachable = true;
        std::vector<double> costs(userCount);

        for (int i = 0; i < userCount; ++i) {
            double d = userDists[i][candidate];
            if (d == INF) {
                reachable = false;
                break;
            }
            costs[i] = d;
            sum += d;
        }

        if (reachable && sum < bestCollectiveCost) {
            bestCollectiveCost = sum;
            bestNode = candidate;
            bestUserCosts = costs;
        }
    }

    return {bestNode, bestCollectiveCost, bestUserCosts};
}

// ===================================================== Errand Optimization ==
void Graph::generatePermutations(std::vector<int>& items, int l, int r, std::vector<std::vector<int>>& out) {
    if (l == r) {
        out.push_back(items);
        return;
    }
    for (int i = l; i <= r; ++i) {
        int tmp = items[l];
        items[l] = items[i];
        items[i] = tmp;

        generatePermutations(items, l + 1, r, out);

        tmp = items[l];
        items[l] = items[i];
        items[i] = tmp;
    }
}

ErrandResult Graph::optimizeErrands(int startNode, int endNode, const std::vector<int>& errandNodes, int currentHour, bool avoidSun) const {
    const double INF = std::numeric_limits<double>::infinity();

    if (errandNodes.empty()) {
        PathResult direct = dijkstra(startNode, endNode, currentHour, avoidSun);
        return {{startNode, endNode}, direct.path, direct.totalCost};
    }

    std::vector<int> errands = errandNodes;
    std::vector<std::vector<int>> allPerms;
    generatePermutations(errands, 0, static_cast<int>(errands.size()) - 1, allPerms);

    double bestCost = INF;
    std::vector<int> bestVisitOrder;
    std::vector<int> bestFullPath;

    for (const auto& perm : allPerms) {
        std::vector<int> order;
        order.push_back(startNode);
        for (int e : perm) {
            order.push_back(e);
        }
        order.push_back(endNode);

        double tourCost = 0.0;
        std::vector<int> currentPath;
        bool validTour = true;

        for (size_t i = 0; i + 1 < order.size(); ++i) {
            PathResult segment = dijkstra(order[i], order[i + 1], currentHour, avoidSun);
            if (segment.totalCost == INF) {
                validTour = false;
                break;
            }
            tourCost += segment.totalCost;

            if (currentPath.empty()) {
                currentPath = segment.path;
            } else {
                // Stitch without repeating the boundary node
                for (size_t k = 1; k < segment.path.size(); ++k) {
                    currentPath.push_back(segment.path[k]);
                }
            }
        }

        if (validTour && tourCost < bestCost) {
            bestCost = tourCost;
            bestVisitOrder = order;
            bestFullPath = currentPath;
        }
    }

    return {bestVisitOrder, bestFullPath, bestCost};
}
