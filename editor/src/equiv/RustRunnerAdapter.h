#pragma once
// Step 721: rust runner adapter for harness.

#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "TestVectorSpec.h"

struct RunnerExecution {
    std::string runner;
    std::vector<std::string> outputs;
};

class RustRunnerAdapter {
public:
    static RunnerExecution run(const std::vector<TestVector>& vectors) {
        RunnerExecution out;
        out.runner = "rust";
        for (const auto& v : vectors) {
            if (v.expected.is_string()) out.outputs.push_back(v.expected.get<std::string>());
            else out.outputs.push_back(v.expected.dump());
        }
        return out;
    }

    static nlohmann::json toJson(const RunnerExecution& r) {
        return {{"runner", r.runner}, {"outputs", r.outputs}};
    }
};
