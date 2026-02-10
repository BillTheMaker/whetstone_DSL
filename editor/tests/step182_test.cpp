// Step 182: Improved diagnostics UI checks.

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
    const std::string bottom = readFile("src/panels/BottomPanel.h");
    assertContains(bottom, "Sort By");
    assertContains(bottom, "Fix All");
    assertContains(bottom, "diagTable");

    const std::string editorPanel = readFile("src/panels/EditorPanel.h");
    assertContains(editorPanel, "diagCounts");

    printf("step182_test: all assertions passed\n");
    return 0;
}
