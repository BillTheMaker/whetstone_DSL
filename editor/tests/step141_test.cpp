// Step 141 TDD Test: Emacs daemon with user config path
#include "EmacsIntegration.h"
#include <iostream>
#include <filesystem>

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

    // resolveEmacsInitPath uses directory -> init.el
    std::filesystem::path dir = std::filesystem::current_path() / "emacs_cfg_tmp";
    std::filesystem::create_directories(dir);
    std::string resolved = resolveEmacsInitPath(dir.string());
    expect(resolved.find("init.el") != std::string::npos, "resolve init.el", passed, failed);

    // Mock emacs daemon startup with init path
    MockEmacsConnection mock;
    std::string log;
    bool ok = mock.startDaemonWithConfig("C:/init.el", log);
    expect(ok, "mock start daemon ok", passed, failed);
    expect(mock.getLastInitPath() == "C:/init.el", "init path passed", passed, failed);
    expect(log.find("Loaded") != std::string::npos, "startup log returned", passed, failed);

    std::filesystem::remove_all(dir);

    std::cout << "\n=== Step 141 Results: " << passed << " passed, " << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
