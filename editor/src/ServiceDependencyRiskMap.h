#pragma once
// Step 610: Service Dependency Risk Map

#include <map>
#include <string>
#include <vector>

#include "ValidationErrorUtil.h"

enum class DependencyRisk {
    Low,
    Medium,
    High
};

struct DependencyEdge {
    std::string edgeId;
    std::string sourceService;
    std::string targetService;
    DependencyRisk risk = DependencyRisk::Low;
};

class ServiceDependencyRiskMap {
public:
    bool addEdge(const DependencyEdge& edge, std::string* error) {
        if (!error) return false;
        error->clear();
        if (edge.edgeId.empty()) return failWith(error, "edge_id_missing");
        if (edge.sourceService.empty()) return failWith(error, "source_service_missing");
        if (edge.targetService.empty()) return failWith(error, "target_service_missing");
        if (edges_.count(edge.edgeId) != 0) return failWith(error, "edge_duplicate");
        edges_[edge.edgeId] = edge;
        order_.push_back(edge.edgeId);
        return true;
    }

    bool updateRisk(const std::string& edgeId, DependencyRisk risk, std::string* error) {
        if (!error) return false;
        error->clear();
        auto it = edges_.find(edgeId);
        if (it == edges_.end()) return failWith(error, "edge_missing");
        it->second.risk = risk;
        return true;
    }

    int highRiskInboundCount(const std::string& targetService) const {
        int count = 0;
        for (const auto& id : order_) {
            const auto& edge = edges_.at(id);
            if (edge.targetService == targetService && edge.risk == DependencyRisk::High) ++count;
        }
        return count;
    }

    int highRiskOutboundCount(const std::string& sourceService) const {
        int count = 0;
        for (const auto& id : order_) {
            const auto& edge = edges_.at(id);
            if (edge.sourceService == sourceService && edge.risk == DependencyRisk::High) ++count;
        }
        return count;
    }

    std::vector<DependencyEdge> byService(const std::string& serviceId) const {
        std::vector<DependencyEdge> out;
        for (const auto& id : order_) {
            const auto& edge = edges_.at(id);
            if (serviceId.empty() || edge.sourceService == serviceId || edge.targetService == serviceId) {
                out.push_back(edge);
            }
        }
        return out;
    }

private:
    std::map<std::string, DependencyEdge> edges_;
    std::vector<std::string> order_;
};
