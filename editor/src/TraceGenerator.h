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
    MultiStepDebug,
    AnnotateAllSubjects,
    ValidateAndFix,
    SemannoExport,
    CrossLanguageAnnotated
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

        // Kotlin samples
        {"kotlin", "fun greet(name: String): String {\n    return \"Hello, $name!\"\n}\n\nfun sum(a: Int, b: Int): Int = a + b\n",
         "Simple Kotlin functions", 1},
        {"kotlin", "data class Point(val x: Double, val y: Double)\n\nfun distance(a: Point, b: Point): Double {\n    val dx = a.x - b.x\n    val dy = a.y - b.y\n    return Math.sqrt(dx * dx + dy * dy)\n}\n",
         "Kotlin data class and math", 2},

        // C# samples
        {"csharp", "public class Calculator {\n    public int Add(int a, int b) {\n        return a + b;\n    }\n    public int Multiply(int a, int b) {\n        return a * b;\n    }\n}\n",
         "Simple C# calculator class", 1},
        {"csharp", "using System.Collections.Generic;\n\npublic class Stack<T> {\n    private List<T> items = new List<T>();\n    public void Push(T item) { items.Add(item); }\n    public T Pop() { var item = items[items.Count - 1]; items.RemoveAt(items.Count - 1); return item; }\n    public int Count => items.Count;\n}\n",
         "Generic stack implementation", 2},

        // Go samples
        {"go", "package main\n\nfunc fibonacci(n int) int {\n    if n <= 1 {\n        return n\n    }\n    return fibonacci(n-1) + fibonacci(n-2)\n}\n",
         "Recursive fibonacci", 1},

        // Java samples
        {"java", "public class StringUtils {\n    public static String reverse(String s) {\n        return new StringBuilder(s).reverse().toString();\n    }\n    public static boolean isPalindrome(String s) {\n        String rev = reverse(s);\n        return s.equals(rev);\n    }\n}\n",
         "String utility methods", 1},
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
            case ScenarioType::AnnotateAllSubjects:
                return generateAnnotateAllSubjects(params);
            case ScenarioType::ValidateAndFix:
                return generateValidateAndFix(params);
            case ScenarioType::SemannoExport:
                return generateSemannoExport(params);
            case ScenarioType::CrossLanguageAnnotated:
                return generateCrossLanguageAnnotated(params);
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
            ScenarioType::MultiStepDebug,
            ScenarioType::AnnotateAllSubjects,
            ScenarioType::ValidateAndFix,
            ScenarioType::SemannoExport,
            ScenarioType::CrossLanguageAnnotated
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

#include "trace/TraceScenariosPart1.h"
#include "trace/TraceScenariosPart2.h"
