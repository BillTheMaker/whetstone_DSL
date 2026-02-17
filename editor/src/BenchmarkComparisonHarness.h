#pragma once
// Step 591: Benchmark and Comparison Harness

#include <algorithm>
#include <map>
#include <string>
#include <vector>

struct BenchmarkSample {
    std::string runId;
    std::string scenario;
    int throughput = 0;
    int reliability = 0;
    int tokenEfficiency = 0;
};

struct BenchmarkAggregate {
    std::string scenario;
    int avgThroughput = 0;
    int avgReliability = 0;
    int avgTokenEfficiency = 0;
    int sampleCount = 0;
};

struct BenchmarkComparison {
    std::string scenario;
    int throughputDelta = 0;
    int reliabilityDelta = 0;
    int tokenEfficiencyDelta = 0;
};

class BenchmarkComparisonHarness {
public:
    bool addSample(const BenchmarkSample& sample, std::string* error) {
        if (!error) return false;
        error->clear();
        if (sample.runId.empty()) return fail(error, "run_id_missing");
        if (sample.scenario.empty()) return fail(error, "scenario_missing");
        if (sample.throughput < 0 || sample.reliability < 0 || sample.tokenEfficiency < 0) {
            return fail(error, "metric_invalid");
        }
        if (seenRunIds_.count(sample.runId) != 0) return fail(error, "run_duplicate");
        seenRunIds_[sample.runId] = true;
        samples_.push_back(sample);
        return true;
    }

    std::vector<BenchmarkAggregate> aggregates() const {
        struct Sum { int t = 0; int r = 0; int e = 0; int n = 0; };
        std::map<std::string, Sum> sums;
        for (const auto& s : samples_) {
            auto& x = sums[s.scenario];
            x.t += s.throughput;
            x.r += s.reliability;
            x.e += s.tokenEfficiency;
            ++x.n;
        }
        std::vector<BenchmarkAggregate> out;
        for (const auto& kv : sums) {
            BenchmarkAggregate a;
            a.scenario = kv.first;
            a.sampleCount = kv.second.n;
            a.avgThroughput = kv.second.t / kv.second.n;
            a.avgReliability = kv.second.r / kv.second.n;
            a.avgTokenEfficiency = kv.second.e / kv.second.n;
            out.push_back(a);
        }
        return out;
    }

    std::vector<BenchmarkComparison> compareRuns(const std::string& baselineRunId,
                                                 const std::string& candidateRunId) const {
        const auto* a = findRun(baselineRunId);
        const auto* b = findRun(candidateRunId);
        if (!a || !b || a->scenario != b->scenario) return {};
        BenchmarkComparison c;
        c.scenario = a->scenario;
        c.throughputDelta = b->throughput - a->throughput;
        c.reliabilityDelta = b->reliability - a->reliability;
        c.tokenEfficiencyDelta = b->tokenEfficiency - a->tokenEfficiency;
        return {c};
    }

    std::vector<BenchmarkSample> topByThroughput(const std::string& scenario,
                                                 std::size_t limit) const {
        std::vector<BenchmarkSample> out;
        for (const auto& s : samples_) if (scenario.empty() || s.scenario == scenario) out.push_back(s);
        std::stable_sort(out.begin(), out.end(), [](const BenchmarkSample& a, const BenchmarkSample& b) {
            if (a.throughput != b.throughput) return a.throughput > b.throughput;
            return a.runId < b.runId;
        });
        if (out.size() > limit) out.resize(limit);
        return out;
    }

private:
    std::vector<BenchmarkSample> samples_;
    std::map<std::string, bool> seenRunIds_;

    static bool fail(std::string* error, const char* code) {
        *error = code;
        return false;
    }

    const BenchmarkSample* findRun(const std::string& runId) const {
        for (const auto& s : samples_) if (s.runId == runId) return &s;
        return nullptr;
    }
};
