#pragma once
// Step 562: Workflow-Aware Debug Hooks

#include <map>
#include <string>
#include <vector>

struct WorkflowDebugBinding {
    std::string workflowItemId;
    std::string debugSessionId;
    std::vector<std::string> diagnosticIds;
    bool active = true;
};

class WorkflowAwareDebugHooks {
public:
    bool attach(const std::string& workflowItemId,
                const std::string& debugSessionId,
                const std::vector<std::string>& diagnosticIds) {
        if (workflowItemId.empty() || debugSessionId.empty()) return false;
        if (bindings_.count(workflowItemId) != 0) return false;
        bindings_[workflowItemId] = {workflowItemId, debugSessionId, diagnosticIds, true};
        return true;
    }

    bool detach(const std::string& workflowItemId) {
        auto it = bindings_.find(workflowItemId);
        if (it == bindings_.end()) return false;
        bindings_.erase(it);
        return true;
    }

    bool setActive(const std::string& workflowItemId, bool active) {
        auto* b = find(workflowItemId);
        if (!b) return false;
        b->active = active;
        return true;
    }

    bool addDiagnostic(const std::string& workflowItemId,
                       const std::string& diagnosticId) {
        auto* b = find(workflowItemId);
        if (!b || diagnosticId.empty()) return false;
        for (const auto& d : b->diagnosticIds) {
            if (d == diagnosticId) return false;
        }
        b->diagnosticIds.push_back(diagnosticId);
        return true;
    }

    std::vector<WorkflowDebugBinding> activeBindings() const {
        std::vector<WorkflowDebugBinding> out;
        for (const auto& kv : bindings_) {
            if (kv.second.active) out.push_back(kv.second);
        }
        return out;
    }

    bool isAttached(const std::string& workflowItemId) const {
        return bindings_.count(workflowItemId) != 0;
    }

    std::vector<std::string> diagnosticsFor(const std::string& workflowItemId) const {
        auto it = bindings_.find(workflowItemId);
        if (it == bindings_.end()) return {};
        return it->second.diagnosticIds;
    }

private:
    std::map<std::string, WorkflowDebugBinding> bindings_;

    WorkflowDebugBinding* find(const std::string& workflowItemId) {
        auto it = bindings_.find(workflowItemId);
        if (it == bindings_.end()) return nullptr;
        return &it->second;
    }
};
