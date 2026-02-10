// Step 144 TDD Test: Emacs keybinding deep integration
#include "EmacsKeybinding.h"
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

    MockEmacsConnection mock;
    EmacsKeybindingState state;
    std::string log;

    bool prefixOk = emacsHandleKeySequence(state, mock, "C-x", log);
    expect(prefixOk, "prefix handled", passed, failed);
    expect(state.prefix == "C-x", "prefix stored", passed, failed);

    bool seqOk = emacsHandleKeySequence(state, mock, "C-s", log);
    expect(seqOk, "sequence handled", passed, failed);
    expect(state.prefix.empty(), "prefix cleared", passed, failed);
    expect(state.lastCommand == "save-buffer", "command resolved", passed, failed);
    expect(mock.getLastSentCommand().find("call-interactively") != std::string::npos,
           "call-interactively sent", passed, failed);

    bool mxOk = emacsHandleKeySequence(state, mock, "M-x", log);
    expect(mxOk, "M-x handled", passed, failed);
    expect(state.minibufferActive, "minibuffer active", passed, failed);

    std::snprintf(state.minibufferBuf, sizeof(state.minibufferBuf), "find-file");
    bool mbOk = emacsExecuteMinibuffer(state, mock, log);
    expect(mbOk, "minibuffer command executed", passed, failed);
    expect(!state.minibufferActive, "minibuffer cleared", passed, failed);

    updateEmacsModeLine(state, mock, 2.0, log);
    expect(!state.modeLine.empty(), "mode line updated", passed, failed);

    std::cout << "\n=== Step 144 Results: " << passed << " passed, "
              << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
