// Step 196: High contrast + colorblind/shape markers.

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
    const std::string theme = readFile("themes/high_contrast.json");
    const std::string settings = readFile("src/SettingsManager.h");
    const std::string settingsPanel = readFile("src/panels/SettingsPanel.h");
    const std::string widget = readFile("src/CodeEditorWidget.h");
    const std::string rendering = readFile("src/CodeEditorRendering.h");
    const std::string helpers = readFile("src/CodeEditorRenderHelpers.h");

    assertContains(theme, "\"High Contrast\"");
    assertContains(theme, "windowBorderSize");
    assertContains(theme, "frameBorderSize");

    assertContains(settings, "useAnnotationShapes");
    assertContains(settingsPanel, "Use Shapes for Annotations");
    assertContains(widget, "useAnnotationShapes");
    assertContains(rendering, "drawAnnotationShape");
    assertContains(rendering, "annotationShapeIndex");

    assertContains(helpers, "dotted");
    assertContains(helpers, "segment % 2");
    assertContains(helpers, "AddCircleFilled");

    printf("step196_test: all assertions passed\n");
    return 0;
}
