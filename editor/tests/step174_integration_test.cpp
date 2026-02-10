// Step 174: Icon system integration checks.

#include <cassert>

#include "IconSet.h"

int main() {
    ImGui::CreateContext();
    ImU32 color = IconSet::fileColor(FileIconType::Python);
    ImU32 warn = IconSet::diagnosticColor(DiagnosticIconType::Warning);
    assert(color != 0);
    assert(warn != 0);
    ImGui::DestroyContext();

    printf("step174_integration_test: all assertions passed\n");
    return 0;
}
