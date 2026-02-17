#pragma once

// Step 482: Step Spec Expander
// Expands a sprint step description into test plan, header skeleton, and build entry.

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

enum class StepComplexity {
    Trivial,
    Standard,
    Complex,
    Pipeline
};

enum class PlannedTestType {
    Unit,
    Negative,
    Integration,
    Contract,
    Regression,
    Boundary,
    Property,
    Smoke,
    Performance
};

struct PlannedTestStub {
    std::string name;
    PlannedTestType type = PlannedTestType::Unit;
    std::string description;
    std::string expectedRouting; // worker | architect
};

struct HeaderFieldStub {
    std::string type;
    std::string name;
};

struct HeaderMethodStub {
    std::string returnType;
    std::string signature;
    std::string docComment;
};

struct HeaderSkeletonStub {
    std::string fileName;
    std::string className;
    std::string docComment;
    std::vector<HeaderFieldStub> fields;
    std::vector<HeaderMethodStub> methods;
};

struct BuildEntryStub {
    std::string buildSystem;    // cmake, cargo, etc.
    std::string targetName;
    std::string testFileName;
    std::string snippet;
};

struct StepSpec {
    StepComplexity complexity = StepComplexity::Standard;
    std::vector<PlannedTestStub> testPlan;
    HeaderSkeletonStub headerSkeleton;
    BuildEntryStub buildEntry;
    std::vector<std::string> warnings;
};

struct StepExpanderInput {
    std::string stepMarkdown;
    std::string stepNumber; // e.g. "482"
    std::string componentName;
    std::string buildSystem = "cmake";
    std::vector<std::string> projectConventions;
    std::vector<std::string> priorStepHeaders;
};

class StepSpecExpander {
public:
    static StepSpec expand(const StepExpanderInput& in) {
        StepSpec out;
        const std::string text = lower(in.stepMarkdown);
        if (trim(in.stepMarkdown).empty()) {
            out.warnings.push_back("Step markdown is empty");
        }

        out.complexity = classify(text);
        out.testPlan = makeTestPlan(text, in.stepNumber, out.complexity);
        out.headerSkeleton = makeHeaderSkeleton(in);
        out.buildEntry = makeBuildEntry(in);

        if (in.projectConventions.empty()) {
            out.warnings.push_back("Project conventions not provided");
        }
        if (in.priorStepHeaders.empty()) {
            out.warnings.push_back("Prior step headers not provided");
        }
        return out;
    }

private:
    static std::string trim(const std::string& s) {
        size_t b = 0, e = s.size();
        while (b < e && std::isspace(static_cast<unsigned char>(s[b]))) ++b;
        while (e > b && std::isspace(static_cast<unsigned char>(s[e - 1]))) --e;
        return s.substr(b, e - b);
    }

    static std::string lower(const std::string& s) {
        std::string out = s;
        std::transform(out.begin(), out.end(), out.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return out;
    }

    static bool hasAny(const std::string& text, std::initializer_list<const char*> terms) {
        for (const auto& t : terms) {
            if (text.find(t) != std::string::npos) return true;
        }
        return false;
    }

    static StepComplexity classify(const std::string& text) {
        if (hasAny(text, {"pipeline", "workflow", "phase-end", "integration suite"})) {
            return StepComplexity::Pipeline;
        }
        int signals = 0;
        if (hasAny(text, {"orchestrate", "dispatch", "protocol"})) ++signals;
        if (hasAny(text, {"validate", "extract", "generate"})) ++signals;
        if (hasAny(text, {"convention", "context", "skeleton"})) ++signals;
        if (hasAny(text, {"mcp", "rpc"})) ++signals;
        if (signals >= 3) return StepComplexity::Complex;
        if (text.size() < 120 && hasAny(text, {"struct", "utility", "simple"})) {
            return StepComplexity::Trivial;
        }
        return StepComplexity::Standard;
    }

    static std::vector<PlannedTestStub> makeTestPlan(const std::string& text,
                                                     const std::string& step,
                                                     StepComplexity complexity) {
        std::vector<PlannedTestStub> plan;

        add(plan, step, PlannedTestType::Unit, "core behavior generates expected spec", "worker");
        add(plan, step, PlannedTestType::Unit, "header skeleton has required API fields", "worker");

        if (hasAny(text, {"validate", "invalid", "reject", "error"})) {
            add(plan, step, PlannedTestType::Negative, "invalid input is rejected with warning", "worker");
            add(plan, step, PlannedTestType::Negative, "missing required fields emits warning", "worker");
        }
        if (hasAny(text, {"boundary", "limit", "score", "arithmetic", "max"})) {
            add(plan, step, PlannedTestType::Boundary, "boundary values keep stable classification", "worker");
        }
        if (hasAny(text, {"pipeline", "workflow"})) {
            add(plan, step, PlannedTestType::Integration, "full flow composes all outputs", "architect");
            add(plan, step, PlannedTestType::Integration, "output is sufficient for worker handoff", "architect");
        }
        if (hasAny(text, {"mcp", "rpc"})) {
            add(plan, step, PlannedTestType::Smoke, "tooling-facing shape can be consumed", "worker");
        }
        if (hasAny(text, {"modify", "change", "existing"})) {
            add(plan, step, PlannedTestType::Regression, "prior behavior remains stable", "architect");
        }

        enforceBudget(plan, step, complexity);
        return plan;
    }

    static void add(std::vector<PlannedTestStub>& plan,
                    const std::string& step,
                    PlannedTestType type,
                    const std::string& description,
                    const std::string& routing) {
        PlannedTestStub t;
        t.type = type;
        t.name = "test_step" + (step.empty() ? std::string("x") : step) + "_" +
                 testTypeLabel(type) + "_" + std::to_string(plan.size() + 1);
        t.description = description;
        t.expectedRouting = routing;
        plan.push_back(std::move(t));
    }

    static std::string testTypeLabel(PlannedTestType t) {
        switch (t) {
            case PlannedTestType::Unit: return "unit";
            case PlannedTestType::Negative: return "negative";
            case PlannedTestType::Integration: return "integration";
            case PlannedTestType::Contract: return "contract";
            case PlannedTestType::Regression: return "regression";
            case PlannedTestType::Boundary: return "boundary";
            case PlannedTestType::Property: return "property";
            case PlannedTestType::Smoke: return "smoke";
            case PlannedTestType::Performance: return "performance";
        }
        return "unit";
    }

    static void enforceBudget(std::vector<PlannedTestStub>& plan,
                              const std::string& step,
                              StepComplexity complexity) {
        size_t minCount = 5, maxCount = 10;
        if (complexity == StepComplexity::Trivial) {
            minCount = 1; maxCount = 4;
        } else if (complexity == StepComplexity::Complex) {
            minCount = 10; maxCount = 15;
        } else if (complexity == StepComplexity::Pipeline) {
            minCount = 6; maxCount = 10;
        }

        while (plan.size() < minCount) {
            add(plan, step, PlannedTestType::Unit, "additional coverage for complexity budget", "worker");
        }
        if (plan.size() > maxCount) plan.resize(maxCount);
    }

    static std::string fallbackComponentName(const std::string& markdown) {
        std::string out;
        for (char c : markdown) {
            unsigned char uc = static_cast<unsigned char>(c);
            if (std::isalnum(uc)) out.push_back(c);
            if (out.size() >= 24) break;
        }
        if (out.empty()) return "GeneratedComponent";
        out[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(out[0])));
        return out;
    }

    static HeaderSkeletonStub makeHeaderSkeleton(const StepExpanderInput& in) {
        HeaderSkeletonStub h;
        h.className = in.componentName.empty() ? fallbackComponentName(in.stepMarkdown)
                                               : in.componentName;
        h.fileName = h.className + ".h";
        h.docComment = "Auto-generated skeleton for sprint step expansion";
        h.fields.push_back({"std::string", "stepDescription"});
        h.fields.push_back({"StepComplexity", "complexity"});
        h.fields.push_back({"std::vector<PlannedTestStub>", "testPlan"});
        h.methods.push_back({"static StepSpec", "expand(const StepExpanderInput& in)",
                             "Expand step markdown into structured spec output."});
        h.methods.push_back({"static StepComplexity", "classify(const std::string& text)",
                             "Classify complexity from step description heuristics."});
        h.methods.push_back({"static std::vector<PlannedTestStub>",
                             "makeTestPlan(const std::string& text, const std::string& step, StepComplexity c)",
                             "Generate typed test stubs constrained by complexity budget."});
        return h;
    }

    static BuildEntryStub makeBuildEntry(const StepExpanderInput& in) {
        BuildEntryStub b;
        b.buildSystem = in.buildSystem.empty() ? "cmake" : lower(in.buildSystem);
        const std::string step = in.stepNumber.empty() ? "x" : in.stepNumber;
        b.targetName = "step" + step + "_test";
        b.testFileName = "tests/step" + step + "_test.cpp";

        if (b.buildSystem == "cmake") {
            b.snippet =
                "add_executable(" + b.targetName + " " + b.testFileName + ")\n"
                "target_include_directories(" + b.targetName + " PRIVATE src)\n";
        } else if (b.buildSystem == "cargo") {
            b.snippet = "[[test]]\nname = \"" + b.targetName + "\"\npath = \"" + b.testFileName + "\"\n";
        } else {
            b.snippet = "# Build entry stub for " + b.buildSystem + ": " + b.targetName;
        }
        return b;
    }
};
