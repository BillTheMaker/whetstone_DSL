#pragma once
// Step 218: Trace Export Pipeline
//
// Exports interaction traces in formats suitable for LLM fine-tuning:
//   - Anthropic Messages format (tool_use / tool_result content blocks)
//   - OpenAI Chat format (tool_calls array + tool role messages)
//   - JSONL (one trace per line, streaming-friendly)
//   - Markdown (human-readable for review)
//
// Also provides filtering and statistics.

#include <string>
#include <vector>
#include <sstream>
#include <nlohmann/json.hpp>
#include "TraceGenerator.h"

using json = nlohmann::json;

// -----------------------------------------------------------------------
//  Trace statistics
// -----------------------------------------------------------------------

struct TraceStats {
    int totalTraces = 0;
    int successCount = 0;
    int failureCount = 0;
    int totalToolCalls = 0;
    double avgToolCalls = 0.0;
    double avgSteps = 0.0;
    std::map<std::string, int> scenarioDistribution;
    std::map<std::string, int> languageDistribution;
    std::map<std::string, int> toolUsageCount;
    std::map<std::string, int> difficultyDistribution;

    json toJson() const {
        json j;
        j["totalTraces"] = totalTraces;
        j["successCount"] = successCount;
        j["failureCount"] = failureCount;
        j["totalToolCalls"] = totalToolCalls;
        j["avgToolCalls"] = avgToolCalls;
        j["avgSteps"] = avgSteps;
        j["scenarioDistribution"] = scenarioDistribution;
        j["languageDistribution"] = languageDistribution;
        j["toolUsageCount"] = toolUsageCount;
        j["difficultyDistribution"] = difficultyDistribution;
        return j;
    }
};

// -----------------------------------------------------------------------
//  Filter criteria
// -----------------------------------------------------------------------

struct TraceFilter {
    std::string scenario;       // empty = all
    std::string difficulty;     // empty = all
    std::string language;       // empty = all
    bool successOnly = false;
    bool failureOnly = false;
    int minToolCalls = 0;
    int maxToolCalls = INT_MAX;
};

// -----------------------------------------------------------------------
//  TraceExporter
// -----------------------------------------------------------------------

class TraceExporter {
public:
    // --- Export to Anthropic Messages format ---
    static json toAnthropicMessages(const Trace& trace) {
        json messages = json::array();

        for (const auto& step : trace.steps) {
            if (step.role == "user") {
                messages.push_back({
                    {"role", "user"},
                    {"content", step.content}
                });
            } else if (step.role == "assistant") {
                if (!step.content.empty()) {
                    messages.push_back({
                        {"role", "assistant"},
                        {"content", step.content}
                    });
                }
            } else if (step.role == "tool_call") {
                // Anthropic uses tool_use content blocks inside assistant messages
                json toolUseBlock = {
                    {"type", "tool_use"},
                    {"id", "call_" + step.toolName + "_" + trace.id},
                    {"name", step.toolName},
                    {"input", step.toolInput.is_null() ? json::object() : step.toolInput}
                };
                messages.push_back({
                    {"role", "assistant"},
                    {"content", json::array({toolUseBlock})}
                });
            } else if (step.role == "tool_result") {
                json resultBlock = {
                    {"type", "tool_result"},
                    {"tool_use_id", "call_" + step.toolName + "_" + trace.id},
                    {"content", step.toolOutput.dump()}
                };
                messages.push_back({
                    {"role", "user"},
                    {"content", json::array({resultBlock})}
                });
            }
        }

        json result;
        result["id"] = trace.id;
        result["model"] = "claude-opus-4-6";
        result["messages"] = messages;
        result["metadata"] = {
            {"scenario", trace.scenario},
            {"difficulty", trace.difficulty},
            {"language", trace.language},
            {"toolsUsed", trace.toolsUsed},
            {"toolCallCount", trace.toolCallCount},
            {"success", trace.success}
        };
        return result;
    }

    // --- Export to OpenAI Chat format ---
    static json toOpenAIChat(const Trace& trace) {
        json messages = json::array();
        int callIdx = 0;

        for (size_t i = 0; i < trace.steps.size(); ++i) {
            const auto& step = trace.steps[i];

            if (step.role == "user") {
                messages.push_back({
                    {"role", "user"},
                    {"content", step.content}
                });
            } else if (step.role == "assistant") {
                messages.push_back({
                    {"role", "assistant"},
                    {"content", step.content}
                });
            } else if (step.role == "tool_call") {
                std::string callId = "call_" + std::to_string(callIdx++);
                json toolCall = {
                    {"id", callId},
                    {"type", "function"},
                    {"function", {
                        {"name", step.toolName},
                        {"arguments", step.toolInput.is_null() ? "{}" : step.toolInput.dump()}
                    }}
                };
                messages.push_back({
                    {"role", "assistant"},
                    {"content", nullptr},
                    {"tool_calls", json::array({toolCall})}
                });

                // Look for matching tool_result
                if (i + 1 < trace.steps.size() && trace.steps[i + 1].role == "tool_result") {
                    messages.push_back({
                        {"role", "tool"},
                        {"tool_call_id", callId},
                        {"content", trace.steps[i + 1].toolOutput.dump()}
                    });
                    ++i; // skip the tool_result since we consumed it
                }
            }
        }

        json result;
        result["id"] = trace.id;
        result["model"] = "gpt-4";
        result["messages"] = messages;
        result["metadata"] = {
            {"scenario", trace.scenario},
            {"difficulty", trace.difficulty},
            {"language", trace.language}
        };
        return result;
    }

    // --- Export to JSONL string (one trace per line) ---
    static std::string toJSONL(const std::vector<Trace>& traces) {
        std::ostringstream out;
        for (const auto& trace : traces) {
            out << trace.toJson().dump() << "\n";
        }
        return out.str();
    }

    static std::string toJSONLSingle(const Trace& trace) {
        return trace.toJson().dump() + "\n";
    }

    // --- Export to Markdown (human-readable) ---
    static std::string toMarkdown(const Trace& trace) {
        std::ostringstream md;
        md << "# Trace: " << trace.id << "\n\n";
        md << "- **Scenario:** " << trace.scenario << "\n";
        md << "- **Difficulty:** " << trace.difficulty << "\n";
        md << "- **Language:** " << trace.language << "\n";
        md << "- **Tool calls:** " << trace.toolCallCount << "\n";
        md << "- **Success:** " << (trace.success ? "Yes" : "No") << "\n";
        md << "- **Tools used:** ";
        for (size_t i = 0; i < trace.toolsUsed.size(); ++i) {
            if (i > 0) md << ", ";
            md << "`" << trace.toolsUsed[i] << "`";
        }
        md << "\n\n---\n\n";

        int stepNum = 1;
        for (const auto& step : trace.steps) {
            md << "### Step " << stepNum++ << ": ";
            if (step.role == "user") {
                md << "User\n\n" << step.content << "\n\n";
            } else if (step.role == "assistant") {
                md << "Assistant\n\n" << step.content << "\n\n";
            } else if (step.role == "tool_call") {
                md << "Tool Call: `" << step.toolName << "`\n\n";
                md << "```json\n";
                md << (step.toolInput.is_null() ? "{}" : step.toolInput.dump(2)) << "\n";
                md << "```\n\n";
            } else if (step.role == "tool_result") {
                md << "Tool Result: `" << step.toolName << "`\n\n";
                md << "```json\n";
                md << (step.toolOutput.is_null() ? "{}" : step.toolOutput.dump(2)) << "\n";
                md << "```\n\n";
            }
        }
        return md.str();
    }

    // --- Batch export ---
    static json toAnthropicBatch(const std::vector<Trace>& traces) {
        json arr = json::array();
        for (const auto& t : traces) arr.push_back(toAnthropicMessages(t));
        return arr;
    }

    static json toOpenAIBatch(const std::vector<Trace>& traces) {
        json arr = json::array();
        for (const auto& t : traces) arr.push_back(toOpenAIChat(t));
        return arr;
    }

    // --- Filtering ---
    static std::vector<Trace> filter(const std::vector<Trace>& traces, const TraceFilter& f) {
        std::vector<Trace> result;
        for (const auto& t : traces) {
            if (!f.scenario.empty() && t.scenario != f.scenario) continue;
            if (!f.difficulty.empty() && t.difficulty != f.difficulty) continue;
            if (!f.language.empty() && t.language != f.language) continue;
            if (f.successOnly && !t.success) continue;
            if (f.failureOnly && t.success) continue;
            if (t.toolCallCount < f.minToolCalls) continue;
            if (t.toolCallCount > f.maxToolCalls) continue;
            result.push_back(t);
        }
        return result;
    }

    // --- Statistics ---
    static TraceStats computeStats(const std::vector<Trace>& traces) {
        TraceStats stats;
        stats.totalTraces = static_cast<int>(traces.size());

        int totalSteps = 0;
        for (const auto& t : traces) {
            if (t.success) ++stats.successCount;
            else ++stats.failureCount;
            stats.totalToolCalls += t.toolCallCount;
            totalSteps += static_cast<int>(t.steps.size());
            stats.scenarioDistribution[t.scenario]++;
            stats.languageDistribution[t.language]++;
            stats.difficultyDistribution[t.difficulty]++;
            for (const auto& tool : t.toolsUsed) {
                stats.toolUsageCount[tool]++;
            }
        }

        if (stats.totalTraces > 0) {
            stats.avgToolCalls = static_cast<double>(stats.totalToolCalls) / stats.totalTraces;
            stats.avgSteps = static_cast<double>(totalSteps) / stats.totalTraces;
        }
        return stats;
    }
};
