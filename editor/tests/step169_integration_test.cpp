// Step 169: Notification system integration wiring checks.

#include <cassert>
#include <filesystem>
#include <fstream>
#include <string>

namespace fs = std::filesystem;

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
    assert(fs::exists("src/NotificationSystem.h"));

    const std::string editorState = readFile("src/EditorState.h");
    assertContains(editorState, "NotificationSystem");
    assertContains(editorState, "notifications");

    const std::string statusBar = readFile("src/panels/StatusBarPanel.h");
    assertContains(statusBar, "Notifications");
    assertContains(statusBar, "state.notifications");

    const std::string mainCpp = readFile("src/main.cpp");
    assertContains(mainCpp, "renderHistoryWindow");
    assertContains(mainCpp, "renderToasts");

    const std::string bottomPanel = readFile("src/panels/BottomPanel.h");
    assertContains(bottomPanel, "state.notifications");

    printf("step169_integration_test: all assertions passed\n");
    return 0;
}
