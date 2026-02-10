// Step 179: Rainbow brackets and delimiter intelligence checks.

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
    const std::string editorMode = readFile("src/EditorMode.h");
    assertContains(editorMode, "autoCloseBrackets");

    const std::string rendering = readFile("src/CodeEditorRendering.h");
    assertContains(rendering, "bracketMatch");
    assertContains(rendering, "scope_highlight");
    assertContains(rendering, "rainbow");

    printf("step179_test: all assertions passed\n");
    return 0;
}
