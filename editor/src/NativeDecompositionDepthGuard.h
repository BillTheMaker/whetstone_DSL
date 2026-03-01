#pragma once
// Step 1881b: Native decomposition depth guard (GR-020).
// Closes GR-020: native decomposition task depth was stuck at 2 and retry
// did not improve it. This guard enforces a minimum task count per decomposition
// attempt and rejects shallow decompositions before they can mask weak generation.

#include <string>
#include <vector>

struct DecompositionGuardResult {
    bool accepted = false;
    std::string reason;
    int taskCount = 0;
    int minRequired = 0;
};

class NativeDecompositionDepthGuard {
public:
    static constexpr int kMinTaskCount = 3;  // minimum tasks for a valid decomposition

    static DecompositionGuardResult evaluate(const std::vector<std::string>& tasks,
                                             int minRequired = kMinTaskCount) {
        DecompositionGuardResult result;
        result.taskCount  = static_cast<int>(tasks.size());
        result.minRequired = minRequired;

        if (result.taskCount < minRequired) {
            result.accepted = false;
            result.reason = "decomposition_too_shallow";
            return result;
        }

        result.accepted = true;
        result.reason = "depth_sufficient";
        return result;
    }
};
