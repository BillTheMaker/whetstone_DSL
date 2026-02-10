// Step 146 TDD Test: Emacs-Whetstone bridge
#include "EmacsIntegration.h"
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
    std::string path = "C:/tmp/example.el";

    std::string text = mock.getBufferText(path);
    expect(text == "mock-buffer", "buffer text pulled", passed, failed);

    bool setOk = mock.setBufferText(path, "(message \"hi\")");
    expect(setOk, "buffer text pushed", passed, failed);
    expect(mock.getLastSentCommand().find("erase-buffer") != std::string::npos,
           "replace buffer command", passed, failed);

    bool frameOk = mock.openFileInEmacsFrame(path);
    expect(frameOk, "open emacs frame", passed, failed);
    expect(mock.getLastRunCommand().find("emacsclient") != std::string::npos,
           "emacsclient command", passed, failed);

    std::cout << "\n=== Step 146 Results: " << passed << " passed, "
              << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
