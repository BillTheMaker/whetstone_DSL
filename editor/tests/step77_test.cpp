// Step 77 TDD Test: Custom code editor renderer
//
// Tests:
// 1. Can render with an ImGui context (no crash)
// 2. Text input inserts characters and reports change
// 3. Cursor position updates after input

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

    // --- Test 1: Render without crashing ---
    {
        beginFrame();
        ImGui::SetNextWindowFocus();
        ImGui::Begin("TestWindow");
        CodeEditorWidget widget;
        std::string text;
        std::vector<HighlightSpan> spans;
        CodeEditorOptions opts;
        CodeEditorResult res = widget.render("##editor", text, spans, opts, ImVec2(300, 200), ImGui::GetFont());
        (void)res;
        ImGui::End();
        endFrame();
        std::cout << "Test 1 PASS: Render without crash" << std::endl;
        ++passed;
    }

    // --- Test 2: Text input inserts characters ---
    {
        beginFrame();
        ImGui::SetNextWindowFocus();
        ImGui::Begin("TestWindow2");
        CodeEditorWidget widget;
        std::string text;
        std::vector<HighlightSpan> spans;
        CodeEditorOptions opts;

        ImGuiIO& io = ImGui::GetIO();
        io.AddInputCharacter('a');
        io.AddInputCharacter('b');
        io.AddInputCharacter('\n');

        CodeEditorResult res = widget.render("##editor2", text, spans, opts, ImVec2(300, 200), ImGui::GetFont());
        ImGui::End();
        endFrame();

        assert(res.changed && "Should report changed after input");
        assert(text == "ab\n" && "Text should match input");
        std::cout << "Test 2 PASS: Text input inserts characters" << std::endl;
        ++passed;
    }

    // --- Test 3: Cursor position updates ---
    {
        beginFrame();
        ImGui::SetNextWindowFocus();
        ImGui::Begin("TestWindow3");
        CodeEditorWidget widget;
        std::string text;
        std::vector<HighlightSpan> spans;
        CodeEditorOptions opts;

        ImGuiIO& io = ImGui::GetIO();
        io.AddInputCharacter('x');
        io.AddInputCharacter('y');

        CodeEditorResult res = widget.render("##editor3", text, spans, opts, ImVec2(300, 200), ImGui::GetFont());
        ImGui::End();
        endFrame();

        assert(res.cursorByte == 2 && "Cursor should be at end of inserted text");
        std::cout << "Test 3 PASS: Cursor updates" << std::endl;
        ++passed;
    }

    ImGui::DestroyContext();

    std::cout << "\n=== Step 77 Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
