#pragma once
// Step 1459: deterministic failure fixture catalog.

#include <algorithm>
#include <map>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

struct FailureFixture {
    std::string id;
    std::string failureClass;
    std::string command;
    std::string raw;
    int exitCode = 1;
    std::string expectedPrimaryFile;
};

class FailureFixtureCatalog {
public:
    static std::vector<FailureFixture> defaults() {
        std::vector<FailureFixture> v = {
            {"compile_missing_include", "compile_error", "mock:error: editor/src/a.cpp:10:1 missing include", "error: editor/src/a.cpp:10:1 missing include", 1, "editor/src/a.cpp"},
            {"test_assertion_fail", "test_assertion", "mock:alpha... PASS\nbeta... FAIL", "alpha... PASS\nbeta... FAIL", 1, ""},
            {"tool_contract_missing", "tool_contract_error", "mock:Unknown tool: whetstone_x", "Unknown tool: whetstone_x", 1, ""},
            {"schema_shape_mismatch", "schema_error", "mock:schema mismatch in packet", "schema mismatch in packet", 1, ""},
            {"runtime_nonzero", "runtime_error", "mock:segmentation fault", "segmentation fault", 1, ""}
        };
        std::sort(v.begin(), v.end(), [](const auto& a, const auto& b){ return a.id < b.id; });
        return v;
    }

    static const FailureFixture* find(const std::vector<FailureFixture>& fixtures,
                                      const std::string& id) {
        for (const auto& f : fixtures) if (f.id == id) return &f;
        return nullptr;
    }

    static nlohmann::json toJson(const std::vector<FailureFixture>& fixtures) {
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& f : fixtures) {
            arr.push_back({
                {"id", f.id},
                {"failure_class", f.failureClass},
                {"command", f.command},
                {"raw", f.raw},
                {"exit_code", f.exitCode},
                {"expected_primary_file", f.expectedPrimaryFile}
            });
        }
        return arr;
    }
};
