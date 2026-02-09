// Step 78 TDD Test: Line numbers and gutter
//
// Tests:
// 1. Line count matches content
// 2. Gutter width grows with line count

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

static std::string makeLines(int count) {
    std::string s;
    for (int i = 0; i < count; ++i) {
        s += "line";
        s += std::to_string(i);
        s += "\n";
    }
    return s;
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

    // --- Test 1: Line count ---
    {
        beginFrame();
        ImGui::SetNextWindowFocus();
        ImGui::Begin("TestWindow");
        CodeEditorWidget widget;
        std::string text = makeLines(5);
        std::vector<HighlightSpan> spans;
        CodeEditorOptions opts;
        CodeEditorResult res = widget.render("##editor", text, spans, opts, ImVec2(300, 200), ImGui::GetFont());
        ImGui::End();
        endFrame();

        assert(res.lineCount == 5 && "Line count should match content");
        std::cout << "Test 1 PASS: Line count correct" << std::endl;
        ++passed;
    }

    // --- Test 2: Gutter width grows with line count ---
    {
        beginFrame();
        ImGui::SetNextWindowFocus();
        ImGui::Begin("TestWindow2");
        CodeEditorWidget widget;
        std::string shortText = makeLines(9);
        std::string longText = makeLines(120);
        std::vector<HighlightSpan> spans;
        CodeEditorOptions opts;

        CodeEditorResult res1 = widget.render("##editor2", shortText, spans, opts, ImVec2(300, 200), ImGui::GetFont());
        CodeEditorResult res2 = widget.render("##editor3", longText, spans, opts, ImVec2(300, 200), ImGui::GetFont());
        ImGui::End();
        endFrame();

        assert(res1.gutterWidth > 0.0f && res2.gutterWidth > 0.0f);
        assert(res2.gutterWidth > res1.gutterWidth && "Gutter should grow for more digits");
        std::cout << "Test 2 PASS: Gutter width grows" << std::endl;
        ++passed;
    }

    ImGui::DestroyContext();

    std::cout << "\n=== Step 78 Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
