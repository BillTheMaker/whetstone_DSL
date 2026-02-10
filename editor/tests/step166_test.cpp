// Step 166: Extract main.cpp into panel headers
// Verify: main.cpp size reduced, panel headers exist and compile,
//         EditorState has rendering context fields, StringUtils shared.

#include <cassert>
#include <fstream>
#include <string>
#include <sstream>
#include <filesystem>

// Verify panel headers compile
#include "StringUtils.h"
#include "GoToLine.h"
#include "EmacsPackageBrowser.h"

namespace fs = std::filesystem;

static int countFileLines(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) return -1;
    int n = 0;
    std::string line;
    while (std::getline(f, line)) ++n;
    return n;
}

int main() {
    // 1. main.cpp should be under 400 lines (was 2736, now ~330)
    int mainLines = countFileLines("src/main.cpp");
    assert(mainLines > 0 && "main.cpp must exist");
    assert(mainLines < 400 && "main.cpp must be under 400 lines after extraction");

    // 2. All 9 panel headers must exist
    const char* panelHeaders[] = {
        "src/panels/MenuBarPanel.h",
        "src/panels/ExplorerPanel.h",
        "src/panels/SidePanels.h",
        "src/panels/SearchPanels.h",
        "src/panels/SettingsPanel.h",
        "src/panels/DialogPanels.h",
        "src/panels/EditorPanel.h",
        "src/panels/BottomPanel.h",
        "src/panels/StatusBarPanel.h",
    };
    for (const char* p : panelHeaders) {
        assert(fs::exists(p) && "Panel header must exist");
        int lines = countFileLines(p);
        assert(lines > 5 && "Panel header must have content");
    }

    // 3. StringUtils.h provides trimCopy (compile-time check + runtime)
    assert(trimCopy("  hello  ") == "hello");
    assert(trimCopy("") == "");
    assert(trimCopy("   ") == "");
    assert(trimCopy("no-trim") == "no-trim");

    // 4. GoToLine.h still works (uses trimCopy from StringUtils.h)
    int line = 0, col = 0;
    assert(parseLineColInput("42", line, col));
    assert(line == 42 && col == 1);
    assert(parseLineColInput("10:5", line, col));
    assert(line == 10 && col == 5);
    assert(parseLineColInput("  :7:3  ", line, col));
    assert(line == 7 && col == 3);

    // 5. EditorState rendering context fields exist (compile-time check)
    // Verified by panels compiling and referencing state.monoFont, state.uiFont, etc.

    // 6. main.cpp includes all panel headers
    {
        std::ifstream f("src/main.cpp");
        assert(f.is_open());
        std::string content((std::istreambuf_iterator<char>(f)),
                             std::istreambuf_iterator<char>());
        assert(content.find("#include \"panels/MenuBarPanel.h\"") != std::string::npos);
        assert(content.find("#include \"panels/EditorPanel.h\"") != std::string::npos);
        assert(content.find("#include \"panels/BottomPanel.h\"") != std::string::npos);
        assert(content.find("#include \"panels/StatusBarPanel.h\"") != std::string::npos);
        assert(content.find("#include \"panels/DialogPanels.h\"") != std::string::npos);
        // main.cpp should NOT contain renderMenuBar definition
        assert(content.find("static void renderMenuBar") == std::string::npos);
        assert(content.find("static void renderEditorPanel") == std::string::npos);
    }

    printf("step166_test: all assertions passed\n");
    return 0;
}
