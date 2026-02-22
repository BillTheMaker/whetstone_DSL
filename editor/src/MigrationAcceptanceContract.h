#pragma once
// Step 692: Migration acceptance contract checks.

#include <string>
#include <vector>

#include <nlohmann/json.hpp>

struct MigrationGateThresholds {
    float minimumTestPassRate = 1.0f;
    int maxHighSeverityFindings = 0;
    float maxPerfRegressionPct = 10.0f; // allowed slowdown percent
    bool requireChecklistComplete = true;
};

struct MigrationEvidence {
    bool buildSuccess = false;
    bool testsProvided = false;
    float testPassRate = 0.0f;
    bool securityReportProvided = false;
    int highSeverityFindings = 0;
    bool benchmarkProvided = false;
    float perfRegressionPct = 0.0f;
    bool checklistComplete = false;
};

struct MigrationGateResult {
    bool pass = false;
    std::vector<std::string> failedGates;
    std::vector<std::string> warningGates;
    nlohmann::json gateStates = nlohmann::json::object();
    std::string summary;
};

struct PortingGateEvidence {
    int securityHighFindings = 0;
    bool sanitizerPass = false;
    int supplyChainHighFindings = 0;
    double worstPerfRegressionPct = 0.0;
    bool hasSecurityWaiver = false;
    bool hasPerfWaiver = false;
};

class MigrationAcceptanceContract {
public:
    static MigrationGateResult evaluate(const MigrationEvidence& evidence,
                                        const MigrationGateThresholds& thresholds) {
        MigrationGateResult r;

        auto fail = [&](const std::string& gate, const std::string& state) {
            r.failedGates.push_back(gate);
            r.gateStates[gate] = state;
        };
        auto warn = [&](const std::string& gate, const std::string& state) {
            r.warningGates.push_back(gate);
            r.gateStates[gate] = state;
        };

        if (!evidence.buildSuccess) fail("build_success", "failed");
        else r.gateStates["build_success"] = "passed";

        if (!evidence.testsProvided) fail("tests_provided", "missing");
        else if (evidence.testPassRate < thresholds.minimumTestPassRate) fail("test_pass_rate", "below_threshold");
        else r.gateStates["test_pass_rate"] = "passed";

        if (!evidence.securityReportProvided) fail("security_report", "missing");
        else if (evidence.highSeverityFindings > thresholds.maxHighSeverityFindings) fail("high_severity_findings", "above_threshold");
        else r.gateStates["high_severity_findings"] = "passed";

        if (!evidence.benchmarkProvided) fail("benchmark_data", "missing");
        else if (evidence.perfRegressionPct > thresholds.maxPerfRegressionPct) fail("performance_regression", "above_threshold");
        else r.gateStates["performance_regression"] = "passed";

        if (thresholds.requireChecklistComplete && !evidence.checklistComplete) fail("checklist_complete", "incomplete");
        else r.gateStates["checklist_complete"] = "passed";

        r.pass = r.failedGates.empty();
        if (r.pass) r.summary = "all_gates_passed";
        else r.summary = "failed_gates:" + std::to_string((int)r.failedGates.size());
        return r;
    }

    static MigrationGateResult evaluatePortingGates(const PortingGateEvidence& gates,
                                                    const MigrationGateThresholds& thresholds) {
        MigrationGateResult r;

        auto fail = [&](const std::string& gate, const std::string& state) {
            r.failedGates.push_back(gate);
            r.gateStates[gate] = state;
        };
        auto warn = [&](const std::string& gate, const std::string& state) {
            r.warningGates.push_back(gate);
            r.gateStates[gate] = state;
        };

        if (gates.securityHighFindings > thresholds.maxHighSeverityFindings) {
            if (gates.hasSecurityWaiver) warn("security", "waived");
            else fail("security", "blocked_high_findings");
        } else {
            r.gateStates["security"] = "passed";
        }

        if (!gates.sanitizerPass) fail("sanitizer", "failed");
        else r.gateStates["sanitizer"] = "passed";

        if (gates.supplyChainHighFindings > thresholds.maxHighSeverityFindings) {
            warn("supply_chain", "high_findings");
        } else {
            r.gateStates["supply_chain"] = "passed";
        }

        if (gates.worstPerfRegressionPct > thresholds.maxPerfRegressionPct) {
            if (gates.hasPerfWaiver) warn("performance", "waived");
            else fail("performance", "blocked_regression");
        } else {
            r.gateStates["performance"] = "passed";
        }

        r.pass = r.failedGates.empty();
        r.summary = r.pass ? "all_porting_gates_passed"
                           : "failed_porting_gates:" + std::to_string((int)r.failedGates.size());
        return r;
    }
};
