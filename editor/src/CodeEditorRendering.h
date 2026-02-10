#pragma once
// Included inside CodeEditorWidget (rendering + folds).
public:
    CodeEditorResult render(const char* id,
                             std::string& text,
                             const std::vector<HighlightSpan>& spans,
                             const CodeEditorOptions& options,
                             const ImVec2& size,
                             ImFont* font) {
        CodeEditorResult result;
        if (!font) font = ImGui::GetFont();

        ImGuiIO& io = ImGui::GetIO();
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

        if (options.enableFolding && options.mode) {
            updateFolds(text, options.mode->getLanguage());
        } else {
            folds_.clear();
        }

        // Measure
        const float lineHeight = ImGui::GetTextLineHeightWithSpacing();
        const float charAdvance = font->CalcTextSizeA(font->FontSize, FLT_MAX, -1.0f, "M").x;
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
            } else {
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
                selEnd_ = pos;
            }
        }
        if (selecting_ && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
            selecting_ = false;
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

        // Render visible lines
        const int firstLine = std::max(0, (int)(scrollY / lineHeight));
        const int visibleLines = (int)(ImGui::GetContentRegionAvail().y / lineHeight) + 2;
        const int lastLine = std::min(lineCount - 1, firstLine + visibleLines);

        const float textAreaWidth = std::max(0.0f, windowWidth - gutterWidth - minimapWidth);
        const int currentLine = lineFromPos(cursor_, lineStarts);

        std::vector<bool> hiddenLines(lineCount, false);
        for (const auto& f : folds_) {
            if (!f.folded) continue;
            for (int l = f.startLine + 1; l <= f.endLine && l < lineCount; ++l) {
                hiddenLines[l] = true;
            }
        }

        for (int ln = firstLine; ln <= lastLine; ++ln) {
            if (ln >= 0 && ln < lineCount && hiddenLines[ln]) continue;
            int start = lineStarts[ln];
            int end = (ln + 1 < lineCount) ? lineStarts[ln + 1] - 1 : (int)text.size();
            int len = std::max(0, end - start);

            float y = textBase.y + ln * lineHeight;

            // Gutter background
            ImVec2 gutterA(origin.x, y);
            ImVec2 gutterB(origin.x + gutterWidth, y + lineHeight);
            drawList->AddRectFilled(
                gutterA, gutterB,
                ThemeEngine::instance().editorColor("gutter_bg",
                                                    IM_COL32(20, 20, 20, 255)));

            // Line number (right-aligned)
            if (options.showLineNumbers) {
                char numBuf[16];
                snprintf(numBuf, sizeof(numBuf), "%d", ln + 1);
                ImVec2 numSize = font->CalcTextSizeA(font->FontSize, FLT_MAX, -1.0f, numBuf);
                ImVec2 numPos(origin.x + gutterWidth - 4.0f - numSize.x, y);
                drawList->AddText(font, font->FontSize, numPos,
                                  ThemeEngine::instance().editorColor("gutter_text",
                                                                      IM_COL32(120, 120, 120, 255)),
                                  numBuf);
            }

            // Diagnostics marker
            bool hasError = lineIn(options.errorLines, ln);
            bool hasWarn = lineIn(options.warningLines, ln);
            if (hasError || hasWarn) {
                ImU32 color = hasError
                    ? ThemeEngine::instance().editorColor("diag_error", IM_COL32(220, 80, 80, 255))
                    : ThemeEngine::instance().editorColor("diag_warning", IM_COL32(220, 160, 60, 255));
                ImVec2 center(origin.x + 3.0f, y + lineHeight * 0.5f);
                drawList->AddCircleFilled(center, 3.0f, color);
            }
            if (options.diagnostics) {
                std::string msg = diagnosticMessageAtLine(*options.diagnostics, ln);
                ImVec2 gutterA(origin.x, y);
                ImVec2 gutterB(origin.x + gutterWidth, y + lineHeight);
                if (!msg.empty() && ImGui::IsMouseHoveringRect(gutterA, gutterB)) {
                    ImGui::BeginTooltip();
                    ImGui::TextUnformatted(msg.c_str());
                    ImGui::EndTooltip();
                }
            }

            // Annotation marker
            if (options.annotations) {
                AnnotationMarker marker;
                if (annotationAtLine(*options.annotations, ln, marker)) {
                    ImVec2 center(origin.x + 12.0f, y + lineHeight * 0.5f);
                    drawList->AddCircleFilled(center, 3.0f, marker.color);
                    ImVec2 a(center.x - 4.0f, center.y - 4.0f);
                    ImVec2 b(center.x + 4.0f, center.y + 4.0f);
                    if (ImGui::IsMouseHoveringRect(a, b)) {
                        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                            annotationPopupMessage_ = marker.message;
                            ImGui::OpenPopup(annoPopupId.c_str());
                        }
                        ImGui::BeginTooltip();
                        ImGui::TextUnformatted(marker.message.c_str());
                        ImGui::EndTooltip();
                    }
                }
            }

            // Suggestion marker (lightbulb)
            if (options.suggestions) {
                SuggestionMarker suggestion;
                if (suggestionAtLine(*options.suggestions, ln, suggestion)) {
                    ImVec2 center(origin.x + 20.0f, y + lineHeight * 0.5f);
                    ImU32 color = ThemeEngine::instance().editorColor("suggestion",
                                                                      IM_COL32(240, 200, 40, 255));
                    drawList->AddCircleFilled(center, 3.0f, color);
                    ImVec2 a(center.x - 4.0f, center.y - 4.0f);
                    ImVec2 b(center.x + 4.0f, center.y + 4.0f);
                    if (ImGui::IsMouseHoveringRect(a, b)) {
                        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                            result.suggestionClicked = true;
                            result.clickedSuggestion = suggestion;
                        }
                        ImGui::BeginTooltip();
                        ImGui::Text("%s (%.2f)", suggestion.label.c_str(), suggestion.confidence);
                        ImGui::Separator();
                        ImGui::TextUnformatted(suggestion.reason.c_str());
                        ImGui::EndTooltip();
                    }
                }
            }

            // Conflict markers: highlight and connecting line
            if (options.conflicts) {
                AnnotationConflictMarker conflict;
                if (conflictAtLine(*options.conflicts, ln, conflict)) {
                    ImVec2 center(origin.x + 12.0f, y + lineHeight * 0.5f);
                    drawList->AddCircle(center, 5.0f,
                                        ThemeEngine::instance().editorColor("conflict",
                                                                            IM_COL32(220, 80, 80, 255)),
                                        12, 1.5f);
                    if (ln == std::min(conflict.childLine, conflict.parentLine) &&
                        conflict.childLine >= 0 && conflict.parentLine >= 0) {
                        float y1 = textBase.y + conflict.childLine * lineHeight + lineHeight * 0.5f;
                        float y2 = textBase.y + conflict.parentLine * lineHeight + lineHeight * 0.5f;
                        drawList->AddLine(ImVec2(origin.x + 12.0f, y1),
                                          ImVec2(origin.x + 12.0f, y2),
                                          ThemeEngine::instance().editorColor("conflict_line",
                                                                              IM_COL32(220, 80, 80, 180)),
                                          1.0f);
                    }
                    ImVec2 a(center.x - 5.0f, center.y - 5.0f);
                    ImVec2 b(center.x + 5.0f, center.y + 5.0f);
                    if (ImGui::IsMouseHoveringRect(a, b)) {
                        ImGui::BeginTooltip();
                        ImGui::TextUnformatted(conflict.message.c_str());
                        ImGui::EndTooltip();
                    }
                }
            }

            // Fold indicator
            const FoldRegion* fold = findFoldAtLine(ln);
            if (fold) {
                ImVec2 triCenter(origin.x + 6.0f, y + lineHeight * 0.5f);
                if (fold->folded) {
                    drawTriangle(drawList, triCenter,
                                 ThemeEngine::instance().editorColor("fold_indicator",
                                                                     IM_COL32(160, 160, 160, 255)),
                                 true);
                } else {
                    drawTriangle(drawList, triCenter,
                                 ThemeEngine::instance().editorColor("fold_indicator",
                                                                     IM_COL32(160, 160, 160, 255)),
                                 false);
                }
            }

            // Current line highlight (text area only)
            if (options.showCurrentLine && ln == currentLine) {
                ImVec2 hlA(textBase.x, y);
                ImVec2 hlB(textBase.x + textAreaWidth, y + lineHeight);
                drawList->AddRectFilled(hlA, hlB,
                                        ThemeEngine::instance().editorColor("line_highlight",
                                                                            IM_COL32(40, 40, 40, 120)));
            }
            if (options.highlightLine >= 0 && ln == options.highlightLine) {
                ImVec2 hlA(textBase.x, y);
                ImVec2 hlB(textBase.x + textAreaWidth, y + lineHeight);
                drawList->AddRectFilled(hlA, hlB, options.highlightLineColor);
            }
            if (options.highlightLines) {
                if (std::find(options.highlightLines->begin(),
                              options.highlightLines->end(), ln) != options.highlightLines->end()) {
                    ImVec2 hlA(textBase.x, y);
                    ImVec2 hlB(textBase.x + textAreaWidth, y + lineHeight);
                    drawList->AddRectFilled(hlA, hlB, options.highlightLineColor);
                }
            }

            // Selection background
            if (hasSelection()) {
                int selA = std::min(selStart_, selEnd_);
                int selB = std::max(selStart_, selEnd_);
                int lineSelStart = std::max(selA, start);
                int lineSelEnd = std::min(selB, end);
                if (lineSelStart < lineSelEnd) {
                    int colA = lineSelStart - start;
                    int colB = lineSelEnd - start;
                    ImVec2 a(textBase.x + colA * charAdvance, y);
                    ImVec2 b(textBase.x + colB * charAdvance, y + lineHeight);
                    drawList->AddRectFilled(a, b,
                                            ThemeEngine::instance().editorColor("selection",
                                                                                IM_COL32(60, 100, 160, 120)));
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

                ImVec2 p(textBase.x + (pos - start) * charAdvance, y);
                drawList->AddText(font, font->FontSize, p, colorFor(cat), chunk.c_str());

                pos = spanEnd;
            }

            // Inline annotation tag
            if (options.showAnnotations && options.annotations) {
                AnnotationMarker marker;
                if (annotationAtLine(*options.annotations, ln, marker)) {
                    ImVec2 tagPos;
                    if (options.annotationLayout == 1) {
                        tagPos = ImVec2(origin.x + 18.0f, y);
                    } else if (options.annotationLayout == 2) {
                        tagPos = ImVec2(textBase.x + len * charAdvance + 8.0f, y);
                    } else {
                        tagPos = ImVec2(textBase.x, y - lineHeight * 0.7f);
                    }
                    ImVec2 textSize = font->CalcTextSizeA(font->FontSize, FLT_MAX, -1.0f, marker.message.c_str());
                    ImVec2 bgA(tagPos.x - 2.0f, tagPos.y);
                    ImVec2 bgB(tagPos.x + textSize.x + 6.0f, tagPos.y + lineHeight * 0.8f);
                    drawList->AddRectFilled(bgA, bgB,
                                            ThemeEngine::instance().editorColor("annotation_tag_bg",
                                                                                IM_COL32(30, 30, 30, 220)),
                                            2.0f);
                    drawList->AddText(font, font->FontSize, ImVec2(tagPos.x + 2.0f, tagPos.y),
                                      marker.color, marker.message.c_str());
                }
            }

            // Folded placeholder
            if (fold && fold->folded) {
                ImVec2 p(textBase.x + len * charAdvance + 6.0f, y);
                drawList->AddText(font, font->FontSize, p,
                                  ThemeEngine::instance().editorColor("fold_placeholder",
                                                                      IM_COL32(140, 140, 140, 255)),
                                  "{...}");
            }

            if (options.diagnostics) {
                renderSquiggles(*options.diagnostics, ln, y, lineHeight, charAdvance,
                                textBase.x, len, drawList);
                if (hovered) {
                    ImVec2 mouse = ImGui::GetMousePos();
                    std::string msg = diagnosticMessageAtPoint(*options.diagnostics, ln, mouse,
                                                               y, lineHeight, charAdvance, textBase.x);
                    if (!msg.empty()) {
                        ImGui::BeginTooltip();
                        ImGui::TextUnformatted(msg.c_str());
                        ImGui::EndTooltip();
                    }
                }
            }
        }

        if (ImGui::BeginPopup(annoPopupId.c_str())) {
            ImGui::TextUnformatted("Annotation Editor (TODO)");
            ImGui::Separator();
            ImGui::TextUnformatted(annotationPopupMessage_.c_str());
            ImGui::EndPopup();
        }

        // Cursor
        if (focused) {
            const double t = ImGui::GetTime();
            const bool blinkOn = ((int)(t * 2.0)) % 2 == 0;
            if (blinkOn) {
                int curLine = currentLine;
                int lineStart = lineStarts[curLine];
                int col = cursor_ - lineStart;
                float x = textBase.x + col * charAdvance;
                float y = textBase.y + curLine * lineHeight;
                drawList->AddLine(ImVec2(x, y), ImVec2(x, y + lineHeight),
                                  ThemeEngine::instance().editorColor("caret",
                                                                      IM_COL32(240, 240, 240, 255)),
                                  1.0f);
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

    void setCursor(int pos) { cursor_ = pos; }
    int getCursor() const { return cursor_; }
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
    std::vector<FoldRegion> folds_;
    std::string lastFoldText_;
    std::string lastFoldLang_;
    std::string annotationPopupMessage_;
    std::vector<int> desiredFoldedLines_;
    bool applyDesiredFoldState_ = false;

#include "CodeEditorRenderHelpers.h"
