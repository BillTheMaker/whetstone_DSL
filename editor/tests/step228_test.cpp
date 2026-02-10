// Step 228: Prompt engineering templates.

#include <cassert>
#include <fstream>
#include <string>
#include <vector>

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
    const std::vector<std::string> prompts = {
        "../tools/prompts/annotate_module.prompt",
        "../tools/prompts/cross_language.prompt",
        "../tools/prompts/code_review.prompt",
        "../tools/prompts/refactor.prompt",
        "../tools/prompts/security.prompt"
    };

    for (const auto& path : prompts) {
        const std::string content = readFile(path);
        assert(!content.empty());
        assertContains(content, "system:");
        assertContains(content, "user:");
        assertContains(content, "expected_tools:");
        assertContains(content, "success_criteria:");
        assertContains(content, "{{");
    }

    printf("step228_test: all assertions passed\n");
    return 0;
}
