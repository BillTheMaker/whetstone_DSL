#pragma once

#include "DependencyGraph.h"

#include <map>
#include <set>
#include <string>
#include <vector>

struct DependencyNodeView {
    std::string id;
    std::string label;
    std::string status;
    std::string workerType;
    std::string priority;
    std::string colorHex;
    bool criticalPath = false;
    std::string hoverText;
    std::string taskId;
};

struct DependencyEdgeView {
    std::string from;
    std::string to;
    bool criticalPath = false;
};

struct DependencyGraphView {
    std::vector<DependencyNodeView> nodes;
    std::vector<DependencyEdgeView> edges;
    std::vector<std::string> criticalPath;

    std::optional<DependencyNodeView> getNode(const std::string& id) const {
        for (const auto& n : nodes) if (n.id == id) return n;
        return std::nullopt;
    }
};

class WorkflowDependencyView {
public:
    static DependencyGraphView build(const WorkflowState& wf) {
        auto graph = buildDependencyGraph(wf);
        auto critical = getCriticalPath(graph);
        std::set<std::string> criticalSet(critical.begin(), critical.end());
        std::set<std::string> criticalEdges;
        for (size_t i = 1; i < critical.size(); ++i) {
            criticalEdges.insert(critical[i - 1] + "->" + critical[i]);
        }

        DependencyGraphView out;
        out.criticalPath = critical;

        for (const auto& n : graph.nodes) {
            DependencyNodeView v;
            v.id = n.id;
            v.label = n.label;
            v.status = n.status;
            v.workerType = n.workerType;
            v.priority = n.priority;
            v.colorHex = colorForStatus(n.status);
            v.criticalPath = criticalSet.count(n.id) > 0;
            v.taskId = n.id;
            v.hoverText = "Task: " + n.label + "\nStatus: " + n.status +
                          "\nWorker: " + n.workerType +
                          "\nPriority: " + n.priority;
            out.nodes.push_back(v);
        }

        for (const auto& e : graph.edges) {
            DependencyEdgeView ev;
            ev.from = e.from;
            ev.to = e.to;
            ev.criticalPath = criticalEdges.count(e.from + "->" + e.to) > 0;
            out.edges.push_back(ev);
        }

        return out;
    }

    static std::string colorForStatus(const std::string& status) {
        if (status == WI_COMPLETE) return "#2ECC71";      // green
        if (status == WI_IN_PROGRESS || status == WI_ASSIGNED) return "#3498DB"; // blue
        if (status == WI_PENDING || status == WI_READY) return "#95A5A6"; // gray
        if (status == WI_REVIEW || status == WI_REJECTED) return "#E74C3C"; // red
        return "#7F8C8D";
    }

    static std::string navigateToTaskId(const DependencyGraphView& view,
                                        const std::string& nodeId) {
        auto n = view.getNode(nodeId);
        if (!n.has_value()) return "";
        return n->taskId;
    }
};
