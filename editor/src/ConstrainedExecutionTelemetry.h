#pragma once
// Step 537: Constrained Execution Telemetry

#include <map>
#include <numeric>
#include <string>
#include <vector>

struct TelemetryEvent {
    std::string taskitemId;
    int candidateOperationCount = 0;
    int candidateSymbolCount = 0;
    std::string selectedOperation;
    std::vector<std::string> rejectionReasons;
    int constrainedTokenCount = 0;
    int baselineTokenCount = 0;
    bool success = false;
};

struct TelemetrySummary {
    int events = 0;
    int successCount = 0;
    int failureCount = 0;
    int avgCandidateOperationCount = 0;
    int avgCandidateSymbolCount = 0;
    int constrainedTokens = 0;
    int baselineTokens = 0;
    int tokenSavings = 0;
    double tokenSavingsRatio = 0.0;
    std::map<std::string, int> rejectionHistogram;
    std::map<std::string, int> opSelectionHistogram;
};

class ConstrainedExecutionTelemetry {
public:
    void record(const TelemetryEvent& event) {
        events_.push_back(event);
    }

    size_t size() const { return events_.size(); }

    TelemetrySummary summarize() const {
        TelemetrySummary s;
        s.events = (int)events_.size();
        if (events_.empty()) return s;

        int opTotal = 0;
        int symTotal = 0;
        for (const auto& event : events_) {
            opTotal += event.candidateOperationCount;
            symTotal += event.candidateSymbolCount;
            s.constrainedTokens += event.constrainedTokenCount;
            s.baselineTokens += event.baselineTokenCount;
            if (event.success) ++s.successCount; else ++s.failureCount;
            if (!event.selectedOperation.empty()) {
                ++s.opSelectionHistogram[event.selectedOperation];
            }
            for (const auto& reason : event.rejectionReasons) {
                ++s.rejectionHistogram[reason];
            }
        }

        s.avgCandidateOperationCount = opTotal / s.events;
        s.avgCandidateSymbolCount = symTotal / s.events;
        s.tokenSavings = s.baselineTokens - s.constrainedTokens;
        if (s.baselineTokens > 0) {
            s.tokenSavingsRatio = static_cast<double>(s.tokenSavings) /
                                  static_cast<double>(s.baselineTokens);
        }
        return s;
    }

private:
    std::vector<TelemetryEvent> events_;
};
