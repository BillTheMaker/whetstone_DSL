#pragma once
// Step 585: Workflow Visualization 2.0

#include "ProductizationValidationUtil.h"

#include <algorithm>
#include <map>
#include <string>
#include <vector>

enum class WorkflowNodeKind {
    Intake,
    ReviewGate,
    TaskQueue,
    Execution,
    Verification
};

struct WorkflowNodeVisual {
    std::string nodeId;
    WorkflowNodeKind kind = WorkflowNodeKind::Intake;
    std::string label;
    std::string accentColorHex = "#2F7BFF";  // electric-blue baseline
};

struct WorkflowEdgeVisual {
    std::string edgeId;
    std::string fromNodeId;
    std::string toNodeId;
    std::string routingReason;
};

struct WorkflowVisualizationGraph {
    std::vector<WorkflowNodeVisual> nodes;
    std::vector<WorkflowEdgeVisual> edges;
};

class WorkflowVisualizationV2 {
public:
    static bool addNode(WorkflowVisualizationGraph* graph,
                        const WorkflowNodeVisual& node,
                        std::string* error) {
        if (!graph || !error) return false;
        error->clear();
        if (!hasRequiredProductIds(node.nodeId, node.label)) {
            if (node.nodeId.empty()) return fail(error, "node_id_missing");
            return fail(error, "node_label_missing");
        }
        if (!isValidHexColor7(node.accentColorHex)) return fail(error, "node_color_invalid");
        if (findNode(*graph, node.nodeId) != nullptr) return fail(error, "node_duplicate");
        graph->nodes.push_back(node);
        return true;
    }

    static bool addEdge(WorkflowVisualizationGraph* graph,
                        const WorkflowEdgeVisual& edge,
                        std::string* error) {
        if (!graph || !error) return false;
        error->clear();
        if (edge.edgeId.empty()) return fail(error, "edge_id_missing");
        if (edge.fromNodeId.empty() || edge.toNodeId.empty()) return fail(error, "edge_endpoint_missing");
        if (edge.routingReason.empty()) return fail(error, "edge_reason_missing");
        if (findEdge(*graph, edge.edgeId) != nullptr) return fail(error, "edge_duplicate");
        if (!findNode(*graph, edge.fromNodeId) || !findNode(*graph, edge.toNodeId)) {
            return fail(error, "edge_node_missing");
        }
        graph->edges.push_back(edge);
        return true;
    }

    static std::vector<WorkflowEdgeVisual> outgoing(const WorkflowVisualizationGraph& graph,
                                                    const std::string& nodeId) {
        std::vector<WorkflowEdgeVisual> out;
        for (const auto& edge : graph.edges) if (edge.fromNodeId == nodeId) out.push_back(edge);
        return out;
    }

    static std::vector<std::string> plainLanguageRoutingSummary(
        const WorkflowVisualizationGraph& graph) {
        std::vector<std::string> lines;
        for (const auto& edge : graph.edges) {
            lines.push_back(edge.fromNodeId + " -> " + edge.toNodeId + ": " + edge.routingReason);
        }
        std::stable_sort(lines.begin(), lines.end());
        return lines;
    }

    static std::vector<WorkflowNodeVisual> nodesByKind(const WorkflowVisualizationGraph& graph,
                                                       WorkflowNodeKind kind) {
        std::vector<WorkflowNodeVisual> out;
        for (const auto& node : graph.nodes) if (node.kind == kind) out.push_back(node);
        return out;
    }

private:
    static bool fail(std::string* error, const char* code) {
        *error = code;
        return false;
    }

    static const WorkflowNodeVisual* findNode(const WorkflowVisualizationGraph& graph,
                                              const std::string& nodeId) {
        for (const auto& node : graph.nodes) if (node.nodeId == nodeId) return &node;
        return nullptr;
    }

    static const WorkflowEdgeVisual* findEdge(const WorkflowVisualizationGraph& graph,
                                              const std::string& edgeId) {
        for (const auto& edge : graph.edges) if (edge.edgeId == edgeId) return &edge;
        return nullptr;
    }
};
