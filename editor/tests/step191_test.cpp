// Step 191: Dependency security badges.

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
    const std::string depPanel = readFile("src/DependencyPanel.h");
    assertContains(depPanel, "VulnerabilityDatabase");
    assertContains(depPanel, "Upgrade to Safe Version");
    assertContains(depPanel, "vuln_ignore.json");
    assertContains(depPanel, "drawSecurityBadge");

    printf("step191_test: all assertions passed\n");
    return 0;
}
