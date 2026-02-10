#pragma once
// Steps 214-217: Synthetic Trace Generation
//
// Generates realistic interaction traces for fine-tuning LLMs on Whetstone
// tool use. Traces look like real Claude/Codex sessions working with the editor.
//
// Components:
//   TraceStep       — single step in a trace (user/assistant/tool_call/tool_result)
//   Trace           — complete interaction trace with metadata
//   ScenarioTemplate— parameterized scenario that generates traces
//   TraceGenerator  — engine combining templates + code corpus to produce traces

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <ctime>
#include <cstdlib>
#include <nlohmann/json.hpp>
#include "ast/ASTNode.h"
#include "ast/Module.h"
#include "ast/Function.h"
#include "ast/Variable.h"
#include "ast/Parameter.h"
#include "ast/Expression.h"
#include "ast/Annotation.h"
#include "ast/Serialization.h"
#include "Pipeline.h"
#include "ContextAPI.h"
#include "MemoryStrategyInference.h"

using json = nlohmann::json;

// -----------------------------------------------------------------------
//  Step 214: Trace data model
// -----------------------------------------------------------------------

struct TraceStep {
    std::string role;     // "user", "assistant", "tool_call", "tool_result"
    std::string content;  // text content or JSON string
    std::string toolName; // for tool_call/tool_result
    json toolInput;       // for tool_call
    json toolOutput;      // for tool_result
};

struct Trace {
    std::string id;
    std::string scenario;
    std::string difficulty; // "basic", "intermediate", "advanced"
    std::string language;
    std::vector<TraceStep> steps;
    std::vector<std::string> toolsUsed;
    int toolCallCount = 0;
    bool success = true;

    json toJson() const {
        json j;
        j["id"] = id;
        j["scenario"] = scenario;
        j["difficulty"] = difficulty;
        j["language"] = language;
        j["toolCallCount"] = toolCallCount;
        j["success"] = success;
        j["toolsUsed"] = toolsUsed;
        json stepsArr = json::array();
        for (const auto& s : steps) {
            json step;
            step["role"] = s.role;
            if (!s.content.empty()) step["content"] = s.content;
            if (!s.toolName.empty()) step["toolName"] = s.toolName;
            if (!s.toolInput.is_null()) step["toolInput"] = s.toolInput;
            if (!s.toolOutput.is_null()) step["toolOutput"] = s.toolOutput;
            stepsArr.push_back(step);
        }
        j["steps"] = stepsArr;
        return j;
    }
};

// -----------------------------------------------------------------------
//  Step 215: Scenario templates
// -----------------------------------------------------------------------

enum class ScenarioType {
    ReadAndUnderstand,
    AddAnnotations,
    CrossLanguage,
    Refactor,
    SecurityAudit,
    MultiStepDebug
};

struct ScenarioParams {
    std::string language = "python";
    int functionCount = 3;
    int annotationDensity = 0; // 0=none, 1=some, 2=full
    bool injectErrors = false;
    unsigned int seed = 0;
};

// -----------------------------------------------------------------------
//  Step 216: Code corpus (built-in samples)
// -----------------------------------------------------------------------

struct CodeSample {
    std::string language;
    std::string source;
    std::string description;
    int complexity; // 1-3
};

static std::vector<CodeSample> getBuiltinCorpus() {
    return {
        // Python samples
        {"python", "def add(a, b):\n    return a + b\n\ndef multiply(x, y):\n    return x * y\n",
         "Simple arithmetic functions", 1},
        {"python", "def process_data(items):\n    result = []\n    for item in items:\n        if item > 0:\n            result.append(item * 2)\n    return result\n\ndef filter_negative(data):\n    return [x for x in data if x >= 0]\n",
         "Data processing with lists", 2},
        {"python", "class DataStore:\n    def __init__(self):\n        self.cache = {}\n    def get(self, key):\n        return self.cache.get(key)\n    def put(self, key, value):\n        self.cache[key] = value\n    def delete(self, key):\n        if key in self.cache:\n            del self.cache[key]\n",
         "Cache data store class", 2},
        {"python", "import json\n\ndef parse_config(path):\n    with open(path) as f:\n        return json.load(f)\n\ndef validate_config(config):\n    required = ['host', 'port', 'database']\n    for key in required:\n        if key not in config:\n            raise ValueError(f'Missing key: {key}')\n    return True\n",
         "Config parsing and validation", 2},
        {"python", "def fibonacci(n):\n    if n <= 1:\n        return n\n    a, b = 0, 1\n    for _ in range(2, n + 1):\n        a, b = b, a + b\n    return b\n\ndef factorial(n):\n    result = 1\n    for i in range(2, n + 1):\n        result = result * i\n    return result\n",
         "Mathematical functions", 1},

        // C++ samples
        {"cpp", "#include <vector>\nint sum(std::vector<int>& nums) {\n    int total = 0;\n    for (int n : nums) total += n;\n    return total;\n}\n",
         "Vector sum function", 1},
        {"cpp", "#include <memory>\nclass Node {\npublic:\n    int value;\n    std::unique_ptr<Node> next;\n    Node(int v) : value(v), next(nullptr) {}\n};\n\nvoid append(Node* head, int val) {\n    Node* curr = head;\n    while (curr->next) curr = curr->next.get();\n    curr->next = std::make_unique<Node>(val);\n}\n",
         "Linked list with unique_ptr", 2},

        // JavaScript samples
        {"javascript", "function fetchData(url) {\n    return fetch(url).then(r => r.json());\n}\n\nfunction processResponse(data) {\n    return data.items.filter(item => item.active).map(item => item.name);\n}\n",
         "Async data fetching", 2},

        // Rust samples
        {"rust", "fn binary_search(arr: &[i32], target: i32) -> Option<usize> {\n    let mut low = 0;\n    let mut high = arr.len();\n    while low < high {\n        let mid = low + (high - low) / 2;\n        if arr[mid] == target { return Some(mid); }\n        if arr[mid] < target { low = mid + 1; }\n        else { high = mid; }\n    }\n    None\n}\n",
         "Binary search implementation", 2},
    };
}

// -----------------------------------------------------------------------
//  Step 217: Trace generator engine
// -----------------------------------------------------------------------

class TraceGenerator {
public:
    TraceGenerator() : corpus_(getBuiltinCorpus()) {}

    void setSeed(unsigned int seed) { seed_ = seed; srand(seed); }

    // Generate a single trace for a given scenario
    Trace generate(ScenarioType scenario, const ScenarioParams& params = {}) {
        if (params.seed != 0) setSeed(params.seed);

        switch (scenario) {
            case ScenarioType::ReadAndUnderstand:
                return generateReadAndUnderstand(params);
            case ScenarioType::AddAnnotations:
                return generateAddAnnotations(params);
            case ScenarioType::CrossLanguage:
                return generateCrossLanguage(params);
            case ScenarioType::Refactor:
                return generateRefactor(params);
            case ScenarioType::SecurityAudit:
                return generateSecurityAudit(params);
            case ScenarioType::MultiStepDebug:
                return generateMultiStepDebug(params);
        }
        return {};
    }

    // Generate a batch of traces with varied scenarios
    std::vector<Trace> generateBatch(int count, unsigned int baseSeed = 42) {
        std::vector<Trace> traces;
        std::vector<ScenarioType> types = {
            ScenarioType::ReadAndUnderstand,
            ScenarioType::AddAnnotations,
            ScenarioType::CrossLanguage,
            ScenarioType::Refactor,
            ScenarioType::SecurityAudit,
            ScenarioType::MultiStepDebug
        };

        for (int i = 0; i < count; ++i) {
            ScenarioType type = types[i % types.size()];
            ScenarioParams params;
            params.seed = baseSeed + i;
            params.language = (i % 2 == 0) ? "python" : "cpp";
            params.functionCount = 2 + (i % 3);
            traces.push_back(generate(type, params));
        }
        return traces;
    }

    const std::vector<CodeSample>& corpus() const { return corpus_; }

private:
    std::vector<CodeSample> corpus_;
    unsigned int seed_ = 42;
    int traceCounter_ = 0;

    std::string nextId() {
        return "trace_" + std::to_string(++traceCounter_);
    }

    const CodeSample& pickSample(const std::string& language) {
        std::vector<const CodeSample*> matches;
        for (const auto& s : corpus_) {
            if (s.language == language) matches.push_back(&s);
        }
        if (matches.empty()) return corpus_[0];
        return *matches[rand() % matches.size()];
    }

    void addToolUsed(Trace& trace, const std::string& tool) {
        for (const auto& t : trace.toolsUsed) {
            if (t == tool) return;
        }
        trace.toolsUsed.push_back(tool);
    }

    // --- Scenario: Read & Understand ---
    Trace generateReadAndUnderstand(const ScenarioParams& params) {
        Trace trace;
        trace.id = nextId();
        trace.scenario = "read_and_understand";
        trace.difficulty = "basic";
        trace.language = params.language;

        const auto& sample = pickSample(params.language);

        // User asks to understand the code
        trace.steps.push_back({"user", "Read the current AST and describe its structure. "
            "What functions are defined and what do they do?", "", {}, {}});

        // Assistant thinks
        trace.steps.push_back({"assistant", "I'll read the AST to understand the code structure.", "", {}, {}});

        // Tool call: getAST
        json astResult = simulateGetAST(sample.source, params.language);
        trace.steps.push_back({"tool_call", "", "whetstone_get_ast", json::object(), {}});
        trace.steps.push_back({"tool_result", "", "whetstone_get_ast", {}, astResult});
        trace.toolCallCount += 1;
        addToolUsed(trace, "whetstone_get_ast");

        // Assistant summarizes
        trace.steps.push_back({"assistant",
            "The module contains " + std::to_string(params.functionCount) +
            " functions in " + params.language + ". " + sample.description, "", {}, {}});

        return trace;
    }

    // --- Scenario: Add Annotations ---
    Trace generateAddAnnotations(const ScenarioParams& params) {
        Trace trace;
        trace.id = nextId();
        trace.scenario = "add_annotations";
        trace.difficulty = "intermediate";
        trace.language = params.language;

        const auto& sample = pickSample(params.language);

        trace.steps.push_back({"user",
            "Suggest memory annotations for all functions in the current module.", "", {}, {}});

        trace.steps.push_back({"assistant",
            "I'll get the AST first, then check each function for annotation suggestions.", "", {}, {}});

        // Get AST
        json astResult = simulateGetAST(sample.source, params.language);
        trace.steps.push_back({"tool_call", "", "whetstone_get_ast", json::object(), {}});
        trace.steps.push_back({"tool_result", "", "whetstone_get_ast", {}, astResult});
        trace.toolCallCount += 1;
        addToolUsed(trace, "whetstone_get_ast");

        // Suggest annotations
        json suggestInput = {{"nodeId", "mod1"}};
        json suggestResult = {
            {"scopeId", "mod1"},
            {"suggestions", json::array({{
                {"nodeId", "mod1"},
                {"annotationType", "ReclaimAnnotation"},
                {"strategy", "Tracing"},
                {"reason", "Python module uses garbage collection"},
                {"confidence", 0.85}
            }})},
            {"diagnostics", json::array()}
        };
        trace.steps.push_back({"tool_call", "", "whetstone_suggest_annotations", suggestInput, {}});
        trace.steps.push_back({"tool_result", "", "whetstone_suggest_annotations", {}, suggestResult});
        trace.toolCallCount += 1;
        addToolUsed(trace, "whetstone_suggest_annotations");

        trace.steps.push_back({"assistant",
            "Found a suggestion: @Reclaim(Tracing) for the module (confidence: 0.85). "
            "This makes sense since Python uses garbage collection. Applying it.", "", {}, {}});

        // Apply annotation
        json applyInput = {
            {"nodeId", "mod1"}, {"annotationType", "ReclaimAnnotation"},
            {"strategy", "Tracing"}, {"reason", "Python GC"}, {"confidence", 0.85}
        };
        json applyResult = {{"success", true}, {"warning", ""}};
        trace.steps.push_back({"tool_call", "", "whetstone_apply_annotation", applyInput, {}});
        trace.steps.push_back({"tool_result", "", "whetstone_apply_annotation", {}, applyResult});
        trace.toolCallCount += 1;
        addToolUsed(trace, "whetstone_apply_annotation");

        trace.steps.push_back({"assistant",
            "Applied @Reclaim(Tracing) annotation to the module. "
            "All functions now have appropriate memory strategy annotations.", "", {}, {}});

        return trace;
    }

    // --- Scenario: Cross-Language ---
    Trace generateCrossLanguage(const ScenarioParams& params) {
        Trace trace;
        trace.id = nextId();
        trace.scenario = "cross_language";
        trace.difficulty = "advanced";
        trace.language = params.language;

        std::string target = (params.language == "python") ? "cpp" : "python";

        trace.steps.push_back({"user",
            "Project this " + params.language + " code to " + target + ".", "", {}, {}});

        trace.steps.push_back({"assistant",
            "I'll run the pipeline to project the code from " + params.language +
            " to " + target + ".", "", {}, {}});

        json pipelineInput = {
            {"source", pickSample(params.language).source},
            {"sourceLanguage", params.language},
            {"targetLanguage", target}
        };
        json pipelineResult = {
            {"success", true},
            {"generatedCode", "// Generated " + target + " code"},
            {"parseDiagnostics", json::array()},
            {"validationDiagnostics", json::array()},
            {"violations", json::array()},
            {"suggestions", json::array()},
            {"foldCount", 0}, {"dceCount", 0}
        };
        trace.steps.push_back({"tool_call", "", "whetstone_run_pipeline", pipelineInput, {}});
        trace.steps.push_back({"tool_result", "", "whetstone_run_pipeline", {}, pipelineResult});
        trace.toolCallCount += 1;
        addToolUsed(trace, "whetstone_run_pipeline");

        trace.steps.push_back({"assistant",
            "Successfully projected the code to " + target + ". "
            "The annotations were adapted for the target language.", "", {}, {}});

        return trace;
    }

    // --- Scenario: Refactor ---
    Trace generateRefactor(const ScenarioParams& params) {
        Trace trace;
        trace.id = nextId();
        trace.scenario = "refactor";
        trace.difficulty = "intermediate";
        trace.language = params.language;

        trace.steps.push_back({"user",
            "Rename the first function in the module to 'compute'.", "", {}, {}});

        trace.steps.push_back({"assistant",
            "I'll get the AST, find the first function, and rename it.", "", {}, {}});

        json astResult = simulateGetAST(pickSample(params.language).source, params.language);
        trace.steps.push_back({"tool_call", "", "whetstone_get_ast", json::object(), {}});
        trace.steps.push_back({"tool_result", "", "whetstone_get_ast", {}, astResult});
        trace.toolCallCount += 1;
        addToolUsed(trace, "whetstone_get_ast");

        json mutateInput = {
            {"type", "setProperty"}, {"nodeId", "fn1"},
            {"property", "name"}, {"value", "compute"}
        };
        json mutateResult = {{"success", true}, {"warning", ""}};
        trace.steps.push_back({"tool_call", "", "whetstone_mutate", mutateInput, {}});
        trace.steps.push_back({"tool_result", "", "whetstone_mutate", {}, mutateResult});
        trace.toolCallCount += 1;
        addToolUsed(trace, "whetstone_mutate");

        trace.steps.push_back({"assistant",
            "Renamed the first function to 'compute'. The AST has been updated.", "", {}, {}});

        return trace;
    }

    // --- Scenario: Security Audit ---
    Trace generateSecurityAudit(const ScenarioParams& params) {
        Trace trace;
        trace.id = nextId();
        trace.scenario = "security_audit";
        trace.difficulty = "intermediate";
        trace.language = params.language;

        trace.steps.push_back({"user",
            "Check the project dependencies for security vulnerabilities.", "", {}, {}});

        trace.steps.push_back({"assistant",
            "I'll examine the AST to identify imported libraries, then check for vulnerabilities.", "", {}, {}});

        json astResult = simulateGetAST(pickSample(params.language).source, params.language);
        trace.steps.push_back({"tool_call", "", "whetstone_get_ast", json::object(), {}});
        trace.steps.push_back({"tool_result", "", "whetstone_get_ast", {}, astResult});
        trace.toolCallCount += 1;
        addToolUsed(trace, "whetstone_get_ast");

        trace.steps.push_back({"assistant",
            "The module has no external imports with known vulnerabilities. "
            "The code uses standard library functions only.", "", {}, {}});

        return trace;
    }

    // --- Scenario: Multi-Step Debug ---
    Trace generateMultiStepDebug(const ScenarioParams& params) {
        Trace trace;
        trace.id = nextId();
        trace.scenario = "multi_step_debug";
        trace.difficulty = "advanced";
        trace.language = params.language;

        trace.steps.push_back({"user",
            "Check the code for annotation errors and fix them.", "", {}, {}});

        trace.steps.push_back({"assistant",
            "I'll get the AST, check for annotation issues, and fix any problems.", "", {}, {}});

        // Get AST
        json astResult = simulateGetAST(pickSample(params.language).source, params.language);
        trace.steps.push_back({"tool_call", "", "whetstone_get_ast", json::object(), {}});
        trace.steps.push_back({"tool_result", "", "whetstone_get_ast", {}, astResult});
        trace.toolCallCount += 1;
        addToolUsed(trace, "whetstone_get_ast");

        // Suggest annotations
        json suggestResult = {
            {"scopeId", "mod1"},
            {"suggestions", json::array({{
                {"nodeId", "fn1"},
                {"annotationType", "ReclaimAnnotation"},
                {"strategy", "Tracing"},
                {"reason", "Unannotated function"},
                {"confidence", 0.7}
            }})},
            {"diagnostics", json::array({{
                {"severity", "warning"},
                {"message", "Function 'fn1' has no memory annotation"},
                {"nodeId", "fn1"}
            }})}
        };
        trace.steps.push_back({"tool_call", "", "whetstone_suggest_annotations", {{"nodeId", "fn1"}}, {}});
        trace.steps.push_back({"tool_result", "", "whetstone_suggest_annotations", {}, suggestResult});
        trace.toolCallCount += 1;
        addToolUsed(trace, "whetstone_suggest_annotations");

        trace.steps.push_back({"assistant",
            "Found a warning: function 'fn1' has no memory annotation. "
            "Suggested @Reclaim(Tracing) with confidence 0.7. Applying the fix.", "", {}, {}});

        // Apply fix
        json applyResult = {{"success", true}, {"warning", ""}};
        trace.steps.push_back({"tool_call", "", "whetstone_apply_annotation",
            {{"nodeId", "fn1"}, {"annotationType", "ReclaimAnnotation"},
             {"strategy", "Tracing"}, {"reason", "fix"}, {"confidence", 0.7}}, {}});
        trace.steps.push_back({"tool_result", "", "whetstone_apply_annotation", {}, applyResult});
        trace.toolCallCount += 1;
        addToolUsed(trace, "whetstone_apply_annotation");

        trace.steps.push_back({"assistant",
            "Fixed the annotation issue. All functions now have proper memory annotations.", "", {}, {}});

        return trace;
    }

    // Simulate getAST result for a code sample
    json simulateGetAST(const std::string& source, const std::string& language) {
        // Use real parser if possible, fall back to mock
        Pipeline pipeline;
        std::vector<ParseDiagnostic> diags;
        auto mod = pipeline.parse(source, language, diags);
        if (mod) {
            return {
                {"ast", toJson(mod.get())},
                {"annotationCount", 0},
                {"diagnostics", json::array()}
            };
        }
        // Mock fallback
        return {
            {"ast", {{"conceptType", "Module"}, {"id", "mod1"}, {"name", "parsed"}}},
            {"annotationCount", 0},
            {"diagnostics", json::array()}
        };
    }
};
