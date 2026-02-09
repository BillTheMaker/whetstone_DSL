// Step 80 TDD Test: Scroll, selection, clipboard
//
// Tests:
// 1. Ctrl+A then Ctrl+C copies full buffer to clipboard
// 2. Ctrl+X cuts selection to clipboard
// 3. Ctrl+V pastes clipboard at cursor

#include <cassert>
#include <iostream>
#include <string>
#include "imgui.h"
#include "CodeEditorWidget.h"
#include "EditorMode.h"

static std::string g_clipboard;

static void SetClipboardText(void*, const char* text) {
    g_clipboard = text ? text : "";
}

static const char* GetClipboardText(void*) {
    return g_clipboard.c_str();
}

static void beginFrame() {
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(800, 600);
    io.DeltaTime = 1.0f / 60.0f;
    ImGui::NewFrame();
}

static void endFrame() {
    ImGui::Render();
}

static void queueCtrlKey(ImGuiKey key) {
    ImGuiIO& io = ImGui::GetIO();
    io.AddKeyEvent(ImGuiKey_LeftCtrl, true);
    io.AddKeyEvent(key, true);
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
    io.SetClipboardTextFn = SetClipboardText;
    io.GetClipboardTextFn = GetClipboardText;

    // --- Test 1: Ctrl+A then Ctrl+C copies full text ---
    {
        g_clipboard.clear();
        beginFrame();
        ImGui::SetNextWindowFocus();
        ImGui::Begin("TestWindow");
        CodeEditorWidget widget;
        std::string text = "abc";
        std::vector<HighlightSpan> spans;
        CodeEditorOptions opts;

        queueCtrlKey(ImGuiKey_A);
        queueCtrlKey(ImGuiKey_C);

        CodeEditorResult res = widget.render("##editor", text, spans, opts, ImVec2(300, 200), ImGui::GetFont());
        (void)res;
        ImGui::End();
        endFrame();

        assert(g_clipboard == "abc" && "Clipboard should contain full text");
        std::cout << "Test 1 PASS: Ctrl+A/C copies text" << std::endl;
        ++passed;
    }

    // --- Test 2: Ctrl+X cuts selection ---
    {
        g_clipboard.clear();
        beginFrame();
        ImGui::SetNextWindowFocus();
        ImGui::Begin("TestWindow2");
        CodeEditorWidget widget;
        std::string text = "hello";
        std::vector<HighlightSpan> spans;
        CodeEditorOptions opts;

        queueCtrlKey(ImGuiKey_A);
        queueCtrlKey(ImGuiKey_X);

        CodeEditorResult res = widget.render("##editor2", text, spans, opts, ImVec2(300, 200), ImGui::GetFont());
        (void)res;
        ImGui::End();
        endFrame();

        assert(g_clipboard == "hello" && "Clipboard should contain cut text");
        assert(text.empty() && "Text should be empty after cut");
        std::cout << "Test 2 PASS: Ctrl+X cuts selection" << std::endl;
        ++passed;
    }

    // --- Test 3: Ctrl+V pastes clipboard ---
    {
        g_clipboard = "zz";
        beginFrame();
        ImGui::SetNextWindowFocus();
        ImGui::Begin("TestWindow3");
        CodeEditorWidget widget;
        std::string text = "";
        std::vector<HighlightSpan> spans;
        CodeEditorOptions opts;
        widget.setCursor(0);

        queueCtrlKey(ImGuiKey_V);

        CodeEditorResult res = widget.render("##editor3", text, spans, opts, ImVec2(300, 200), ImGui::GetFont());
        (void)res;
        ImGui::End();
        endFrame();

        assert(text == "zz" && "Paste should insert clipboard contents");
        std::cout << "Test 3 PASS: Ctrl+V pastes" << std::endl;
        ++passed;
    }

    ImGui::DestroyContext();

    std::cout << "\n=== Step 80 Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
