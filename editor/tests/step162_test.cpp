// Step 162 TDD Test: Help panel section parsing
#include "HelpPanel.h"
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

    std::string text =
        "# Intro\n"
        "Hello.\n"
        "# Usage\n"
        "Details.\n";
    auto sections = parseHelpSections(text);
    expect(sections.size() == 2, "section count", passed, failed);
    expect(sections[0].title == "Intro", "first title", passed, failed);
    expect(sections[1].title == "Usage", "second title", passed, failed);
    expect(sections[0].content.find("Hello") != std::string::npos,
           "content captured", passed, failed);

    std::cout << "\n=== Step 162 Results: " << passed << " passed, "
              << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
