// Step 95 TDD Test: Gutter markers and squiggles
//
// Tests:
// 1. Render with diagnostics ranges without crashing

#include <cassert>
#include <iostream>
#include "imgui.h"
#include "CodeEditorWidget.h"

static void beginFrame() {
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(800, 600);
    io.DeltaTime = 1.0f / 60.0f;
    ImGui::NewFrame();
}

static void endFrame() {
    ImGui::Render();
}

int main() {
    int passed = 0;
    int failed = 0;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontDefault();
    io.Fonts->Build();

    beginFrame();
    ImGui::SetNextWindowFocus();
    ImGui::Begin("TestWindow");
    CodeEditorWidget widget;
    std::string text = "int main() {\n  return 0;\n}\n";
    std::vector<HighlightSpan> spans;
    CodeEditorOptions opts;
    DiagnosticRange diag;
    diag.startLine = 1;
    diag.startCol = 2;
    diag.endLine = 1;
    diag.endCol = 8;
    diag.severity = 1;
    diag.message = "Example error";
    std::vector<DiagnosticRange> diags = {diag};
    opts.diagnostics = &diags;

    CodeEditorResult res = widget.render("##editor", text, spans, opts, ImVec2(300, 200), ImGui::GetFont());
    (void)res;
    ImGui::End();
    endFrame();
    std::cout << "Test 1 PASS: Render with diagnostics" << std::endl;
    ++passed;

    ImGui::DestroyContext();

    std::cout << "\n=== Step 95 Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
