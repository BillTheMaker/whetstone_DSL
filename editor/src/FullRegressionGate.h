#pragma once

// Step 507: Full Test Suite Regression Gate
// Evaluates multi-run regression summaries for pass/fail, flake, performance, and leaks.

#include <string>
#include <vector>

struct RegressionRunSample {
    bool passed = false;
    double durationSeconds = 0.0;
    int memoryLeakCount = 0;
    int testsExecuted = 0;
};

struct RegressionSuiteExecution {
    std::string suiteName;
    std::vector<RegressionRunSample> runs; // typically 3 runs for flake detection
};

struct FullRegressionReport {
    int suites = 0;
    int totalTestsExecuted = 0;
    int totalFailures = 0;
    int totalLeaks = 0;
    double totalDurationSeconds = 0.0;
    bool allPassing = false;
    bool noFlakes = false;
    bool performanceTargetMet = false; // < 60s
    bool memoryStable = false;         // 0 leaks
    bool gatePass = false;
    std::vector<std::string> flakySuites;
    std::vector<std::string> failedSuites;
    std::vector<std::string> notes;
};

class FullRegressionGate {
public:
    static FullRegressionReport evaluate(const std::vector<RegressionSuiteExecution>& suites,
                                         double durationBudgetSeconds = 60.0) {
        FullRegressionReport out;
        out.suites = static_cast<int>(suites.size());
        out.allPassing = true;
        out.noFlakes = true;

        for (const auto& s : suites) {
            bool anyPass = false;
            bool anyFail = false;
            for (const auto& run : s.runs) {
                out.totalTestsExecuted += run.testsExecuted;
                out.totalDurationSeconds += run.durationSeconds;
                out.totalLeaks += run.memoryLeakCount;
                if (run.passed) anyPass = true;
                else {
                    anyFail = true;
                    out.totalFailures++;
                }
            }

            if (anyFail) {
                out.allPassing = false;
                out.failedSuites.push_back(s.suiteName);
            }
            if (anyPass && anyFail) {
                out.noFlakes = false;
                out.flakySuites.push_back(s.suiteName);
            }
        }

        out.performanceTargetMet = out.totalDurationSeconds < durationBudgetSeconds;
        out.memoryStable = out.totalLeaks == 0;
        out.gatePass = out.allPassing && out.noFlakes &&
                       out.performanceTargetMet && out.memoryStable;

        out.notes.push_back(out.gatePass ? "Full regression gate passed" : "Full regression gate failed");
        out.notes.push_back("Total tests executed: " + std::to_string(out.totalTestsExecuted));
        return out;
    }

    static std::vector<RegressionSuiteExecution> samplePhase25Dataset() {
        // Synthetic 245-506 summary with >5000 tests, 3 stable runs each.
        return {
            {"steps_245_320", stableSuite(1200, 4.4)},
            {"steps_321_400", stableSuite(1300, 4.6)},
            {"steps_401_470", stableSuite(1250, 4.5)},
            {"steps_471_506", stableSuite(1400, 4.7)}
        };
    }

private:
    static std::vector<RegressionRunSample> stableSuite(int testsPerRun, double durationPerRun) {
        return {
            {true, durationPerRun, 0, testsPerRun},
            {true, durationPerRun, 0, testsPerRun},
            {true, durationPerRun, 0, testsPerRun}
        };
    }
};
