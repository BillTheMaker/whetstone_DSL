#pragma once
// Step 233: Training data pipeline helpers

#include <string>
#include <vector>
#include <unordered_set>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <nlohmann/json.hpp>
#include "SessionAnonymizer.h"
#include "SessionToTrace.h"
#include "TraceExporter.h"

using json = nlohmann::json;

struct PipelineStats {
    int sessionsProcessed = 0;
    int tracesGenerated = 0;
    int tracesFiltered = 0;
    int tracesDeduped = 0;
    int tracesValidated = 0;
};

struct PipelineOptions {
    SessionAnonymizer::Level anonymizeLevel = SessionAnonymizer::Level::Medium;
    bool anonymizeEnabled = true;
    bool deduplicate = true;
    std::string format = "jsonl"; // jsonl | anthropic | openai | markdown
};

static std::vector<json> loadSessionsFromDir(const std::string& dir) {
    std::vector<json> sessions;
    std::error_code ec;
    if (!std::filesystem::exists(dir, ec)) return sessions;
    for (const auto& entry : std::filesystem::directory_iterator(dir, ec)) {
        if (ec) break;
        if (!entry.is_regular_file()) continue;
        auto path = entry.path();
        if (path.extension() != ".json") continue;
        std::ifstream in(path.string());
        if (!in.is_open()) continue;
        try {
            json j;
            in >> j;
            sessions.push_back(j);
        } catch (...) {
            continue;
        }
    }
    return sessions;
}

static std::vector<json> anonymizeSessions(const std::vector<json>& sessions,
                                           const PipelineOptions& opts,
                                           SessionAnonymizer& anonymizer) {
    if (!opts.anonymizeEnabled) return sessions;
    anonymizer.setLevel(opts.anonymizeLevel);
    std::vector<json> out;
    out.reserve(sessions.size());
    for (const auto& s : sessions) {
        out.push_back(anonymizer.anonymize(s));
    }
    return out;
}

static std::vector<Trace> sessionsToTraces(const std::vector<json>& sessions) {
    std::vector<Trace> traces;
    SessionTraceConverter converter;
    for (const auto& s : sessions) {
        auto out = converter.convert(s);
        traces.insert(traces.end(), out.begin(), out.end());
    }
    return traces;
}

static bool traceHasMeaningfulWork(const Trace& trace) {
    if (trace.toolCallCount == 0) return false;
    if (trace.steps.size() < 2) return false;
    return true;
}

static std::vector<Trace> filterTraces(const std::vector<Trace>& traces, PipelineStats& stats) {
    std::vector<Trace> filtered;
    for (const auto& t : traces) {
        if (!traceHasMeaningfulWork(t)) {
            stats.tracesFiltered++;
            continue;
        }
        if (!t.success && t.toolCallCount < 2) {
            stats.tracesFiltered++;
            continue;
        }
        filtered.push_back(t);
    }
    return filtered;
}

static std::string traceSignature(const Trace& trace) {
    std::ostringstream oss;
    for (const auto& step : trace.steps) {
        oss << step.role << ":";
        if (step.role == "tool_call" || step.role == "tool_result") {
            oss << step.toolName;
        } else {
            oss << step.content.substr(0, 48);
        }
        oss << "|";
    }
    return oss.str();
}

static std::vector<Trace> dedupeTraces(const std::vector<Trace>& traces, PipelineStats& stats) {
    std::vector<Trace> out;
    std::unordered_set<std::string> seen;
    for (const auto& t : traces) {
        std::string sig = traceSignature(t);
        if (seen.count(sig)) {
            stats.tracesDeduped++;
            continue;
        }
        seen.insert(std::move(sig));
        out.push_back(t);
    }
    return out;
}

static bool validateTrace(const Trace& trace) {
    if (trace.steps.empty()) return false;
    for (const auto& step : trace.steps) {
        if (step.role == "tool_call" || step.role == "tool_result") {
            if (step.toolName.empty()) return false;
        }
    }
    return true;
}

static std::string exportTraces(const std::vector<Trace>& traces, const std::string& format) {
    if (format == "anthropic") {
        return TraceExporter::toAnthropicBatch(traces).dump(2);
    }
    if (format == "openai") {
        return TraceExporter::toOpenAIBatch(traces).dump(2);
    }
    if (format == "markdown") {
        std::ostringstream md;
        for (const auto& t : traces) {
            md << TraceExporter::toMarkdown(t) << "\n";
        }
        return md.str();
    }
    return TraceExporter::toJSONL(traces);
}
