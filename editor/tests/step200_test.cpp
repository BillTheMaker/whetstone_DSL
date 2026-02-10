// Step 200: Startup and panel performance.

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
    const std::string settings = readFile("src/SettingsManager.h");
    const std::string editorState = readFile("src/EditorState.h");
    const std::string mainSrc = readFile("src/main.cpp");
    const std::string editorPanel = readFile("src/panels/EditorPanel.h");

    assertContains(settings, "getLspDebounceMs");
    assertContains(settings, "getDiagnosticsDebounceMs");
    assertContains(settings, "getHighlightDebounceMs");

    assertContains(editorState, "flushDeferredAstSync");
    assertContains(editorState, "flushLspDidChange");
    assertContains(editorState, "pendingAstSync");

    assertContains(editorPanel, "getDiagnosticsDebounceMs");
    assertContains(mainSrc, "Frame time exceeded 16ms");

    printf("step200_test: all assertions passed\n");
    return 0;
}
