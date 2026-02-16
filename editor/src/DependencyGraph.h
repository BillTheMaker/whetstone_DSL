#pragma once
// Step 392: DependencyGraph — Export workflow dependency graph for visualization
//
// Builds a node/edge graph from WorkflowState. Computes critical path
// (longest dependency chain). Data format ready for Sprint 19's GUI.

#include "WorkflowState.h"
#include "RoutingEngine.h"
#include "CostEstimator.h"
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>

// --- GraphNode ---

struct GraphNode {
    std::string id;
    std::string label;
    std::string type;        // "Function" | "Class" | "Method"
    std::string status;      // WorkItem status
    std::string workerType;  // routing decision
    std::string priority;

    json toJson() const {
        return json{{"id", id}, {"label", label}, {"type", type},
                    {"status", status}, {"workerType", workerType}, {"priority", priority}};
    }

    static GraphNode fromJson(const json& j) {
        GraphNode n;
        if (j.contains("id")) n.id = j["id"].get<std::string>();
        if (j.contains("label")) n.label = j["label"].get<std::string>();
        if (j.contains("type")) n.type = j["type"].get<std::string>();
        if (j.contains("status")) n.status = j["status"].get<std::string>();
        if (j.contains("workerType")) n.workerType = j["workerType"].get<std::string>();
        if (j.contains("priority")) n.priority = j["priority"].get<std::string>();
        return n;
    }
};

// --- GraphEdge ---

struct GraphEdge {
    std::string from;
    std::string to;
    std::string type;  // "depends-on" | "blocks"

    json toJson() const {
        return json{{"from", from}, {"to", to}, {"type", type}};
    }

    static GraphEdge fromJson(const json& j) {
        GraphEdge e;
        if (j.contains("from")) e.from = j["from"].get<std::string>();
        if (j.contains("to")) e.to = j["to"].get<std::string>();
        if (j.contains("type")) e.type = j["type"].get<std::string>();
        return e;
    }
};

// --- GraphData ---

struct GraphData {
    std::vector<GraphNode> nodes;
    std::vector<GraphEdge> edges;

    json toJson() const {
        json nodeArr = json::array();
        for (const auto& n : nodes) nodeArr.push_back(n.toJson());
        json edgeArr = json::array();
        for (const auto& e : edges) edgeArr.push_back(e.toJson());
        return json{{"nodes", nodeArr}, {"edges", edgeArr}};
    }

    static GraphData fromJson(const json& j) {
        GraphData g;
        if (j.contains("nodes") && j["nodes"].is_array()) {
            for (const auto& n : j["nodes"]) g.nodes.push_back(GraphNode::fromJson(n));
        }
        if (j.contains("edges") && j["edges"].is_array()) {
            for (const auto& e : j["edges"]) g.edges.push_back(GraphEdge::fromJson(e));
        }
        return g;
    }
};

// --- DependencyGraph builder ---

inline GraphData buildDependencyGraph(const WorkflowState& workflow) {
    GraphData graph;
    auto allItems = collectAllItems(workflow.queue);

    for (const auto& item : allItems) {
        GraphNode node;
        node.id = item.id;
        node.label = item.nodeName;
        node.type = item.nodeType;
        node.status = item.status;
        node.workerType = item.workerType;
        node.priority = item.priority;
        graph.nodes.push_back(node);

        for (const auto& depId : item.dependencies) {
            GraphEdge edge;
            edge.from = depId;
            edge.to = item.id;
            edge.type = "depends-on";
            graph.edges.push_back(edge);
        }
    }
    return graph;
}

inline json graphToJson(const GraphData& graph) {
    return graph.toJson();
}

// --- Critical path computation ---
// Returns the node IDs in the longest dependency chain.

inline std::vector<std::string> getCriticalPath(const GraphData& graph) {
    // Build adjacency: from -> [to]
    std::map<std::string, std::vector<std::string>> adj;
    std::set<std::string> allIds;
    for (const auto& node : graph.nodes) allIds.insert(node.id);
    for (const auto& edge : graph.edges) {
        if (edge.type == "depends-on") {
            adj[edge.from].push_back(edge.to);
        }
    }

    // Find roots (nodes with no incoming "depends-on" edges)
    std::set<std::string> hasIncoming;
    for (const auto& edge : graph.edges) {
        if (edge.type == "depends-on") hasIncoming.insert(edge.to);
    }

    // DFS to find longest path from each root
    std::map<std::string, int> memo;
    std::map<std::string, std::string> parent;

    std::function<int(const std::string&)> longestFrom = [&](const std::string& id) -> int {
        if (memo.count(id)) return memo[id];
        int best = 0;
        std::string bestChild;
        for (const auto& next : adj[id]) {
            int childLen = longestFrom(next);
            if (childLen + 1 > best) {
                best = childLen + 1;
                bestChild = next;
            }
        }
        memo[id] = best;
        if (!bestChild.empty()) parent[id] = bestChild;
        return best;
    };

    // Find the longest path across all roots
    std::string bestStart;
    int bestLen = 0;
    for (const auto& id : allIds) {
        if (hasIncoming.find(id) == hasIncoming.end()) {
            int len = longestFrom(id);
            if (len >= bestLen) {
                bestLen = len;
                bestStart = id;
            }
        }
    }

    // If no roots found (isolated nodes), just pick any node
    if (bestStart.empty() && !allIds.empty()) {
        bestStart = *allIds.begin();
    }

    // Reconstruct path
    std::vector<std::string> path;
    if (!bestStart.empty()) {
        std::string cur = bestStart;
        path.push_back(cur);
        while (parent.count(cur)) {
            cur = parent[cur];
            path.push_back(cur);
        }
    }

    return path;
}
