// Step 82 TDD Test: Minimap
//
// Tests:
// 1. Minimap width reported when enabled

#include <cassert>
#include <iostream>
#include "imgui.h"
#include "CodeEditorWidget.h"
#include "EditorMode.h"

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

    // --- Test 1: Minimap enabled reports width ---
    {
        beginFrame();
        ImGui::SetNextWindowFocus();
        ImGui::Begin("TestWindow");
        CodeEditorWidget widget;
        EditorMode mode("python");
        std::string text = "def f():\n    x = 1\n    y = 2\n";
        std::vector<HighlightSpan> spans;
        CodeEditorOptions opts;
        opts.mode = &mode;
        opts.enableFolding = true;
        opts.showMinimap = true;

        CodeEditorResult res = widget.render("##editor", text, spans, opts, ImVec2(300, 200), ImGui::GetFont());
        ImGui::End();
        endFrame();

        assert(res.minimapEnabled);
        assert(res.minimapWidth > 0.0f);
        std::cout << "Test 1 PASS: Minimap width reported" << std::endl;
        ++passed;
    }

    ImGui::DestroyContext();

    std::cout << "\n=== Step 82 Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
