// Step 81 TDD Test: Code folding
//
// Tests:
// 1. Fold regions detected for Python function
// 2. Toggle fold updates fold state

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

    // --- Test 1: Fold region detection ---
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

        CodeEditorResult res = widget.render("##editor", text, spans, opts, ImVec2(300, 200), ImGui::GetFont());
        (void)res;
        ImGui::End();
        endFrame();

        const auto& folds = widget.getFoldRegions();
        assert(!folds.empty() && "Should detect fold region");
        std::cout << "Test 1 PASS: Fold region detected" << std::endl;
        ++passed;
    }

    // --- Test 2: Toggle fold ---
    {
        beginFrame();
        ImGui::SetNextWindowFocus();
        ImGui::Begin("TestWindow2");
        CodeEditorWidget widget;
        EditorMode mode("python");
        std::string text = "def f():\n    x = 1\n    y = 2\n";
        std::vector<HighlightSpan> spans;
        CodeEditorOptions opts;
        opts.mode = &mode;
        opts.enableFolding = true;

        widget.render("##editor2", text, spans, opts, ImVec2(300, 200), ImGui::GetFont());
        widget.toggleFoldAtLine(0);
        ImGui::End();
        endFrame();

        const auto& folds = widget.getFoldRegions();
        assert(!folds.empty());
        assert(folds[0].folded && "Fold should be toggled on");
        std::cout << "Test 2 PASS: Fold toggled" << std::endl;
        ++passed;
    }

    ImGui::DestroyContext();

    std::cout << "\n=== Step 81 Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
