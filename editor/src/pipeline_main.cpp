#include <iostream>
#include <string>
#include <filesystem>
#include "SessionPipeline.h"

static std::string getArg(int argc, char** argv, const std::string& key) {
    for (int i = 1; i < argc - 1; ++i) {
        if (argv[i] == key) return argv[i + 1];
    }
    return "";
}

static bool hasFlag(int argc, char** argv, const std::string& key) {
    for (int i = 1; i < argc; ++i) {
        if (argv[i] == key) return true;
    }
    return false;
}

static SessionAnonymizer::Level parseLevel(const std::string& value, bool& enabled) {
    std::string lower;
    for (char c : value) lower.push_back((char)std::tolower((unsigned char)c));
    if (lower == "off" || lower == "none") {
        enabled = false;
        return SessionAnonymizer::Level::Medium;
    }
    enabled = true;
    if (lower == "light") return SessionAnonymizer::Level::Light;
    if (lower == "full") return SessionAnonymizer::Level::Full;
    return SessionAnonymizer::Level::Medium;
}

static std::string defaultOutputFile(const std::string& format) {
    if (format == "markdown") return "traces.md";
    if (format == "anthropic" || format == "openai") return "traces.json";
    return "traces.jsonl";
}

int main(int argc, char** argv) {
    std::string inputDir = getArg(argc, argv, "--input");
    std::string outputPath = getArg(argc, argv, "--output");
    std::string format = getArg(argc, argv, "--format");
    std::string anonymize = getArg(argc, argv, "--anonymize");
    std::string statsPath = getArg(argc, argv, "--stats");
    std::string seedStr = getArg(argc, argv, "--seed");

    if (inputDir.empty()) {
        std::cerr << "Usage: whetstone_pipeline --input <dir> [--output path] [--format jsonl|anthropic|openai|markdown] "
                     "[--anonymize off|light|medium|full] [--dedup] [--stats file.json] [--seed N]\n";
        return 1;
    }

    PipelineOptions opts;
    if (!format.empty()) opts.format = format;
    if (!anonymize.empty()) opts.anonymizeLevel = parseLevel(anonymize, opts.anonymizeEnabled);
    if (hasFlag(argc, argv, "--dedup")) opts.deduplicate = true;
    if (hasFlag(argc, argv, "--no-dedup")) opts.deduplicate = false;

    SessionAnonymizer anonymizer;
    if (!seedStr.empty()) {
        try {
            anonymizer.setSeed(std::stoull(seedStr));
        } catch (...) {
            std::cerr << "Invalid seed value.\n";
        }
    }

    PipelineStats stats;
    auto sessions = loadSessionsFromDir(inputDir);
    stats.sessionsProcessed = static_cast<int>(sessions.size());

    auto anonSessions = anonymizeSessions(sessions, opts, anonymizer);
    auto traces = sessionsToTraces(anonSessions);
    stats.tracesGenerated = static_cast<int>(traces.size());

    auto filtered = filterTraces(traces, stats);
    if (opts.deduplicate) {
        filtered = dedupeTraces(filtered, stats);
    }

    std::vector<Trace> validated;
    for (const auto& t : filtered) {
        if (validateTrace(t)) {
            validated.push_back(t);
            stats.tracesValidated++;
        }
    }

    std::string output = exportTraces(validated, opts.format);

    if (!outputPath.empty()) {
        std::filesystem::path outPath(outputPath);
        std::error_code ec;
        if (std::filesystem::is_directory(outPath, ec)) {
            outPath /= defaultOutputFile(opts.format);
        }
        std::filesystem::create_directories(outPath.parent_path(), ec);
        std::ofstream out(outPath.string(), std::ios::binary);
        if (!out.is_open()) {
            std::cerr << "Failed to write output: " << outPath.string() << "\n";
            return 2;
        }
        out << output;
    } else {
        std::cout << output << "\n";
    }

    if (!statsPath.empty()) {
        TraceStats traceStats = TraceExporter::computeStats(validated);
        json report;
        report["sessionsProcessed"] = stats.sessionsProcessed;
        report["tracesGenerated"] = stats.tracesGenerated;
        report["tracesFiltered"] = stats.tracesFiltered;
        report["tracesDeduped"] = stats.tracesDeduped;
        report["tracesValidated"] = stats.tracesValidated;
        report["traceStats"] = traceStats.toJson();
        std::ofstream out(statsPath, std::ios::binary);
        if (!out.is_open()) {
            std::cerr << "Failed to write stats: " << statsPath << "\n";
            return 3;
        }
        out << report.dump(2);
    }

    return 0;
}
