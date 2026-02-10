// Step 167: Split EditorState into sub-state structs.
// Verify: state headers exist, EditorState uses sub-states, panels reference new fields.

#include <cassert>
#include <filesystem>
#include <fstream>
#include <string>

#include "state/SearchState.h"
#include "state/AgentState.h"
#include "state/BuildState.h"
#include "state/LibraryState.h"
#include "state/EmacsState.h"
#include "state/UIFlags.h"

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

static void assertNotContains(const std::string& text, const std::string& needle) {
    assert(text.find(needle) == std::string::npos);
}

int main() {
    // 1) State headers exist.
    const char* stateHeaders[] = {
        "src/state/SearchState.h",
        "src/state/AgentState.h",
        "src/state/BuildState.h",
        "src/state/LibraryState.h",
        "src/state/EmacsState.h",
        "src/state/UIFlags.h",
    };
    for (const char* p : stateHeaders) {
        assert(fs::exists(p) && "State header must exist");
    }

    // 2) EditorState uses sub-state fields.
    SearchState search;
    AgentState agent;
    BuildState build;
    LibraryState library;
    EmacsState emacs;
    UIFlags ui;
    search.showFind = true;
    agent.port = 1234;
    build.showTerminalPanel = true;
    library.showLibraryBrowserPanel = true;
    emacs.showEmacsPackagesPanel = true;
    ui.showMinimap = true;

    // 3) EditorState header reflects new sub-state fields.
    const std::string editorState = readFile("src/EditorState.h");
    assertContains(editorState, "SearchState");
    assertContains(editorState, "AgentState");
    assertContains(editorState, "BuildState");
    assertContains(editorState, "LibraryState");
    assertContains(editorState, "EmacsState");
    assertContains(editorState, "UIFlags");
    assertNotContains(editorState, "ProjectSearch     projectSearch");

    // 4) Panels reference sub-states (spot check).
    const std::string searchPanels = readFile("src/panels/SearchPanels.h");
    assertContains(searchPanels, "state.search");
    const std::string sidePanels = readFile("src/panels/SidePanels.h");
    assertContains(sidePanels, "state.library");
    assertContains(sidePanels, "state.emacsState");
    const std::string statusBar = readFile("src/panels/StatusBarPanel.h");
    assertContains(statusBar, "state.build");
    assertContains(statusBar, "state.ui");

    printf("step167_test: all assertions passed\n");
    return 0;
}
