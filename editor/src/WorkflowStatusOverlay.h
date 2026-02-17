#pragma once

#include "WorkflowState.h"

#include <algorithm>
#include <map>
#include <string>
#include <vector>

struct WorkflowStatusMarker {
    std::string itemId;
    std::string nodeId;
    std::string displayName;
    std::string bufferId;
    int line = 0;
    std::string status;
    std::string statusIcon;
    std::string statusColor;
    std::string boardColumn;
    std::string detailRoute;
};

class WorkflowStatusOverlay {
public:
    static std::vector<WorkflowStatusMarker> build(const WorkflowState& wf,
                                                   const std::map<std::string, int>& nodeLines,
                                                   const std::string& bufferFilter = "") {
        std::vector<WorkflowStatusMarker> out;
        for (const auto& status : {WI_PENDING, WI_READY, WI_ASSIGNED, WI_IN_PROGRESS,
                                   WI_REVIEW, WI_COMPLETE, WI_REJECTED}) {
            for (const auto& wi : wf.queue.getByStatus(status)) {
                if (!bufferFilter.empty() && wi.bufferId != bufferFilter) continue;
                WorkflowStatusMarker marker;
                marker.itemId = wi.id;
                marker.nodeId = wi.nodeId;
                marker.displayName = wi.nodeName.empty() ? wi.nodeId : wi.nodeName;
                marker.bufferId = wi.bufferId;
                marker.line = lineForNode(wi.nodeId, nodeLines);
                marker.status = wi.status;
                marker.statusIcon = iconForStatus(wi.status);
                marker.statusColor = colorForStatus(wi.status);
                marker.boardColumn = boardColumnForStatus(wi.status);
                marker.detailRoute = detailRoute(wi.id);
                out.push_back(marker);
            }
        }

        std::sort(out.begin(), out.end(), [](const WorkflowStatusMarker& a,
                                             const WorkflowStatusMarker& b) {
            if (a.bufferId != b.bufferId) return a.bufferId < b.bufferId;
            if (a.line != b.line) return a.line < b.line;
            return a.itemId < b.itemId;
        });
        return out;
    }

    static std::string iconForStatus(const std::string& status) {
        if (status == WI_PENDING || status == WI_READY || status == WI_REJECTED) return "outline";
        if (status == WI_ASSIGNED || status == WI_IN_PROGRESS) return "spinner";
        if (status == WI_REVIEW) return "eye";
        if (status == WI_COMPLETE) return "check";
        return "outline";
    }

    static std::string colorForStatus(const std::string& status) {
        if (status == WI_PENDING || status == WI_REJECTED) return "#7F8C8D";
        if (status == WI_READY) return "#3498DB";
        if (status == WI_ASSIGNED || status == WI_IN_PROGRESS) return "#2980B9";
        if (status == WI_REVIEW) return "#F39C12";
        if (status == WI_COMPLETE) return "#27AE60";
        return "#7F8C8D";
    }

    static std::string boardColumnForStatus(const std::string& status) {
        if (status == WI_PENDING || status == WI_REJECTED) return "pending";
        if (status == WI_READY) return "ready";
        if (status == WI_ASSIGNED || status == WI_IN_PROGRESS) return "in-progress";
        if (status == WI_REVIEW) return "review";
        if (status == WI_COMPLETE) return "complete";
        return "pending";
    }

    static std::string detailRoute(const std::string& itemId) {
        return "task://detail/" + itemId;
    }

private:
    static int lineForNode(const std::string& nodeId,
                           const std::map<std::string, int>& nodeLines) {
        auto it = nodeLines.find(nodeId);
        if (it == nodeLines.end()) return 0;
        return it->second;
    }
};
