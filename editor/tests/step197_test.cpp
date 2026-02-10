// Step 197: Keyboard navigation completeness audit.

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

static void assertNotContains(const std::string& text, const std::string& needle) {
    assert(text.find(needle) == std::string::npos);
}

int main() {
    const std::string uiFlags = readFile("src/state/UIFlags.h");
    const std::string editorState = readFile("src/EditorState.h");
    const std::string mainSrc = readFile("src/main.cpp");
    const std::string theme = readFile("src/ThemeEngine.h");
    const std::string explorer = readFile("src/panels/ExplorerPanel.h");
    const std::string editor = readFile("src/panels/EditorPanel.h");
    const std::string bottom = readFile("src/panels/BottomPanel.h");
    const std::string side = readFile("src/panels/SidePanels.h");

    assertContains(uiFlags, "FocusRegion");
    assertContains(uiFlags, "focusTarget");
    assertContains(editorState, "cyclePanelFocus");
    assertContains(mainSrc, "SDLK_F6");
    assertContains(mainSrc, "SDLK_ESCAPE");
    assertContains(theme, "NavHighlight");
    assertContains(theme, "NavWindowingHighlight");

    assertContains(explorer, "SetNextWindowFocus");
    assertContains(editor, "SetNextWindowFocus");
    assertContains(bottom, "SetNextWindowFocus");
    assertContains(side, "SetNextWindowFocus");
    assertNotContains(side, "NoNavFocus");

    printf("step197_test: all assertions passed\n");
    return 0;
}
