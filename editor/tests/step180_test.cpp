// Step 180: Rich tooltip system checks.

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
    const std::string tooltip = readFile("src/RichTooltip.h");
    assertContains(tooltip, "renderRichTooltip");
    assertContains(tooltip, "Pin");

    const std::string editorPanel = readFile("src/panels/EditorPanel.h");
    assertContains(editorPanel, "renderRichTooltip");

    printf("step180_test: all assertions passed\n");
    return 0;
}
