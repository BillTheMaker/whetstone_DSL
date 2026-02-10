// Step 189: Guided workflow wizard scaffolding.

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
    const std::string framework = readFile("src/WizardFramework.h");
    const std::string panels = readFile("src/panels/WizardPanels.h");

    assertContains(framework, "WizardFrame");
    assertContains(framework, "renderWizardFooter");
    assertContains(panels, "Annotate File Wizard");
    assertContains(panels, "Cross-Language Project Wizard");
    assertContains(panels, "Connect Agent Wizard");

    printf("step189_test: all assertions passed\n");
    return 0;
}
