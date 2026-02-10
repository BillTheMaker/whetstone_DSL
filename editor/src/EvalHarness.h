#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <chrono>
#include <nlohmann/json.hpp>
#include "TraceGenerator.h"
#include "ast/Serialization.h"

using json = nlohmann::json;

struct EvalTask {
    std::string id;
    std::string description;
    std::string language = "python";
    std::string setupAst;
    json expectedOutcome = json::object();
    std::vector<std::string> tools;
    int maxSteps = 8;
    bool allowPartial = false;
};

struct EvalResult {
    std::string taskId;
    bool passed = false;
    int steps = 0;
    int toolCallCount = 0;
    std::vector<std::string> errors;
    double durationMs = 0.0;
    double score = 0.0;
};

class EvalRunner {
public:
    static EvalTask taskFromJson(const json& j) {
        EvalTask task;
        task.id = j.value("id", "");
        task.description = j.value("description", "");
        task.language = j.value("language", "python");
        task.setupAst = j.value("setupAst", "");
        task.expectedOutcome = j.value("expectedOutcome", json::object());
        if (j.contains("tools") && j["tools"].is_array()) {
            for (const auto& t : j["tools"]) task.tools.push_back(t.get<std::string>());
        }
        task.maxSteps = j.value("maxSteps", 8);
        task.allowPartial = j.value("allowPartial", false);
        return task;
    }

    static json taskToJson(const EvalTask& task) {
        json j;
        j["id"] = task.id;
        j["description"] = task.description;
        j["language"] = task.language;
        j["setupAst"] = task.setupAst;
        j["expectedOutcome"] = task.expectedOutcome;
        j["tools"] = task.tools;
        j["maxSteps"] = task.maxSteps;
        j["allowPartial"] = task.allowPartial;
        return j;
    }

    static Trace traceFromJson(const json& j) {
        Trace t;
        t.id = j.value("id", "");
        t.scenario = j.value("scenario", "");
        t.difficulty = j.value("difficulty", "");
        t.language = j.value("language", "");
        t.toolCallCount = j.value("toolCallCount", 0);
        t.success = j.value("success", true);
        if (j.contains("toolsUsed") && j["toolsUsed"].is_array()) {
            for (const auto& tool : j["toolsUsed"]) t.toolsUsed.push_back(tool.get<std::string>());
        }
        if (j.contains("steps") && j["steps"].is_array()) {
            for (const auto& s : j["steps"]) {
                TraceStep step;
                step.role = s.value("role", "");
                step.content = s.value("content", "");
                step.toolName = s.value("toolName", "");
                if (s.contains("toolInput")) step.toolInput = s["toolInput"];
                if (s.contains("toolOutput")) step.toolOutput = s["toolOutput"];
                t.steps.push_back(std::move(step));
            }
        }
        return t;
    }

    static std::vector<EvalTask> loadTasksFromDir(const std::string& dir) {
        std::vector<EvalTask> tasks;
        std::error_code ec;
        for (const auto& entry : std::filesystem::directory_iterator(dir, ec)) {
            if (ec) break;
            if (!entry.is_regular_file()) continue;
            if (entry.path().extension() != ".json") continue;
            std::ifstream in(entry.path());
            if (!in.is_open()) continue;
            json j;
            in >> j;
            tasks.push_back(taskFromJson(j));
        }
        return tasks;
    }

    static std::vector<Trace> loadTracesJsonl(const std::string& path) {
        std::vector<Trace> traces;
        std::ifstream in(path);
        if (!in.is_open()) return traces;
        std::string line;
        while (std::getline(in, line)) {
            if (line.empty()) continue;
            json j = json::parse(line, nullptr, false);
            if (j.is_discarded()) continue;
            traces.push_back(traceFromJson(j));
        }
        return traces;
    }

    static EvalResult evaluateTraceForTask(const EvalTask& task, const Trace& trace) {
        EvalResult res;
        res.taskId = task.id;
        res.steps = static_cast<int>(trace.steps.size());
        res.toolCallCount = trace.toolCallCount;

        const auto start = std::chrono::high_resolution_clock::now();
        int expected = 0;
        int matched = 0;

        if (task.maxSteps > 0 && res.steps > task.maxSteps) {
            res.errors.push_back("Exceeded maxSteps");
        }

        if (task.expectedOutcome.contains("toolCalls")) {
            for (const auto& tool : task.expectedOutcome["toolCalls"]) {
                expected++;
                std::string name = tool.get<std::string>();
                bool found = false;
                for (const auto& step : trace.steps) {
                    if (step.role == "tool_call" && step.toolName == name) {
                        found = true;
                        break;
                    }
                }
                if (found) matched++;
                else res.errors.push_back("Missing tool call: " + name);
            }
        }

        if (task.expectedOutcome.contains("mustContain")) {
            for (const auto& needle : task.expectedOutcome["mustContain"]) {
                expected++;
                std::string text = needle.get<std::string>();
                bool found = false;
                for (const auto& step : trace.steps) {
                    if (step.role == "assistant" && step.content.find(text) != std::string::npos) {
                        found = true;
                        break;
                    }
                }
                if (found) matched++;
                else res.errors.push_back("Missing assistant content: " + text);
            }
        }

        if (expected == 0) {
            res.score = trace.success ? 1.0 : 0.0;
        } else {
            res.score = static_cast<double>(matched) / static_cast<double>(expected);
        }

        res.passed = res.errors.empty();
        if (task.allowPartial && res.score > 0.0) {
            res.passed = res.score >= 0.5;
        }

        const auto end = std::chrono::high_resolution_clock::now();
        res.durationMs = std::chrono::duration<double, std::milli>(end - start).count();
        return res;
    }

    static json resultToJson(const EvalResult& res) {
        json j;
        j["taskId"] = res.taskId;
        j["passed"] = res.passed;
        j["steps"] = res.steps;
        j["toolCallCount"] = res.toolCallCount;
        j["errors"] = res.errors;
        j["durationMs"] = res.durationMs;
        j["score"] = res.score;
        return j;
    }

    static json run(const std::vector<EvalTask>& tasks,
                    const std::vector<Trace>& traces) {
        json report;
        json results = json::array();
        int passed = 0;
        double totalScore = 0.0;
        std::unordered_map<std::string, Trace> traceById;
        for (const auto& t : traces) traceById[t.id] = t;

        for (const auto& task : tasks) {
            auto it = traceById.find(task.id);
            if (it == traceById.end()) {
                EvalResult missing;
                missing.taskId = task.id;
                missing.passed = false;
                missing.errors.push_back("Missing trace");
                results.push_back(resultToJson(missing));
                continue;
            }
            EvalResult r = evaluateTraceForTask(task, it->second);
            if (r.passed) passed++;
            totalScore += r.score;
            results.push_back(resultToJson(r));
        }

        report["totalTasks"] = static_cast<int>(tasks.size());
        report["passed"] = passed;
        report["avgScore"] = tasks.empty() ? 0.0 : totalScore / tasks.size();
        report["results"] = results;
        return report;
    }
};
