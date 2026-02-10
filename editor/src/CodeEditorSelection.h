#pragma once
// Included inside CodeEditorWidget (selection + editing).
    bool hasSelection() const {
        return selStart_ >= 0 && selEnd_ >= 0 && selStart_ != selEnd_;
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

    void insertPair(std::string& text, char open, char close, bool& changed) {
        deleteSelection(text, changed);
        text.insert(cursor_, 1, open);
        text.insert(cursor_ + 1, 1, close);
        cursor_ += 1;
        changed = true;
    }

    static std::string lineTextAt(const std::string& text, const std::vector<int>& lineStarts, int line) {
        if (line < 0 || line >= (int)lineStarts.size()) return "";
        int start = lineStarts[line];
        int end = (line + 1 < (int)lineStarts.size()) ? lineStarts[line + 1] - 1 : (int)text.size();
        if (end < start) end = start;
        return text.substr(start, end - start);
    }

    static std::string leadingIndent(const std::string& line) {
        size_t i = 0;
        while (i < line.size() && (line[i] == ' ' || line[i] == '\t')) ++i;
        return line.substr(0, i);
    }

    void insertNewlineWithIndent(std::string& text,
                                 const std::vector<int>& lineStarts,
                                 const EditorMode* mode,
                                 bool& changed) {
        std::string indent;
        if (mode) {
            int line = lineFromPos(cursor_, lineStarts);
            std::string lineText = lineTextAt(text, lineStarts, line);
            indent = leadingIndent(lineText);
            if (mode->shouldIndentAfter(lineText)) {
                indent += mode->getIndentString();
            }
        }
        insertText(text, "\n" + indent, changed);
    }

    void handleKeyboard(std::string& text,
                        bool& changed,
                        const std::vector<int>& lineStarts,
                        const EditorMode* mode) {
        ImGuiIO& io = ImGui::GetIO();

        // Text input
        for (int n = 0; n < io.InputQueueCharacters.Size; n++) {
            ImWchar c = io.InputQueueCharacters[n];
            if (c == 0) continue;
            if (c == '\r') c = '\n';
            if (c == '\n') {
                insertNewlineWithIndent(text, lineStarts, mode, changed);
            } else if (c == '\t') {
                if (mode) insertText(text, mode->getIndentString(), changed);
                else insertText(text, "\t", changed);
            } else if (c >= 32) {
                if (mode) {
                    char close = mode->getClosingBracket((char)c);
                    if (close != 0) {
                        insertPair(text, (char)c, close, changed);
                    } else {
                        char buf[5] = {0};
                        buf[0] = (char)c;
                        insertText(text, std::string(buf), changed);
                    }
                } else {
                    char buf[5] = {0};
                    buf[0] = (char)c;
                    insertText(text, std::string(buf), changed);
                }
            }
        }
        io.InputQueueCharacters.resize(0);

        // Navigation
        auto moveCursor = [&](int newPos) {
            newPos = std::max(0, std::min(newPos, (int)text.size()));
            if (io.KeyShift) {
                if (!hasSelection()) selStart_ = cursor_;
                cursor_ = newPos;
                selEnd_ = cursor_;
            } else {
                cursor_ = newPos;
                selStart_ = selEnd_ = -1;
            }
        };

        if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow)) {
            if (!io.KeyShift && hasSelection()) {
                moveCursor(std::min(selStart_, selEnd_));
            } else if (cursor_ > 0) {
                moveCursor(cursor_ - 1);
            }
        }
        if (ImGui::IsKeyPressed(ImGuiKey_RightArrow)) {
            if (!io.KeyShift && hasSelection()) {
                moveCursor(std::max(selStart_, selEnd_));
            } else if (cursor_ < (int)text.size()) {
                moveCursor(cursor_ + 1);
            }
        }
        if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
            int curLine = lineFromPos(cursor_, lineStarts);
            if (curLine > 0) {
                int curCol = cursor_ - lineStarts[curLine];
                int prevStart = lineStarts[curLine - 1];
                int prevEnd = (curLine < (int)lineStarts.size()) ? lineStarts[curLine] - 1 : (int)text.size();
                int prevLen = std::max(0, prevEnd - prevStart);
                moveCursor(prevStart + std::min(curCol, prevLen));
            }
        }
        if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
            int curLine = lineFromPos(cursor_, lineStarts);
            if (curLine + 1 < (int)lineStarts.size()) {
                int curCol = cursor_ - lineStarts[curLine];
                int nextStart = lineStarts[curLine + 1];
                int nextEnd = (curLine + 2 < (int)lineStarts.size()) ? lineStarts[curLine + 2] - 1 : (int)text.size();
                int nextLen = std::max(0, nextEnd - nextStart);
                moveCursor(nextStart + std::min(curCol, nextLen));
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

        if ((io.KeyCtrl || io.KeySuper) && ImGui::IsKeyPressed(ImGuiKey_C)) {
            if (hasSelection()) {
                ImGui::SetClipboardText(getSelectionText(text).c_str());
            }
        }
        if ((io.KeyCtrl || io.KeySuper) && ImGui::IsKeyPressed(ImGuiKey_X)) {
            if (hasSelection()) {
                ImGui::SetClipboardText(getSelectionText(text).c_str());
                deleteSelection(text, changed);
            }
        }
        if ((io.KeyCtrl || io.KeySuper) && ImGui::IsKeyPressed(ImGuiKey_V)) {
            const char* clip = ImGui::GetClipboardText();
            if (clip && *clip) {
                insertText(text, std::string(clip), changed);
            }
        }
    }

    std::string getSelectionText(const std::string& text) const {
        if (!hasSelection()) return "";
        int a = std::min(selStart_, selEnd_);
        int b = std::max(selStart_, selEnd_);
        if (a < 0 || b > (int)text.size() || a >= b) return "";
        return text.substr(a, b - a);
    }

    void selectWordAt(const std::string& text, int pos) {
        if (pos < 0 || pos > (int)text.size()) return;
        auto isWord = [](char c) { return std::isalnum((unsigned char)c) || c == '_'; };
        int start = pos;
        int end = pos;
        while (start > 0 && isWord(text[start - 1])) --start;
        while (end < (int)text.size() && isWord(text[end])) ++end;
        selStart_ = start;
        selEnd_ = end;
        cursor_ = end;
    }
