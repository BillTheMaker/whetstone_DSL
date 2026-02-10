// Step 192: Security diagnostics integration.

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
    const std::string editorState = readFile("src/EditorState.h");
    const std::string settingsPanel = readFile("src/panels/SettingsPanel.h");
    const std::string registry = readFile("src/PrimitivesRegistry.h");
    const std::string rendering = readFile("src/CodeEditorRendering.h");

    assertContains(editorState, "appendVulnerabilityDiagnostics");
    assertContains(settingsPanel, "Block Vulnerable Imports");
    assertContains(registry, "vulnerableLibraries");
    assertContains(rendering, "security_gutter_");

    printf("step192_test: all assertions passed\n");
    return 0;
}
