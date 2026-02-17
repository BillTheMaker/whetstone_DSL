#pragma once
// Step 572: Performance Probe Overlay

#include "DebugValidationUtil.h"

#include <algorithm>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

struct ProbeSample {
    std::string sampleId;
    std::string sessionId;
    std::string bufferId;
    int line = 0;
    std::uint64_t durationMicros = 0;
    std::uint64_t timestamp = 0;
};

struct ProbeAggregate {
    int line = 0;
    std::uint64_t totalDurationMicros = 0;
    std::uint64_t sampleCount = 0;
};

class PerformanceProbeOverlay {
public:
    bool addSample(const ProbeSample& sample, std::string* error) {
        if (!error) return false;
        error->clear();
        if (!hasRequiredDebugIds(sample.sampleId, sample.sessionId)) return fail(error, "sample_or_session_missing");
        if (sample.bufferId.empty()) return fail(error, "sample_buffer_missing");
        if (!isPositiveLine(sample.line)) return fail(error, "sample_line_invalid");
        if (!isNonZeroU64(sample.durationMicros)) return fail(error, "sample_duration_invalid");
        if (!isNonZeroU64(sample.timestamp)) return fail(error, "sample_timestamp_invalid");
        if (containsSample(sample.sampleId)) return fail(error, "sample_duplicate");

        samples_.push_back(sample);
        return true;
    }

    std::vector<ProbeAggregate> aggregatesFor(const std::string& sessionId,
                                              const std::string& bufferId) const {
        std::map<int, ProbeAggregate> grouped;
        for (const auto& sample : samples_) {
            if (!sessionId.empty() && sample.sessionId != sessionId) continue;
            if (!bufferId.empty() && sample.bufferId != bufferId) continue;
            auto& entry = grouped[sample.line];
            entry.line = sample.line;
            entry.totalDurationMicros += sample.durationMicros;
            ++entry.sampleCount;
        }

        std::vector<ProbeAggregate> out;
        for (const auto& kv : grouped) out.push_back(kv.second);
        std::stable_sort(out.begin(), out.end(), [](const ProbeAggregate& a, const ProbeAggregate& b) {
            return a.line < b.line;
        });
        return out;
    }

    std::vector<ProbeAggregate> topHotspots(const std::string& sessionId,
                                            const std::string& bufferId,
                                            std::size_t limit,
                                            std::uint64_t minTotalDurationMicros) const {
        auto aggregates = aggregatesFor(sessionId, bufferId);
        std::vector<ProbeAggregate> filtered;
        for (const auto& aggregate : aggregates) {
            if (aggregate.totalDurationMicros >= minTotalDurationMicros) filtered.push_back(aggregate);
        }

        std::stable_sort(filtered.begin(), filtered.end(), [](const ProbeAggregate& a, const ProbeAggregate& b) {
            if (a.totalDurationMicros != b.totalDurationMicros) return a.totalDurationMicros > b.totalDurationMicros;
            return a.line < b.line;
        });
        if (filtered.size() > limit) filtered.resize(limit);
        return filtered;
    }

    std::vector<ProbeAggregate> overlayWindow(const std::string& sessionId,
                                              const std::string& bufferId,
                                              int pauseLine,
                                              int radius) const {
        std::vector<ProbeAggregate> window;
        if (!isPositiveLine(pauseLine) || radius < 0) return window;

        const int minLine = pauseLine - radius;
        const int maxLine = pauseLine + radius;
        for (const auto& aggregate : aggregatesFor(sessionId, bufferId)) {
            if (aggregate.line >= minLine && aggregate.line <= maxLine) window.push_back(aggregate);
        }
        return window;
    }

private:
    std::vector<ProbeSample> samples_;

    static bool fail(std::string* error, const char* code) {
        *error = code;
        return false;
    }

    bool containsSample(const std::string& sampleId) const {
        for (const auto& sample : samples_) {
            if (sample.sampleId == sampleId) return true;
        }
        return false;
    }
};
