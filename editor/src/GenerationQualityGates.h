#pragma once

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct GenerationQualitySummary {
    bool compilableEstimate = false;
    bool testsPassedEstimate = false;
    bool placeholderPassed = false;
    bool overallReady = false;
    bool fallbackUsed = false;
    int placeholderCount = 0;
    int todoCount = 0;
    std::vector<std::string> warnings;
};

namespace GenerationQualityGates {

inline std::string lowerCopy(const std::string& s) {
    std::string out = s;
    std::transform(out.begin(), out.end(), out.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return out;
}

inline int countSubstrInsensitive(const std::string& haystack,
                                  const std::string& needle) {
    if (needle.empty()) return 0;
    std::string h = lowerCopy(haystack);
    std::string n = lowerCopy(needle);
    int count = 0;
    size_t pos = 0;
    while (true) {
        pos = h.find(n, pos);
        if (pos == std::string::npos) break;
        ++count;
        pos += n.size();
    }
    return count;
}

inline bool containsInsensitive(const std::string& haystack,
                                const std::string& needle) {
    return countSubstrInsensitive(haystack, needle) > 0;
}

inline GenerationQualitySummary evaluate(const std::string& code,
                                         const std::string& language,
                                         const std::string& note = "") {
    GenerationQualitySummary out;

    out.todoCount =
        countSubstrInsensitive(code, "todo") +
        countSubstrInsensitive(code, "fixme");

    out.placeholderCount =
        countSubstrInsensitive(code, "placeholder") +
        countSubstrInsensitive(code, "auto /* todo") +
        countSubstrInsensitive(code, "/* missing") +
        countSubstrInsensitive(code, "auto self") +
        countSubstrInsensitive(code, "auto enqueue(") +
        countSubstrInsensitive(code, "auto dequeue(") +
        countSubstrInsensitive(code, "auto peek(") +
        countSubstrInsensitive(code, "auto size(") +
        countSubstrInsensitive(code, "auto empty(");

    const bool looksLikeGeneratedStub =
        containsInsensitive(note, "generated using printf") ||
        containsInsensitive(code, "printf(") && containsInsensitive(code, "generated(");
    out.fallbackUsed = looksLikeGeneratedStub;

    const bool nonEmpty = !code.empty();
    const bool hasStructure =
        containsInsensitive(code, "class ") ||
        containsInsensitive(code, "struct ") ||
        containsInsensitive(code, "void ") ||
        containsInsensitive(code, "def ") ||
        containsInsensitive(code, "fn ");

    out.compilableEstimate = nonEmpty && hasStructure && !out.fallbackUsed;
    out.placeholderPassed = (out.placeholderCount + out.todoCount) == 0;

    // Lightweight behavior signal for queue-like code. If not queue code, don't fail tests.
    const bool queueSpecDetected =
        containsInsensitive(code, "priorityqueue") || containsInsensitive(note, "priorityqueue");
    if (!queueSpecDetected) {
        out.testsPassedEstimate = true;
    } else {
        out.testsPassedEstimate =
            containsInsensitive(code, "enqueue") &&
            containsInsensitive(code, "dequeue") &&
            containsInsensitive(code, "peek") &&
            containsInsensitive(code, "size") &&
            containsInsensitive(code, "empty") &&
            containsInsensitive(code, "job_id") &&
            containsInsensitive(code, "priority") &&
            containsInsensitive(code, "payload");
    }

    out.overallReady =
        out.compilableEstimate &&
        out.testsPassedEstimate &&
        out.placeholderPassed &&
        !out.fallbackUsed;

    if (!out.compilableEstimate) out.warnings.push_back("compile_estimate_failed");
    if (!out.testsPassedEstimate) out.warnings.push_back("test_estimate_failed");
    if (!out.placeholderPassed) out.warnings.push_back("placeholder_or_todo_detected");
    if (out.fallbackUsed) out.warnings.push_back("fallback_generation_detected");
    (void)language;  // reserved for language-specific scoring expansions
    return out;
}

inline nlohmann::json toJson(const GenerationQualitySummary& q) {
    return {
        {"quality", {
            {"compilable_estimate", q.compilableEstimate},
            {"placeholder_count", q.placeholderCount},
            {"todo_count", q.todoCount},
            {"fallback_used", q.fallbackUsed},
            {"warnings", q.warnings}
        }},
        {"gates", {
            {"compile", {{"passed", q.compilableEstimate}}},
            {"tests", {{"passed", q.testsPassedEstimate}}},
            {"placeholder", {{"passed", q.placeholderPassed}}},
            {"overall_ready", q.overallReady}
        }}
    };
}

}  // namespace GenerationQualityGates
