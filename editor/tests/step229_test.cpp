// Step 229: Tool definition tests.

#include <cassert>
#include <fstream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct ToolSpec {
    std::set<std::string> props;
    std::set<std::string> required;
};

static std::string readFile(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) return {};
    return std::string((std::istreambuf_iterator<char>(f)),
                       std::istreambuf_iterator<char>());
}

static json readJson(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) return json();
    json j;
    f >> j;
    return j;
}

static void assertContains(const std::string& text, const std::string& needle) {
    assert(text.find(needle) != std::string::npos);
}

static void assertSubset(const std::set<std::string>& expected,
                         const std::set<std::string>& actual) {
    for (const auto& item : expected) {
        assert(actual.count(item) == 1);
    }
}

static std::map<std::string, ToolSpec> expectedToolSpecs() {
    std::map<std::string, ToolSpec> specs;
    specs["whetstone_get_ast"] = {{}, {}};
    specs["whetstone_mutate"] = {{"type", "nodeId", "property", "value", "parentId", "role", "node"}, {"type"}};
    specs["whetstone_batch_mutate"] = {{"mutations"}, {"mutations"}};
    specs["whetstone_get_scope"] = {{"nodeId"}, {"nodeId"}};
    specs["whetstone_get_call_hierarchy"] = {{"functionId"}, {"functionId"}};
    specs["whetstone_suggest_annotations"] = {{"nodeId", "line", "col"}, {}};
    specs["whetstone_apply_annotation"] = {{"nodeId", "annotationType", "strategy", "reason", "confidence"}, {"nodeId", "annotationType", "strategy"}};
    specs["whetstone_generate_code"] = {{"spec", "preferImports"}, {"spec"}};
    specs["whetstone_run_pipeline"] = {{"source", "sourceLanguage", "targetLanguage"}, {"source", "sourceLanguage", "targetLanguage"}};
    specs["whetstone_project_language"] = {{"targetLanguage"}, {"targetLanguage"}};
    return specs;
}

static std::map<std::string, ToolSpec> parseClaudeTools(const json& j) {
    std::map<std::string, ToolSpec> result;
    for (const auto& tool : j.at("tools")) {
        ToolSpec spec;
        const auto& schema = tool.at("input_schema");
        if (schema.contains("properties")) {
            for (auto it = schema.at("properties").begin(); it != schema.at("properties").end(); ++it) {
                spec.props.insert(it.key());
            }
        }
        if (schema.contains("required")) {
            for (const auto& req : schema.at("required")) {
                spec.required.insert(req.get<std::string>());
            }
        }
        result[tool.at("name").get<std::string>()] = spec;
    }
    return result;
}

static std::map<std::string, ToolSpec> parseOpenAITools(const json& j) {
    std::map<std::string, ToolSpec> result;
    for (const auto& tool : j.at("tools")) {
        const auto& fn = tool.at("function");
        ToolSpec spec;
        const auto& schema = fn.at("parameters");
        if (schema.contains("properties")) {
            for (auto it = schema.at("properties").begin(); it != schema.at("properties").end(); ++it) {
                spec.props.insert(it.key());
            }
        }
        if (schema.contains("required")) {
            for (const auto& req : schema.at("required")) {
                spec.required.insert(req.get<std::string>());
            }
        }
        result[fn.at("name").get<std::string>()] = spec;
    }
    return result;
}

static std::map<std::string, ToolSpec> parseGenericTools(const json& j) {
    std::map<std::string, ToolSpec> result;
    for (const auto& tool : j.at("tools")) {
        ToolSpec spec;
        if (tool.contains("args")) {
            for (const auto& arg : tool.at("args")) {
                spec.props.insert(arg.at("name").get<std::string>());
                if (arg.value("required", false)) {
                    spec.required.insert(arg.at("name").get<std::string>());
                }
            }
        }
        result[tool.at("name").get<std::string>()] = spec;
    }
    return result;
}

static void assertToolCoverage(const std::map<std::string, ToolSpec>& tools,
                               const std::map<std::string, ToolSpec>& expected) {
    for (const auto& kv : expected) {
        assert(tools.count(kv.first) == 1);
    }
}

static void assertToolSchemas(const std::map<std::string, ToolSpec>& tools,
                              const std::map<std::string, ToolSpec>& expected) {
    for (const auto& kv : expected) {
        const auto& name = kv.first;
        const auto& expectedSpec = kv.second;
        const auto& actualSpec = tools.at(name);
        assertSubset(expectedSpec.props, actualSpec.props);
        assertSubset(expectedSpec.required, actualSpec.required);
        assert(expectedSpec.required.size() == actualSpec.required.size());
    }
}

static void assertPromptMentionsTools(const std::string& prompt,
                                      const std::map<std::string, ToolSpec>& expected) {
    for (const auto& kv : expected) {
        assertContains(prompt, kv.first);
    }
}

static void assertClaudeExamplesValid(const json& j, const std::map<std::string, ToolSpec>& expected) {
    for (const auto& tool : j.at("tools")) {
        const auto name = tool.at("name").get<std::string>();
        if (!tool.contains("examples")) {
            continue;
        }
        const auto& exp = expected.at(name);
        for (const auto& example : tool.at("examples")) {
            assert(example.is_object());
            for (const auto& req : exp.required) {
                assert(example.contains(req));
            }
        }
    }
}

static void assertPromptTemplateVars(const std::string& path,
                                     const std::vector<std::string>& vars) {
    const std::string content = readFile(path);
    assert(!content.empty());
    for (const auto& var : vars) {
        assertContains(content, "{{" + var + "}}");
    }
}

int main() {
    const auto expected = expectedToolSpecs();

    const json claude = readJson("../tools/claude/tools.json");
    const json openai = readJson("../tools/openai/functions.json");
    const json generic = readJson("../tools/generic/tools.json");

    const auto claudeTools = parseClaudeTools(claude);
    const auto openaiTools = parseOpenAITools(openai);
    const auto genericTools = parseGenericTools(generic);

    assertToolCoverage(claudeTools, expected);
    assertToolCoverage(openaiTools, expected);
    assertToolCoverage(genericTools, expected);

    assertToolSchemas(claudeTools, expected);
    assertToolSchemas(openaiTools, expected);
    assertToolSchemas(genericTools, expected);

    assertPromptMentionsTools(readFile("../tools/claude/system_prompt.txt"), expected);
    assertPromptMentionsTools(readFile("../tools/openai/system_prompt.txt"), expected);
    assertPromptMentionsTools(readFile("../tools/generic/system_prompt.txt"), expected);

    assertClaudeExamplesValid(claude, expected);

    assertPromptTemplateVars("../tools/prompts/annotate_module.prompt", {"module_name"});
    assertPromptTemplateVars("../tools/prompts/cross_language.prompt", {"source_language", "target_language"});
    assertPromptTemplateVars("../tools/prompts/code_review.prompt", {"module_name"});
    assertPromptTemplateVars("../tools/prompts/refactor.prompt", {"module_name", "goal"});
    assertPromptTemplateVars("../tools/prompts/security.prompt", {"module_name"});

    printf("step229_test: all assertions passed\n");
    return 0;
}
