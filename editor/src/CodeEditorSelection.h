#pragma once
// Included inside CodeEditorWidget (selection + editing).
    struct MultiCursor {
        int cursor = 0;
        int selStart = -1;
        int selEnd = -1;
    };

    void syncPrimaryToMulti() {
        if (cursors_.empty()) {
            cursors_.push_back({cursor_, selStart_, selEnd_});
            return;
        }
        cursors_[0].cursor = cursor_;
        cursors_[0].selStart = selStart_;
        cursors_[0].selEnd = selEnd_;
    }

    void syncMultiToPrimary() {
        if (cursors_.empty()) return;
        cursor_ = cursors_[0].cursor;
        selStart_ = cursors_[0].selStart;
        selEnd_ = cursors_[0].selEnd;
    }

    void clearExtraCursors() {
        if (cursors_.size() > 1) {
            cursors_.erase(cursors_.begin() + 1, cursors_.end());
        }
    }

    void addCursorAt(int pos, int selStart = -1, int selEnd = -1) {
        for (const auto& c : cursors_) {
            if (c.cursor == pos) return;
        }
        MultiCursor c;
        c.cursor = pos;
        c.selStart = selStart >= 0 ? selStart : pos;
        c.selEnd = selEnd >= 0 ? selEnd : pos;
        cursors_.push_back(c);
    }

    void dedupeCursors() {
        if (cursors_.size() <= 1) return;
        std::sort(cursors_.begin(), cursors_.end(),
                  [](const MultiCursor& a, const MultiCursor& b) { return a.cursor < b.cursor; });
        auto it = std::unique(cursors_.begin(), cursors_.end(),
                              [](const MultiCursor& a, const MultiCursor& b) {
                                  return a.cursor == b.cursor &&
                                         a.selStart == b.selStart &&
                                         a.selEnd == b.selEnd;
                              });
        cursors_.erase(it, cursors_.end());
    }

    static int positionFromLineCol(const std::vector<int>& lineStarts, int line, int col, const std::string& text) {
        if (lineStarts.empty()) return 0;
        line = std::max(0, std::min(line, (int)lineStarts.size() - 1));
        int start = lineStarts[line];
        int end = (line + 1 < (int)lineStarts.size()) ? lineStarts[line + 1] - 1 : (int)text.size();
        int len = std::max(0, end - start);
        int clampedCol = std::max(0, std::min(col, len));
        return start + clampedCol;
    }

    void updateColumnSelection(const std::string& text,
                               const std::vector<int>& lineStarts,
                               int anchorLine,
                               int anchorCol,
                               int currentLine,
                               int currentCol) {
        cursors_.clear();
        int startLine = std::min(anchorLine, currentLine);
        int endLine = std::max(anchorLine, currentLine);
        int col = currentCol;
        for (int line = startLine; line <= endLine; ++line) {
            int pos = positionFromLineCol(lineStarts, line, col, text);
            MultiCursor c;
            c.cursor = pos;
            c.selStart = pos;
            c.selEnd = pos;
            cursors_.push_back(c);
        }
        if (!cursors_.empty()) {
            cursor_ = cursors_[0].cursor;
            selStart_ = cursors_[0].selStart;
            selEnd_ = cursors_[0].selEnd;
        }
    }
    bool hasSelection() const {
        return selStart_ >= 0 && selEnd_ >= 0 && selStart_ != selEnd_;
    }

    static bool cursorHasSelection(const MultiCursor& c) {
        return c.selStart >= 0 && c.selEnd >= 0 && c.selStart != c.selEnd;
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
        syncPrimaryToMulti();
    }

    void applyEdits(std::string& text,
                    const std::vector<MultiCursor>& edits,
                    const std::vector<std::string>& inserts,
                    bool& changed) {
        if (edits.empty()) return;
        struct Plan { int index; int start; int end; std::string insert; };
        std::vector<Plan> plans;
        plans.reserve(edits.size());
        for (size_t i = 0; i < edits.size(); ++i) {
            const auto& c = edits[i];
            int start = c.cursor;
            int end = c.cursor;
            if (cursorHasSelection(c)) {
                start = std::min(c.selStart, c.selEnd);
                end = std::max(c.selStart, c.selEnd);
            }
            plans.push_back({(int)i, start, end, inserts[i]});
        }
        std::sort(plans.begin(), plans.end(),
                  [](const Plan& a, const Plan& b) { return a.start > b.start; });

        std::vector<MultiCursor> next = edits;
        bool didChange = false;
        for (const auto& plan : plans) {
            if (plan.start < 0 || plan.start > (int)text.size()) continue;
            int safeEnd = std::max(plan.start, std::min(plan.end, (int)text.size()));
            if (safeEnd > plan.start) {
                text.erase(plan.start, safeEnd - plan.start);
                didChange = true;
            }
            if (!plan.insert.empty()) {
                text.insert(plan.start, plan.insert);
                didChange = true;
            }
            next[plan.index].cursor = plan.start + (int)plan.insert.size();
            next[plan.index].selStart = next[plan.index].cursor;
            next[plan.index].selEnd = next[plan.index].cursor;
        }
        if (didChange) {
            changed = true;
        }
        cursors_ = std::move(next);
        syncMultiToPrimary();
    }

    void insertText(std::string& text, const std::string& insert, bool& changed) {
        if (insert.empty()) return;
        syncPrimaryToMulti();
        std::vector<MultiCursor> edits = cursors_;
        std::vector<std::string> inserts(edits.size(), insert);
        applyEdits(text, edits, inserts, changed);
    }

    void insertPair(std::string& text, char open, char close, bool& changed) {
        syncPrimaryToMulti();
        if (cursors_.empty()) return;

        struct Plan { int index; int start; int end; std::string insert; int cursorTarget; };
        std::vector<Plan> plans;
        plans.reserve(cursors_.size());
        for (size_t i = 0; i < cursors_.size(); ++i) {
            const auto& c = cursors_[i];
            int start = c.cursor;
            int end = c.cursor;
            std::string insert;
            int target = 0;
            if (cursorHasSelection(c)) {
                start = std::min(c.selStart, c.selEnd);
                end = std::max(c.selStart, c.selEnd);
                std::string selected = text.substr(start, end - start);
                insert.push_back(open);
                insert += selected;
                insert.push_back(close);
                target = (int)insert.size();
            } else {
                insert.push_back(open);
                insert.push_back(close);
                target = 1;
            }
            plans.push_back({(int)i, start, end, insert, target});
        }

        std::sort(plans.begin(), plans.end(),
                  [](const Plan& a, const Plan& b) { return a.start > b.start; });
        std::vector<MultiCursor> next = cursors_;
        bool didChange = false;
        for (const auto& plan : plans) {
            if (plan.start < 0 || plan.start > (int)text.size()) continue;
            int safeEnd = std::max(plan.start, std::min(plan.end, (int)text.size()));
            if (safeEnd > plan.start) {
                text.erase(plan.start, safeEnd - plan.start);
                didChange = true;
            }
            if (!plan.insert.empty()) {
                text.insert(plan.start, plan.insert);
                didChange = true;
            }
            next[plan.index].cursor = plan.start + plan.cursorTarget;
            next[plan.index].selStart = next[plan.index].cursor;
            next[plan.index].selEnd = next[plan.index].cursor;
        }
        if (didChange) changed = true;
        cursors_ = std::move(next);
        syncMultiToPrimary();
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
        syncPrimaryToMulti();

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
                    if (close != 0 && mode->autoCloseBrackets()) {
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
        auto moveCursor = [&](MultiCursor& c, int newPos) {
            newPos = std::max(0, std::min(newPos, (int)text.size()));
            if (io.KeyShift) {
                if (!cursorHasSelection(c)) c.selStart = c.cursor;
                c.cursor = newPos;
                c.selEnd = c.cursor;
            } else {
                c.cursor = newPos;
                c.selStart = c.selEnd = -1;
            }
        };

        if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow)) {
            for (auto& c : cursors_) {
                if (!io.KeyShift && cursorHasSelection(c)) {
                    moveCursor(c, std::min(c.selStart, c.selEnd));
                } else if (c.cursor > 0) {
                    moveCursor(c, c.cursor - 1);
                }
            }
        }
        if (ImGui::IsKeyPressed(ImGuiKey_RightArrow)) {
            for (auto& c : cursors_) {
                if (!io.KeyShift && cursorHasSelection(c)) {
                    moveCursor(c, std::max(c.selStart, c.selEnd));
                } else if (c.cursor < (int)text.size()) {
                    moveCursor(c, c.cursor + 1);
                }
            }
        }
        if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
            for (auto& c : cursors_) {
                int curLine = lineFromPos(c.cursor, lineStarts);
                if (curLine > 0) {
                    int curCol = c.cursor - lineStarts[curLine];
                    int prevStart = lineStarts[curLine - 1];
                    int prevEnd = (curLine < (int)lineStarts.size()) ? lineStarts[curLine] - 1 : (int)text.size();
                    int prevLen = std::max(0, prevEnd - prevStart);
                    moveCursor(c, prevStart + std::min(curCol, prevLen));
                }
            }
        }
        if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
            for (auto& c : cursors_) {
                int curLine = lineFromPos(c.cursor, lineStarts);
                if (curLine + 1 < (int)lineStarts.size()) {
                    int curCol = c.cursor - lineStarts[curLine];
                    int nextStart = lineStarts[curLine + 1];
                    int nextEnd = (curLine + 2 < (int)lineStarts.size()) ? lineStarts[curLine + 2] - 1 : (int)text.size();
                    int nextLen = std::max(0, nextEnd - nextStart);
                    moveCursor(c, nextStart + std::min(curCol, nextLen));
                }
            }
        }

        // Editing keys
        if (ImGui::IsKeyPressed(ImGuiKey_Backspace) && !text.empty()) {
            std::vector<MultiCursor> edits = cursors_;
            std::vector<std::string> inserts(edits.size(), "");
            for (auto& c : edits) {
                if (!cursorHasSelection(c) && c.cursor > 0) {
                    c.selStart = c.cursor - 1;
                    c.selEnd = c.cursor;
                }
            }
            applyEdits(text, edits, inserts, changed);
        }
        if (ImGui::IsKeyPressed(ImGuiKey_Delete) && !text.empty()) {
            std::vector<MultiCursor> edits = cursors_;
            std::vector<std::string> inserts(edits.size(), "");
            for (auto& c : edits) {
                if (!cursorHasSelection(c) && c.cursor < (int)text.size()) {
                    c.selStart = c.cursor;
                    c.selEnd = c.cursor + 1;
                }
            }
            applyEdits(text, edits, inserts, changed);
        }

        if ((io.KeyCtrl || io.KeySuper) && ImGui::IsKeyPressed(ImGuiKey_A)) {
            clearExtraCursors();
            selStart_ = 0;
            selEnd_ = (int)text.size();
            cursor_ = selEnd_;
            syncPrimaryToMulti();
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
        if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            clearExtraCursors();
            selStart_ = selEnd_ = -1;
            syncPrimaryToMulti();
        }

        if ((io.KeyCtrl || io.KeySuper) && ImGui::IsKeyPressed(ImGuiKey_D)) {
            int start = -1;
            int end = -1;
            std::string needle;
            if (hasSelection()) {
                start = std::min(selStart_, selEnd_);
                end = std::max(selStart_, selEnd_);
                needle = text.substr(start, end - start);
            } else {
                int pos = cursor_;
                if (pos >= (int)text.size()) pos = (int)text.size() - 1;
                auto isWord = [](char c) { return std::isalnum((unsigned char)c) || c == '_'; };
                int s = pos;
                while (s > 0 && isWord(text[s - 1])) --s;
                int e = pos;
                while (e < (int)text.size() && isWord(text[e])) ++e;
                if (e > s) {
                    start = s;
                    end = e;
                    needle = text.substr(start, end - start);
                }
            }
            if (!needle.empty()) {
                size_t next = text.find(needle, (size_t)end);
                if (next != std::string::npos) {
                    addCursorAt((int)next, (int)next, (int)next + (int)needle.size());
                    dedupeCursors();
                }
            }
        }

        if ((io.KeyCtrl || io.KeySuper) && io.KeyAlt &&
            ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
            for (const auto& c : cursors_) {
                int curLine = lineFromPos(c.cursor, lineStarts);
                if (curLine <= 0) continue;
                int curCol = c.cursor - lineStarts[curLine];
                int pos = positionFromLineCol(lineStarts, curLine - 1, curCol, text);
                addCursorAt(pos);
            }
            dedupeCursors();
        }

        if ((io.KeyCtrl || io.KeySuper) && io.KeyAlt &&
            ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
            for (const auto& c : cursors_) {
                int curLine = lineFromPos(c.cursor, lineStarts);
                if (curLine + 1 >= (int)lineStarts.size()) continue;
                int curCol = c.cursor - lineStarts[curLine];
                int pos = positionFromLineCol(lineStarts, curLine + 1, curCol, text);
                addCursorAt(pos);
            }
            dedupeCursors();
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
