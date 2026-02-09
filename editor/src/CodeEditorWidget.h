#pragma once
// Step 77: Custom code editor renderer
//
// Immediate-mode code editor widget that renders text with syntax colors,
// handles cursor/selection/input, and supports a visible whitespace toggle.
// This is intentionally minimal; line numbers, gutters, folding, and
// advanced selection behaviors are added in later steps.

#include "imgui.h"
#include "SyntaxHighlighter.h"
#include <string>
#include <vector>
#include <algorithm>

struct CodeEditorOptions {
    bool showWhitespace = false;
    bool readOnly = false;
};

struct CodeEditorResult {
    bool changed = false;
    int cursorByte = 0;
};

class CodeEditorWidget {
public:
    CodeEditorResult render(const char* id,
                             std::string& text,
                             const std::vector<HighlightSpan>& spans,
                             const CodeEditorOptions& options,
                             const ImVec2& size,
                             ImFont* font) {
        CodeEditorResult result;
        if (!font) font = ImGui::GetFont();

        // Build line start offsets
        std::vector<int> lineStarts;
        lineStarts.reserve(128);
        lineStarts.push_back(0);
        for (int i = 0; i < (int)text.size(); ++i) {
            if (text[i] == '\n') lineStarts.push_back(i + 1);
        }
        const int lineCount = (int)lineStarts.size();

        // Clamp cursor
        if (cursor_ < 0) cursor_ = 0;
        if (cursor_ > (int)text.size()) cursor_ = (int)text.size();

        // Build per-byte category map
        std::vector<TokenCategory> charCats(text.size(), TokenCategory::Plain);
        for (const auto& span : spans) {
            uint32_t end = std::min<uint32_t>(span.end, (uint32_t)charCats.size());
            for (uint32_t i = span.start; i < end; ++i) {
                charCats[i] = span.category;
            }
        }

        // Measure
        const float lineHeight = ImGui::GetTextLineHeightWithSpacing();
        const float charAdvance = font->CalcTextSizeA(font->FontSize, FLT_MAX, -1.0f, "M").x;

        // Estimate content size for scrollbars
        int maxLineLen = 0;
        for (int ln = 0; ln < lineCount; ++ln) {
            int start = lineStarts[ln];
            int end = (ln + 1 < lineCount) ? lineStarts[ln + 1] - 1 : (int)text.size();
            int len = std::max(0, end - start);
            maxLineLen = std::max(maxLineLen, len);
        }
        ImVec2 contentSize(maxLineLen * charAdvance + 4.0f, lineCount * lineHeight + 4.0f);

        ImGui::BeginChild(id, size, false, ImGuiWindowFlags_HorizontalScrollbar);
        ImVec2 origin = ImGui::GetCursorScreenPos();

        // Dummy to set scroll extents
        ImGui::Dummy(contentSize);

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        const float scrollX = ImGui::GetScrollX();
        const float scrollY = ImGui::GetScrollY();
        ImVec2 base(origin.x - scrollX, origin.y - scrollY);

        // Focus and input
        const bool hovered = ImGui::IsWindowHovered();
        const bool focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);

        // Mouse handling
        if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            ImGui::SetKeyboardFocusHere(-1);
            cursor_ = positionFromMouse(ImGui::GetMousePos(), base, lineStarts, text, charAdvance, lineHeight);
            selStart_ = cursor_;
            selEnd_ = cursor_;
            selecting_ = true;
        }
        if (hovered && selecting_ && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
            int pos = positionFromMouse(ImGui::GetMousePos(), base, lineStarts, text, charAdvance, lineHeight);
            selEnd_ = pos;
        }
        if (selecting_ && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
            selecting_ = false;
        }

        // Keyboard input
        if (focused && !options.readOnly) {
            handleKeyboard(text, result.changed, lineStarts);
        }

        // Render visible lines
        const int firstLine = std::max(0, (int)(scrollY / lineHeight));
        const int visibleLines = (int)(ImGui::GetContentRegionAvail().y / lineHeight) + 2;
        const int lastLine = std::min(lineCount - 1, firstLine + visibleLines);

        for (int ln = firstLine; ln <= lastLine; ++ln) {
            int start = lineStarts[ln];
            int end = (ln + 1 < lineCount) ? lineStarts[ln + 1] - 1 : (int)text.size();
            int len = std::max(0, end - start);

            float y = base.y + ln * lineHeight;

            // Selection background
            if (hasSelection()) {
                int selA = std::min(selStart_, selEnd_);
                int selB = std::max(selStart_, selEnd_);
                int lineSelStart = std::max(selA, start);
                int lineSelEnd = std::min(selB, end);
                if (lineSelStart < lineSelEnd) {
                    int colA = lineSelStart - start;
                    int colB = lineSelEnd - start;
                    ImVec2 a(base.x + colA * charAdvance, y);
                    ImVec2 b(base.x + colB * charAdvance, y + lineHeight);
                    drawList->AddRectFilled(a, b, IM_COL32(60, 100, 160, 120));
                }
            }

            // Render text with colors
            int pos = start;
            while (pos < end) {
                TokenCategory cat = TokenCategory::Plain;
                if (pos < (int)charCats.size()) cat = charCats[pos];
                int spanEnd = pos + 1;
                while (spanEnd < end) {
                    TokenCategory nextCat = TokenCategory::Plain;
                    if (spanEnd < (int)charCats.size()) nextCat = charCats[spanEnd];
                    if (nextCat != cat) break;
                    ++spanEnd;
                }

                std::string chunk = text.substr(pos, spanEnd - pos);
                if (options.showWhitespace) {
                    for (auto& c : chunk) {
                        if (c == ' ') c = '.';
                        else if (c == '\t') c = '>';
                    }
                }
                if (!chunk.empty() && chunk[chunk.size() - 1] == '\n') {
                    chunk.pop_back();
                }

                ImVec2 p(base.x + (pos - start) * charAdvance, y);
                drawList->AddText(font, font->FontSize, p, colorFor(cat), chunk.c_str());

                pos = spanEnd;
            }
        }

        // Cursor
        if (focused) {
            const double t = ImGui::GetTime();
            const bool blinkOn = ((int)(t * 2.0)) % 2 == 0;
            if (blinkOn) {
                int curLine = lineFromPos(cursor_, lineStarts);
                int lineStart = lineStarts[curLine];
                int col = cursor_ - lineStart;
                float x = base.x + col * charAdvance;
                float y = base.y + curLine * lineHeight;
                drawList->AddLine(ImVec2(x, y), ImVec2(x, y + lineHeight), IM_COL32(240, 240, 240, 255), 1.0f);
            }
        }

        ImGui::EndChild();

        result.cursorByte = cursor_;
        return result;
    }

    void setCursor(int pos) { cursor_ = pos; }
    int getCursor() const { return cursor_; }

private:
    int cursor_ = 0;
    int selStart_ = -1;
    int selEnd_ = -1;
    bool selecting_ = false;

    bool hasSelection() const {
        return selStart_ >= 0 && selEnd_ >= 0 && selStart_ != selEnd_;
    }

    static ImU32 colorFor(TokenCategory cat) {
        switch (cat) {
            case TokenCategory::Keyword:     return IM_COL32(196, 126, 220, 255);
            case TokenCategory::String:      return IM_COL32(207, 138, 94, 255);
            case TokenCategory::Comment:     return IM_COL32(107, 133, 89, 255);
            case TokenCategory::Number:      return IM_COL32(181, 204, 140, 255);
            case TokenCategory::Function:    return IM_COL32(220, 220, 140, 255);
            case TokenCategory::Parameter:   return IM_COL32(153, 199, 229, 255);
            case TokenCategory::Type:        return IM_COL32(77, 179, 174, 255);
            case TokenCategory::Operator:    return IM_COL32(220, 220, 220, 255);
            case TokenCategory::Punctuation: return IM_COL32(160, 160, 160, 255);
            case TokenCategory::Builtin:     return IM_COL32(77, 179, 229, 255);
            case TokenCategory::Identifier:  return IM_COL32(160, 200, 230, 255);
            default:                         return IM_COL32(220, 220, 220, 255);
        }
    }

    static int lineFromPos(int pos, const std::vector<int>& lineStarts) {
        if (lineStarts.empty()) return 0;
        int line = 0;
        for (int i = 0; i < (int)lineStarts.size(); ++i) {
            if (lineStarts[i] <= pos) line = i;
            else break;
        }
        return line;
    }

    static int positionFromMouse(const ImVec2& mouse,
                                 const ImVec2& base,
                                 const std::vector<int>& lineStarts,
                                 const std::string& text,
                                 float charAdvance,
                                 float lineHeight) {
        int line = (int)((mouse.y - base.y) / lineHeight);
        line = std::max(0, std::min(line, (int)lineStarts.size() - 1));

        int start = lineStarts[line];
        int end = (line + 1 < (int)lineStarts.size()) ? lineStarts[line + 1] - 1 : (int)text.size();
        int len = std::max(0, end - start);

        int col = (int)((mouse.x - base.x) / charAdvance);
        col = std::max(0, std::min(col, len));
        return start + col;
    }

    void deleteSelection(std::string& text, bool& changed) {
        if (!hasSelection()) return;
        int a = std::min(selStart_, selEnd_);
        int b = std::max(selStart_, selEnd_);
        if (a >= 0 && b <= (int)text.size() && a < b) {
            text.erase(a, b - a);
            cursor_ = a;
            changed = true;
        }
        selStart_ = selEnd_ = -1;
    }

    void insertText(std::string& text, const std::string& insert, bool& changed) {
        if (insert.empty()) return;
        deleteSelection(text, changed);
        text.insert(cursor_, insert);
        cursor_ += (int)insert.size();
        changed = true;
    }

    void handleKeyboard(std::string& text, bool& changed, const std::vector<int>& lineStarts) {
        ImGuiIO& io = ImGui::GetIO();

        // Text input
        for (int n = 0; n < io.InputQueueCharacters.Size; n++) {
            ImWchar c = io.InputQueueCharacters[n];
            if (c == 0) continue;
            if (c == '\r') c = '\n';
            if (c == '\n') {
                insertText(text, "\n", changed);
            } else if (c == '\t') {
                insertText(text, "\t", changed);
            } else if (c >= 32) {
                char buf[5] = {0};
                buf[0] = (char)c;
                insertText(text, std::string(buf), changed);
            }
        }
        io.InputQueueCharacters.resize(0);

        // Navigation
        if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow)) {
            if (hasSelection()) {
                cursor_ = std::min(selStart_, selEnd_);
                selStart_ = selEnd_ = -1;
            } else if (cursor_ > 0) {
                cursor_--;
            }
        }
        if (ImGui::IsKeyPressed(ImGuiKey_RightArrow)) {
            if (hasSelection()) {
                cursor_ = std::max(selStart_, selEnd_);
                selStart_ = selEnd_ = -1;
            } else if (cursor_ < (int)text.size()) {
                cursor_++;
            }
        }
        if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
            int curLine = lineFromPos(cursor_, lineStarts);
            if (curLine > 0) {
                int curCol = cursor_ - lineStarts[curLine];
                int prevStart = lineStarts[curLine - 1];
                int prevEnd = (curLine < (int)lineStarts.size()) ? lineStarts[curLine] - 1 : (int)text.size();
                int prevLen = std::max(0, prevEnd - prevStart);
                cursor_ = prevStart + std::min(curCol, prevLen);
            }
        }
        if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
            int curLine = lineFromPos(cursor_, lineStarts);
            if (curLine + 1 < (int)lineStarts.size()) {
                int curCol = cursor_ - lineStarts[curLine];
                int nextStart = lineStarts[curLine + 1];
                int nextEnd = (curLine + 2 < (int)lineStarts.size()) ? lineStarts[curLine + 2] - 1 : (int)text.size();
                int nextLen = std::max(0, nextEnd - nextStart);
                cursor_ = nextStart + std::min(curCol, nextLen);
            }
        }

        // Editing keys
        if (ImGui::IsKeyPressed(ImGuiKey_Backspace) && !text.empty()) {
            if (hasSelection()) {
                deleteSelection(text, changed);
            } else if (cursor_ > 0) {
                text.erase(cursor_ - 1, 1);
                cursor_--;
                changed = true;
            }
        }
        if (ImGui::IsKeyPressed(ImGuiKey_Delete) && !text.empty()) {
            if (hasSelection()) {
                deleteSelection(text, changed);
            } else if (cursor_ < (int)text.size()) {
                text.erase(cursor_, 1);
                changed = true;
            }
        }

        if ((io.KeyCtrl || io.KeySuper) && ImGui::IsKeyPressed(ImGuiKey_A)) {
            selStart_ = 0;
            selEnd_ = (int)text.size();
            cursor_ = selEnd_;
        }
    }
};
