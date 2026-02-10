// Step 199: Large file handling.

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
    const std::string dialogs = readFile("src/panels/DialogPanels.h");
    const std::string status = readFile("src/panels/StatusBarPanel.h");

    assertContains(settings, "largeFileWarnMB");
    assertContains(settings, "largeFileTextMB");
    assertContains(settings, "largeFileDisableHighlightMB");

    assertContains(editorState, "disableSyntaxHighlight");
    assertContains(editorState, "largeFilePrompt");
    assertContains(editorState, "setActiveBufferMode");

    assertContains(dialogs, "Large File");
    assertContains(dialogs, "Open in Text Mode");

    assertContains(status, "Large File");
    assertContains(status, "Mem ");

    printf("step199_test: all assertions passed\n");
    return 0;
}
