// Step 117 TDD Test: Session persistence
#include "SessionManager.h"
#include <iostream>

static void expect(bool cond, const std::string& name, int& passed, int& failed) {
    if (cond) {
        std::cout << "Test " << (passed + failed + 1) << " PASS: " << name << "\n";
        ++passed;
    } else {
        std::cout << "Test " << (passed + failed + 1) << " FAIL: " << name << "\n";
        ++failed;
    }
}

int main() {
    int passed = 0;
    int failed = 0;

    SessionData session;
    session.workspaceRoot = "C:/repo";
    session.activePath = "C:/repo/main.py";
    session.layoutPreset = "JetBrains";
    session.imguiIni = "[Window][Editor]";
    SessionBufferState buf;
    buf.path = "C:/repo/main.py";
    buf.language = "python";
    buf.mode = "structured";
    buf.cursorLine = 12;
    buf.cursorCol = 5;
    buf.foldedLines = {1, 10};
    session.buffers.push_back(buf);

    auto j = sessionToJson(session);
    SessionData loaded = sessionFromJson(j);

    expect(loaded.workspaceRoot == "C:/repo", "workspace root", passed, failed);
    expect(loaded.activePath == "C:/repo/main.py", "active path", passed, failed);
    expect(loaded.layoutPreset == "JetBrains", "layout preset", passed, failed);
    expect(loaded.buffers.size() == 1 && loaded.buffers[0].cursorLine == 12, "cursor", passed, failed);
    expect(loaded.buffers[0].foldedLines.size() == 2, "folded lines", passed, failed);

    std::cout << "\n=== Step 117 Results: " << passed << " passed, " << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
