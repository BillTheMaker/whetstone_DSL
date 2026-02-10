#pragma once
// Included inside CodeEditorWidget (rendering + folds).
public:
    static int annotationShapeIndex(const std::string& label) {
        size_t h = 0;
        for (unsigned char c : label) h = h * 131 + c;
        return (int)(h % 5);
    }

    static void drawAnnotationShape(ImDrawList* drawList,
                                    const ImVec2& center,
                                    float size,
                                    ImU32 color,
                                    int shape) {
        switch (shape) {
        case 0: { // circle
            drawList->AddCircleFilled(center, size, color);
            break;
        }
        case 1: { // triangle
            ImVec2 top(center.x, center.y - size);
            ImVec2 left(center.x - size, center.y + size);
            ImVec2 right(center.x + size, center.y + size);
            drawList->AddTriangleFilled(top, left, right, color);
            break;
        }
        case 2: { // square
            ImVec2 a(center.x - size, center.y - size);
            ImVec2 b(center.x + size, center.y + size);
            drawList->AddRectFilled(a, b, color, 0.0f);
            break;
        }
        case 3: { // diamond
            ImVec2 top(center.x, center.y - size);
            ImVec2 right(center.x + size, center.y);
            ImVec2 bottom(center.x, center.y + size);
            ImVec2 left(center.x - size, center.y);
            drawList->AddQuadFilled(top, right, bottom, left, color);
            break;
        }
        default: { // star (outline)
            ImVec2 pts[10];
            for (int i = 0; i < 10; ++i) {
                float angle = (float)(IM_PI * 0.5 + i * (IM_PI / 5.0));
                float radius = (i % 2 == 0) ? size : size * 0.5f;
                pts[i] = ImVec2(center.x + std::cos(angle) * radius,
                                center.y - std::sin(angle) * radius);
            }
            drawList->AddPolyline(pts, 10, color, true, 1.5f);
            break;
        }
        }
    }

    CodeEditorResult render(const char* id,
                             std::string& text,
                             const std::vector<HighlightSpan>& spans,
                             const CodeEditorOptions& options,
                             const ImVec2& size,
                             ImFont* font) {
        CodeEditorResult result;
        if (!font) font = ImGui::GetFont();

        ImGuiIO& io = ImGui::GetIO();
        syncPrimaryToMulti();
        const double nowSeconds = options.nowSeconds > 0.0 ? options.nowSeconds : ImGui::GetTime();
        // Build line start offsets
        std::vector<int> lineStarts;
        lineStarts.reserve(128);
        lineStarts.push_back(0);
        for (int i = 0; i < (int)text.size(); ++i) {
            if (text[i] == '\n') lineStarts.push_back(i + 1);
        }
        const int lineCount = (int)lineStarts.size();

        auto isOpenBracket = [](char c) {
            return c == '(' || c == '[' || c == '{';
        };
        auto isCloseBracket = [](char c) {
            return c == ')' || c == ']' || c == '}';
        };
        auto matchingOpen = [](char c) {
            if (c == ')') return '(';
            if (c == ']') return '[';
            if (c == '}') return '{';
            return '\0';
        };
        auto isBracketChar = [&](char c) {
            return isOpenBracket(c) || isCloseBracket(c);
        };

        std::vector<int> bracketDepth(text.size(), -1);
        std::vector<int> bracketMatch(text.size(), -1);
        std::vector<int> stack;
        std::vector<char> stackChars;
        stack.reserve(128);
        stackChars.reserve(128);
        for (int i = 0; i < (int)text.size(); ++i) {
            char c = text[i];
            if (isOpenBracket(c)) {
                stack.push_back(i);
                stackChars.push_back(c);
                bracketDepth[i] = (int)stack.size() - 1;
            } else if (isCloseBracket(c)) {
                char open = matchingOpen(c);
                if (!stackChars.empty() && stackChars.back() == open) {
                    int openPos = stack.back();
                    stack.pop_back();
                    stackChars.pop_back();
                    int depth = (int)stack.size();
                    bracketDepth[i] = depth;
                    bracketMatch[i] = openPos;
                    bracketMatch[openPos] = i;
                }
            }
        }

        std::array<ImU32, 6> rainbow = {
            ThemeEngine::instance().editorColor("bracket_1", IM_COL32(242, 120, 75, 255)),
            ThemeEngine::instance().editorColor("bracket_2", IM_COL32(241, 196, 15, 255)),
            ThemeEngine::instance().editorColor("bracket_3", IM_COL32(46, 204, 113, 255)),
            ThemeEngine::instance().editorColor("bracket_4", IM_COL32(52, 152, 219, 255)),
            ThemeEngine::instance().editorColor("bracket_5", IM_COL32(155, 89, 182, 255)),
            ThemeEngine::instance().editorColor("bracket_6", IM_COL32(231, 76, 60, 255))
        };

        int focusBracketPos = -1;
        int focusBracketMatch = -1;
        if (cursor_ > 0 && cursor_ - 1 < (int)text.size() && isBracketChar(text[cursor_ - 1])) {
            focusBracketPos = cursor_ - 1;
        } else if (cursor_ < (int)text.size() && isBracketChar(text[cursor_])) {
            focusBracketPos = cursor_;
        }
        if (focusBracketPos >= 0 && focusBracketPos < (int)bracketMatch.size()) {
            focusBracketMatch = bracketMatch[focusBracketPos];
        }
        int scopeStartLine = -1;
        int scopeEndLine = -1;
        if (focusBracketMatch >= 0) {
            int start = std::min(focusBracketPos, focusBracketMatch);
            int end = std::max(focusBracketPos, focusBracketMatch);
            scopeStartLine = lineFromPos(start, lineStarts);
            scopeEndLine = lineFromPos(end, lineStarts);
        }

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

        if (options.enableFolding && options.mode) {
            updateFolds(text, options.mode->getLanguage());
        } else {
            folds_.clear();
        }

        // Measure
        const float lineHeight =
            ImGui::GetTextLineHeightWithSpacing() * std::max(0.5f, options.lineHeightScale);
        const float baseAdvance = font->CalcTextSizeA(font->FontSize, FLT_MAX, -1.0f, "M").x;
        const float charAdvance = baseAdvance + std::max(0.0f, options.letterSpacing);
        const float gutterWidth = options.showLineNumbers ?
            calcGutterWidth(lineCount, font, charAdvance) : 12.0f;
        const float minimapWidth = options.showMinimap ? 80.0f : 0.0f;

        // Estimate content size for scrollbars
        int maxLineLen = 0;
        for (int ln = 0; ln < lineCount; ++ln) {
            int start = lineStarts[ln];
            int end = (ln + 1 < lineCount) ? lineStarts[ln + 1] - 1 : (int)text.size();
            int len = std::max(0, end - start);
            maxLineLen = std::max(maxLineLen, len);
        }
        ImVec2 contentSize(gutterWidth + maxLineLen * charAdvance + minimapWidth + 4.0f,
                           lineCount * lineHeight + 4.0f);

        ImGui::BeginChild(id, size, false, ImGuiWindowFlags_HorizontalScrollbar);
        if (options.syncScrollX && !options.scrollMaster) {
            ImGui::SetScrollX(*options.syncScrollX);
        }
        if (options.syncScrollY && !options.scrollMaster) {
            ImGui::SetScrollY(*options.syncScrollY);
        }
        std::string annoPopupId = std::string(id) + "_AnnoPopup";
        ImVec2 origin = ImGui::GetCursorScreenPos();

        // Dummy to set scroll extents
        ImGui::Dummy(contentSize);

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        const float scrollX = ImGui::GetScrollX();
        const float scrollY = ImGui::GetScrollY();
        if (options.syncScrollX && options.scrollMaster) {
            *options.syncScrollX = scrollX;
        }
        if (options.syncScrollY && options.scrollMaster) {
            *options.syncScrollY = scrollY;
        }
        ImVec2 gutterBase(origin.x, origin.y - scrollY);
        ImVec2 textBase(origin.x + gutterWidth - scrollX, origin.y - scrollY);
        const float windowWidth = ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x;
        const float minimapBaseX = origin.x + std::max(0.0f, windowWidth - minimapWidth);
        const float minimapBaseY = origin.y - scrollY;

        // Focus and input
        const bool hovered = ImGui::IsWindowHovered();
        const bool focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);

        // Mouse handling
        if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            ImGui::SetKeyboardFocusHere(-1);
            const ImVec2 mouse = ImGui::GetMousePos();
            if (options.showMinimap && mouse.x >= minimapBaseX) {
                float miniHeight = lineCount * lineHeight;
                if (miniHeight > 0.0f) {
                    float localY = mouse.y - minimapBaseY;
                    float t = std::max(0.0f, std::min(1.0f, localY / miniHeight));
                    float targetScroll = t * lineCount * lineHeight;
                    ImGui::SetScrollY(targetScroll);
                }
                selecting_ = false;
                goto after_mouse;
            }
            bool ctrlClick = io.KeyCtrl || io.KeySuper;
            bool altClick = io.KeyAlt;
            bool columnSelect = io.KeyAlt && io.KeyShift;
            int clickCount = ImGui::GetIO().MouseClickedCount[0];
            if (mouse.x < origin.x + gutterWidth || clickCount >= 3) {
                int line = lineFromMouseY(mouse.y, gutterBase.y, lineHeight, lineCount);
                result.lineClicked = true;
                result.clickedLine = line;
                const FoldRegion* fold = findFoldAtLine(line);
                if (fold && mouse.x < origin.x + 12.0f) {
                    toggleFoldAtLine(line);
                }
                int lineStart = lineStarts[line];
                int lineEnd = (line + 1 < lineCount) ? lineStarts[line + 1] : (int)text.size();
                cursor_ = lineStart;
                selStart_ = lineStart;
                selEnd_ = lineEnd;
            } else if (columnSelect) {
                int pos = positionFromMouse(mouse, textBase, lineStarts, text, charAdvance, lineHeight);
                int line = lineFromPos(pos, lineStarts);
                int col = pos - lineStarts[line];
                columnSelectActive_ = true;
                columnAnchorLine_ = line;
                columnAnchorCol_ = col;
                updateColumnSelection(text, lineStarts, columnAnchorLine_, columnAnchorCol_, line, col);
                selecting_ = true;
                goto after_mouse;
            } else if (altClick) {
                int pos = positionFromMouse(mouse, textBase, lineStarts, text, charAdvance, lineHeight);
                addCursorAt(pos);
                selecting_ = false;
                goto after_mouse;
            } else {
                clearExtraCursors();
                int pos = positionFromMouse(mouse, textBase, lineStarts, text, charAdvance, lineHeight);
                result.lineClicked = true;
                result.clickedLine = lineFromPos(pos, lineStarts);
                if (ctrlClick) {
                    result.ctrlClick = true;
                    result.ctrlClickPos = pos;
                    result.ctrlClickLine = result.clickedLine;
                    result.ctrlClickCol = pos - lineStarts[result.ctrlClickLine];
                }
                if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                    selectWordAt(text, pos);
                } else if (ImGui::GetIO().KeyShift) {
                    if (!hasSelection()) selStart_ = cursor_;
                    cursor_ = pos;
                    selEnd_ = cursor_;
                } else {
                    cursor_ = pos;
                    selStart_ = cursor_;
                    selEnd_ = cursor_;
                }
            }
            selecting_ = true;
        }
    after_mouse:
        if (hovered && selecting_ && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
            const ImVec2 mouse = ImGui::GetMousePos();
            if (mouse.x >= origin.x + gutterWidth) {
                int pos = positionFromMouse(mouse, textBase, lineStarts, text, charAdvance, lineHeight);
                if (columnSelectActive_ && (io.KeyAlt && io.KeyShift)) {
                    int line = lineFromPos(pos, lineStarts);
                    int col = pos - lineStarts[line];
                    updateColumnSelection(text, lineStarts, columnAnchorLine_, columnAnchorCol_, line, col);
                } else {
                    selEnd_ = pos;
                }
            }
        }
        if (selecting_ && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
            selecting_ = false;
            columnSelectActive_ = false;
        }

        if (hovered) {
            const ImVec2 mouse = ImGui::GetMousePos();
            if (mouse.x >= origin.x + gutterWidth &&
                (!options.showMinimap || mouse.x < minimapBaseX)) {
                int pos = positionFromMouse(mouse, textBase, lineStarts, text, charAdvance, lineHeight);
                int line = lineFromPos(pos, lineStarts);
                result.hoverValid = true;
                result.hoverPos = pos;
                result.hoverLine = line;
                result.hoverCol = pos - lineStarts[line];
            }
        }

        // Keyboard input
        if (focused && !options.readOnly) {
            handleKeyboard(text, result.changed, lineStarts, options.mode);
        }

        // Render visible lines (virtualized with buffer)
        const int virtualBuffer = 50;
        const int viewFirst = std::max(0, (int)(scrollY / lineHeight));
        const int viewLines = (int)(ImGui::GetContentRegionAvail().y / lineHeight) + 2;
        const int firstLine = std::max(0, viewFirst - virtualBuffer);
        const int lastLine = std::min(lineCount - 1, viewFirst + viewLines + virtualBuffer);

        const float textAreaWidth = std::max(0.0f, windowWidth - gutterWidth - minimapWidth);
        const int currentLine = lineFromPos(cursor_, lineStarts);

        renderVisibleLines(text,
                           lineStarts,
                           lineCount,
                           firstLine,
                           lastLine,
                           currentLine,
                           lineHeight,
                           charAdvance,
                           gutterWidth,
                           textAreaWidth,
                           origin,
                           textBase,
                           options,
                           charCats,
                           bracketDepth,
                           rainbow,
                           focusBracketPos,
                           focusBracketMatch,
                           scopeStartLine,
                           scopeEndLine,
                           nowSeconds,
                           hovered,
                           drawList,
                           result,
                           font);
        if (ImGui::BeginPopup(annoPopupId.c_str())) {
            ImGui::TextUnformatted("Annotation Editor (TODO)");
            ImGui::Separator();
            ImGui::TextUnformatted(annotationPopupMessage_.c_str());
            ImGui::EndPopup();
        }

        // Cursor
        if (focused) {
            float blinkAlpha = AnimationUtils::blinkAlpha(nowSeconds,
                                                          options.cursorBlinkRate,
                                                          options.reduceMotion);
            if (blinkAlpha > 0.02f) {
                for (size_t i = 0; i < cursors_.size(); ++i) {
                    const auto& c = cursors_[i];
                    int curLine = lineFromPos(c.cursor, lineStarts);
                    int lineStart = lineStarts[curLine];
                    int col = c.cursor - lineStart;
                    float x = textBase.x + col * charAdvance;
                    float y = textBase.y + curLine * lineHeight;
                    ImU32 base = ThemeEngine::instance().editorColor("caret",
                                                                    IM_COL32(240, 240, 240, 255));
                    ImVec4 color = ImGui::ColorConvertU32ToFloat4(base);
                    color.w *= (i == 0) ? blinkAlpha : (blinkAlpha * 0.6f);
                    ImU32 col32 = ImGui::ColorConvertFloat4ToU32(color);
                    drawList->AddLine(ImVec2(x, y), ImVec2(x, y + lineHeight), col32, 1.0f);
                }
            }
        }

        // Minimap
        if (options.showMinimap && minimapWidth > 0.0f) {
            float miniHeight = lineCount * lineHeight;
            ImVec2 miniA(minimapBaseX, minimapBaseY);
            ImVec2 miniB(minimapBaseX + minimapWidth, minimapBaseY + miniHeight);
            drawList->AddRectFilled(miniA, miniB,
                                    ThemeEngine::instance().editorColor("minimap_bg",
                                                                        IM_COL32(18, 18, 18, 255)));
            for (int ln = 0; ln < lineCount; ++ln) {
                float y = minimapBaseY + ln * lineHeight;
                ImU32 color = ThemeEngine::instance().editorColor("minimap_line",
                                                                  IM_COL32(80, 80, 80, 255));
                if (ln == currentLine) {
                    color = ThemeEngine::instance().editorColor("minimap_line_active",
                                                                IM_COL32(120, 120, 160, 255));
                }
                drawList->AddRectFilled(ImVec2(minimapBaseX + 2.0f, y),
                                        ImVec2(minimapBaseX + minimapWidth - 2.0f, y + 2.0f),
                                        color);
            }
            float viewStart = (lineCount > 0) ? (scrollY / (lineCount * lineHeight)) : 0.0f;
            float viewEnd = viewStart + (ImGui::GetContentRegionAvail().y / (lineCount * lineHeight));
            viewStart = std::max(0.0f, std::min(1.0f, viewStart));
            viewEnd = std::max(0.0f, std::min(1.0f, viewEnd));
            ImVec2 vA(minimapBaseX, minimapBaseY + viewStart * miniHeight);
            ImVec2 vB(minimapBaseX + minimapWidth, minimapBaseY + viewEnd * miniHeight);
            drawList->AddRect(vA, vB,
                              ThemeEngine::instance().editorColor("minimap_view",
                                                                  IM_COL32(120, 160, 220, 180)));
        }

        ImGui::EndChild();

        syncMultiToPrimary();
        result.cursorByte = cursor_;
        result.lineCount = lineCount;
        result.gutterWidth = gutterWidth;
        result.currentLine = currentLine;
        result.foldCount = (int)folds_.size();
        result.anyFolded = std::any_of(folds_.begin(), folds_.end(),
                                       [](const FoldRegion& f) { return f.folded; });
        result.minimapEnabled = options.showMinimap;
        result.minimapWidth = minimapWidth;
        if (options.showMinimap && lineCount > 0) {
            float miniHeight = lineCount * lineHeight;
            result.minimapViewportStart = (scrollY / (lineCount * lineHeight)) * miniHeight;
            result.minimapViewportEnd = result.minimapViewportStart +
                                        (ImGui::GetContentRegionAvail().y / (lineCount * lineHeight)) * miniHeight;
        }
        return result;
    }

    void setCursor(int pos) {
        cursor_ = pos;
        syncPrimaryToMulti();
    }
    int getCursor() const { return cursor_; }
    bool hasSelectionRange() const { return hasSelection(); }
    void getSelectionRange(int& start, int& end) const {
        if (!hasSelection()) {
            start = end = -1;
            return;
        }
        start = std::min(selStart_, selEnd_);
        end = std::max(selStart_, selEnd_);
    }
    void setSelectionRange(int start, int end) {
        selStart_ = start;
        selEnd_ = end;
    }
    const std::vector<FoldRegion>& getFoldRegions() const { return folds_; }
    std::vector<int> getFoldedLines() const {
        std::vector<int> lines;
        for (const auto& f : folds_) {
            if (f.folded) lines.push_back(f.startLine);
        }
        return lines;
    }

    void setDesiredFoldedLines(const std::vector<int>& lines) {
        desiredFoldedLines_ = lines;
        applyDesiredFoldState_ = true;
    }

    void toggleFoldAtLine(int line) {
        for (auto& f : folds_) {
            if (f.startLine == line) {
                f.folded = !f.folded;
                return;
            }
        }
    }

private:
    int cursor_ = 0;
    int selStart_ = -1;
    int selEnd_ = -1;
    bool selecting_ = false;
    std::vector<MultiCursor> cursors_;
    bool columnSelectActive_ = false;
    int columnAnchorLine_ = 0;
    int columnAnchorCol_ = 0;
    std::vector<FoldRegion> folds_;
    std::string lastFoldText_;
    std::string lastFoldLang_;
    std::string annotationPopupMessage_;
    std::vector<int> desiredFoldedLines_;
    bool applyDesiredFoldState_ = false;

#include "CodeEditorRenderHelpers.h"
#include "CodeEditorRenderVisible.h"
