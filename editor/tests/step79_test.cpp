// Step 79 TDD Test: Auto-indent and smart editing
//
// Tests:
// 1. Enter in Python after ':' inserts newline + indent
// 2. Tab inserts spaces per EditorMode
// 3. Auto-close bracket inserts pair and places cursor inside

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

    // --- Test 1: Auto-indent after ':' in Python ---
    {
        beginFrame();
        ImGui::SetNextWindowFocus();
        ImGui::Begin("TestWindow");
        CodeEditorWidget widget;
        EditorMode mode("python");
        std::string text = "def f():";
        widget.setCursor((int)text.size());
        std::vector<HighlightSpan> spans;
        CodeEditorOptions opts;
        opts.mode = &mode;

        io.AddInputCharacter('\n');
        CodeEditorResult res = widget.render("##editor", text, spans, opts, ImVec2(300, 200), ImGui::GetFont());
        ImGui::End();
        endFrame();

        assert(res.changed && "Should change on newline");
        assert(text == "def f():\n    " && "Python indent should be 4 spaces");
        std::cout << "Test 1 PASS: Auto-indent after ':'" << std::endl;
        ++passed;
    }

    // --- Test 2: Tab inserts spaces ---
    {
        beginFrame();
        ImGui::SetNextWindowFocus();
        ImGui::Begin("TestWindow2");
        CodeEditorWidget widget;
        EditorMode mode("cpp");
        std::string text;
        widget.setCursor(0);
        std::vector<HighlightSpan> spans;
        CodeEditorOptions opts;
        opts.mode = &mode;

        io.AddInputCharacter('\t');
        CodeEditorResult res = widget.render("##editor2", text, spans, opts, ImVec2(300, 200), ImGui::GetFont());
        ImGui::End();
        endFrame();

        assert(res.changed);
        assert(text == "    " && "Tab should insert 4 spaces in C++ mode");
        std::cout << "Test 2 PASS: Tab inserts spaces" << std::endl;
        ++passed;
    }

    // --- Test 3: Auto-close bracket ---
    {
        beginFrame();
        ImGui::SetNextWindowFocus();
        ImGui::Begin("TestWindow3");
        CodeEditorWidget widget;
        EditorMode mode("python");
        std::string text;
        widget.setCursor(0);
        std::vector<HighlightSpan> spans;
        CodeEditorOptions opts;
        opts.mode = &mode;

        io.AddInputCharacter('(');
        CodeEditorResult res = widget.render("##editor3", text, spans, opts, ImVec2(300, 200), ImGui::GetFont());
        ImGui::End();
        endFrame();

        assert(res.changed);
        assert(text == "()" && "Bracket pair should be inserted");
        assert(res.cursorByte == 1 && "Cursor should be inside the pair");
        std::cout << "Test 3 PASS: Auto-close bracket" << std::endl;
        ++passed;
    }

    ImGui::DestroyContext();

    std::cout << "\n=== Step 79 Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
