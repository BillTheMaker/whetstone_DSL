#pragma once
// Step 647: Self-modification safety guard

#include <string>
#include <vector>

class SelfModificationSafetyGuard {
public:
    static bool isLiveBinaryPath(const std::string& filePath,
                                 const std::vector<std::string>& liveRoots) {
        for (const auto& root : liveRoots) {
            if (!root.empty() && filePath.rfind(root, 0) == 0) return true;
        }
        return false;
    }

    static bool allowMutation(const std::string& filePath,
                              const std::vector<std::string>& liveRoots,
                              std::string* reason) {
        if (!reason) return false;
        reason->clear();
        if (isLiveBinaryPath(filePath, liveRoots)) {
            *reason = "Cannot mutate live binary - use a build/restart cycle.";
            return false;
        }
        return true;
    }
};
