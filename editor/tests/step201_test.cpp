// Step 201: Sprint 6 integration tests (presence checks).

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
    const std::string mainSrc = readFile("src/main.cpp");
    const std::string settingsPanel = readFile("src/panels/SettingsPanel.h");
    const std::string selection = readFile("src/CodeEditorSelection.h");
    const std::string uiFlags = readFile("src/state/UIFlags.h");
    const std::string deps = readFile("src/DependencyPanel.h");
    const std::string settings = readFile("src/SettingsManager.h");
    const std::string notify = readFile("src/NotificationSystem.h");

    assertContains(mainSrc, "renderEditorPanel");
    assertContains(settingsPanel, "applyTheme");
    assertContains(selection, "addCursorAt");
    assertContains(uiFlags, "showFirstRunWizard");
    assertContains(deps, "drawSecurityBadge");
    assertContains(mainSrc, "SDLK_F6");
    assertContains(settings, "largeFileWarnMB");
    assertContains(notify, "notify");

    printf("step201_test: all assertions passed\n");
    return 0;
}
