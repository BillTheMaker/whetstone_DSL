// Step 227: Open-source model tool definitions.

#include <cassert>
#include <fstream>
#include <string>

static std::string readFile(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) return {};
    return std::string((std::istreambuf_iterator<char>(f)),
                       std::istreambuf_iterator<char>());
}

static void assertContains(const std::string& text, const std::string& needle) {
    assert(text.find(needle) != std::string::npos);
}

int main() {
    const std::string tools = readFile("../tools/generic/tools.json");
    const std::string prompt = readFile("../tools/generic/system_prompt.txt");
    const std::string adapter = readFile("../tools/generic/adapter.json");

    assertContains(tools, "whetstone_get_ast");
    assertContains(tools, "whetstone_mutate");
    assertContains(tools, "whetstone_batch_mutate");
    assertContains(tools, "whetstone_get_scope");
    assertContains(tools, "whetstone_get_call_hierarchy");
    assertContains(tools, "whetstone_suggest_annotations");
    assertContains(tools, "whetstone_apply_annotation");
    assertContains(tools, "whetstone_generate_code");
    assertContains(tools, "whetstone_run_pipeline");
    assertContains(tools, "whetstone_project_language");

    assertContains(prompt, "ReAct");
    assertContains(prompt, "<tool_call");
    assertContains(adapter, "tool_call_tag");
    assertContains(adapter, "tool_result_tag");

    printf("step227_test: all assertions passed\n");
    return 0;
}
