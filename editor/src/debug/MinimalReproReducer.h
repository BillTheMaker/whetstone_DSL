#pragma once
// Step 1511: minimal repro reducer model.

#include <string>
#include <vector>

#include <nlohmann/json.hpp>

struct MinimalReproResult {
    std::string originalCommand;
    std::string reducedCommand;
    int removedFlags = 0;
};

class MinimalReproReducer {
public:
    static MinimalReproResult reduce(const std::string& command,
                                     const std::vector<std::string>& removableFlags) {
        MinimalReproResult r;
        r.originalCommand = command;
        r.reducedCommand = command;
        for (const auto& f : removableFlags) {
            std::string needle = " " + f;
            auto pos = r.reducedCommand.find(needle);
            if (pos != std::string::npos) {
                r.reducedCommand.erase(pos, needle.size());
                ++r.removedFlags;
            }
        }
        return r;
    }

    static nlohmann::json toJson(const MinimalReproResult& r) {
        return {{"original_command", r.originalCommand}, {"reduced_command", r.reducedCommand}, {"removed_flags", r.removedFlags}};
    }
};
