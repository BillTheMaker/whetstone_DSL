#pragma once

// Step 486: Test Plan Generator
// Proposes typed tests from step description heuristics + complexity budget.

#include "StepSpecExpander.h"

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

struct TestPatternSample {
    std::string path;
    std::string content;
};

struct GeneratedTestPlan {
    StepComplexity complexity = StepComplexity::Standard;
    std::vector<PlannedTestStub> tests;
    std::vector<std::string> rationale;

    int countType(PlannedTestType t) const {
        int n = 0;
        for (const auto& x : tests) if (x.type == t) ++n;
        return n;
    }

    bool hasType(PlannedTestType t) const {
        return countType(t) > 0;
    }
};

struct TestPlanInput {
    std::string stepMarkdown;
    std::string stepNumber;
    std::string componentName;
    std::vector<TestPatternSample> priorTests;
    StepComplexity complexityHint = StepComplexity::Standard;
    bool useComplexityHint = false;
};

class TestPlanGenerator {
public:
    static GeneratedTestPlan generate(const TestPlanInput& in) {
        GeneratedTestPlan out;
        const std::string text = lower(in.stepMarkdown);

        out.complexity = in.useComplexityHint ? in.complexityHint : inferComplexity(in);
        out.rationale.push_back("Complexity classified as " + complexityLabel(out.complexity));

        add(out, in, PlannedTestType::Unit, "core behavior correctness", "worker");
        add(out, in, PlannedTestType::Unit, "API surface coverage", "worker");

        if (hasAny(text, {"detect", "validate", "reject", "invalid"})) {
            add(out, in, PlannedTestType::Negative, "rejects invalid input", "worker");
            out.rationale.push_back("Validation keywords detected -> negative tests");
        }
        if (hasAny(text, {"pipeline", "workflow"})) {
            add(out, in, PlannedTestType::Integration, "components compose in full flow", "architect");
            out.rationale.push_back("Pipeline/workflow keywords detected -> integration tests");
        }
        if (hasAny(text, {"rpc", "mcp"})) {
            add(out, in, PlannedTestType::Smoke, "basic RPC/MCP response viability", "worker");
            out.rationale.push_back("RPC/MCP keywords detected -> smoke test");
        }
        if (hasAny(text, {"score", "arithmetic", "limit", "boundary", "max", "min"})) {
            add(out, in, PlannedTestType::Boundary, "boundary limits remain safe", "worker");
            out.rationale.push_back("Boundary-related keywords detected -> boundary test");
        }
        if (hasAny(text, {"contract", "@contract"})) {
            add(out, in, PlannedTestType::Contract, "pre/post contract is preserved", "worker");
            out.rationale.push_back("Contract keywords detected -> contract test");
        }
        if (hasAny(text, {"roundtrip", "invariant", "property", "serialize"})) {
            add(out, in, PlannedTestType::Property, "invariant/property holds across inputs", "worker");
            out.rationale.push_back("Invariant keywords detected -> property test");
        }
        if (hasAny(text, {"performance", "latency", "throughput", "hot path"})) {
            add(out, in, PlannedTestType::Performance, "no major perf regression", "architect");
            out.rationale.push_back("Performance keywords detected -> performance test");
        }
        if (hasAny(text, {"modify", "change existing", "regression"})) {
            add(out, in, PlannedTestType::Regression, "existing behavior remains stable", "architect");
            out.rationale.push_back("Modification keywords detected -> regression test");
        }
        if (priorSuggestsRegression(in.priorTests)) {
            add(out, in, PlannedTestType::Regression, "prior-step regression continuity", "architect");
            out.rationale.push_back("Prior tests indicate regression culture -> regression test");
        }
        if (hasAny(text, {"new struct", "add struct", "data struct"})) {
            add(out, in, PlannedTestType::Unit, "constructor/accessor behavior", "worker");
            out.rationale.push_back("New-struct signal detected -> extra unit test");
        }

        enforceBudget(out, in);
        return out;
    }

private:
    static void add(GeneratedTestPlan& out, const TestPlanInput& in,
                    PlannedTestType type, const std::string& desc,
                    const std::string& routing) {
        PlannedTestStub t;
        t.type = type;
        t.name = "test_step" + (in.stepNumber.empty() ? std::string("x") : in.stepNumber) +
                 "_" + typeLabel(type) + "_" + std::to_string(out.tests.size() + 1);
        t.description = desc;
        t.expectedRouting = routing;
        out.tests.push_back(std::move(t));
    }

    static bool hasAny(const std::string& text, std::initializer_list<const char*> needles) {
        for (const auto& n : needles) if (text.find(n) != std::string::npos) return true;
        return false;
    }

    static std::string lower(const std::string& s) {
        std::string out = s;
        std::transform(out.begin(), out.end(), out.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return out;
    }

    static StepComplexity inferComplexity(const TestPlanInput& in) {
        StepExpanderInput e;
        e.stepMarkdown = in.stepMarkdown;
        e.stepNumber = in.stepNumber;
        e.componentName = in.componentName.empty() ? "Generated" : in.componentName;
        auto spec = StepSpecExpander::expand(e);
        return spec.complexity;
    }

    static bool priorSuggestsRegression(const std::vector<TestPatternSample>& samples) {
        for (const auto& s : samples) {
            const std::string t = lower(s.content + " " + s.path);
            if (t.find("regression") != std::string::npos) return true;
        }
        return false;
    }

    static std::string typeLabel(PlannedTestType t) {
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

    static std::string complexityLabel(StepComplexity c) {
        switch (c) {
            case StepComplexity::Trivial: return "trivial";
            case StepComplexity::Standard: return "standard";
            case StepComplexity::Complex: return "complex";
            case StepComplexity::Pipeline: return "pipeline";
        }
        return "standard";
    }

    static void enforceBudget(GeneratedTestPlan& out, const TestPlanInput& in) {
        (void)in;
        size_t minCount = 5, maxCount = 10;
        if (out.complexity == StepComplexity::Trivial) {
            minCount = 1; maxCount = 4;
        } else if (out.complexity == StepComplexity::Complex) {
            minCount = 10; maxCount = 15;
        } else if (out.complexity == StepComplexity::Pipeline) {
            minCount = 6; maxCount = 10;
        }

        while (out.tests.size() < minCount) {
            PlannedTestType filler = PlannedTestType::Unit;
            std::string desc = "budget filler coverage";
            std::string routing = "worker";
            if (out.complexity == StepComplexity::Pipeline && out.countType(PlannedTestType::Integration) == 0) {
                filler = PlannedTestType::Integration;
                desc = "pipeline integration coverage";
                routing = "architect";
            }
            add(out, in, filler, desc, routing);
        }
        if (out.tests.size() > maxCount) out.tests.resize(maxCount);
    }
};
