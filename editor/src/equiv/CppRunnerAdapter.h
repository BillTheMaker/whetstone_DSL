#pragma once
// Step 722: C++ runner adapter for harness.

#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "TestVectorSpec.h"
#include "RustRunnerAdapter.h"

class CppRunnerAdapter {
public:
    static RunnerExecution run(const std::vector<TestVector>& vectors) {
        RunnerExecution out;
        out.runner = "cpp";
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
