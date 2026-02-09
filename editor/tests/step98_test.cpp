// Step 98 TDD Test: Inline annotation decorations
//
// Tests:
// 1. Render with inline annotation tags without crashing

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
    std::string text = "def foo():\n  return 1\n";
    std::vector<HighlightSpan> spans;
    CodeEditorOptions opts;
    opts.showAnnotations = true;
    opts.annotationLayout = 0;
    AnnotationMarker m;
    m.line = 0;
    m.color = IM_COL32(80, 140, 220, 255);
    m.message = "@Reclaim(Tracing)";
    std::vector<AnnotationMarker> markers = {m};
    opts.annotations = &markers;

    CodeEditorResult res = widget.render("##editor", text, spans, opts, ImVec2(300, 200), ImGui::GetFont());
    (void)res;
    ImGui::End();
    endFrame();
    std::cout << "Test 1 PASS: Render with inline annotation tags" << std::endl;
    ++passed;

    ImGui::DestroyContext();

    std::cout << "\n=== Step 98 Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
