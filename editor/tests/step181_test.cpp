// Step 181: Enhanced status bar checks.

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
    const std::string status = readFile("src/panels/StatusBarPanel.h");
    assertContains(status, "Git");
    assertContains(status, "notifications");
    assertContains(status, "UTF-8");

    printf("step181_test: all assertions passed\n");
    return 0;
}
