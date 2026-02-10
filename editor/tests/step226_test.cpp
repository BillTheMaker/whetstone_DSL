// Step 226: OpenAI tool definitions.

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
    const std::string tools = readFile("../tools/openai/functions.json");
    const std::string prompt = readFile("../tools/openai/system_prompt.txt");
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
    assertContains(prompt, "Whetstone coding assistant");
    printf("step226_test: all assertions passed\n");
    return 0;
}
