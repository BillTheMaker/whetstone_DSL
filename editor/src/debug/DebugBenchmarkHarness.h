#pragma once
// Step 1475: debug benchmark harness.

#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "FailureFixtureCatalog.h"
#include "FailurePacket.h"

struct DebugBenchmarkResult {
    int fixtureCount = 0;
    int packetizedCount = 0;
    bool success = false;
};

class DebugBenchmarkHarness {
public:
    static DebugBenchmarkResult run(const std::vector<FailureFixture>& fixtures) {
        DebugBenchmarkResult r;
        r.fixtureCount = static_cast<int>(fixtures.size());
        for (const auto& f : fixtures) {
            auto p = FailurePacketModel::make(f.command, f.raw, f.exitCode);
            if (!p.packetId.empty()) ++r.packetizedCount;
        }
        r.success = r.packetizedCount == r.fixtureCount;
        return r;
    }

    static nlohmann::json toJson(const DebugBenchmarkResult& r) {
        return {{"fixture_count", r.fixtureCount}, {"packetized_count", r.packetizedCount}, {"success", r.success}};
    }
};
