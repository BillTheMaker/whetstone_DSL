#pragma once
// Included inside CodeEditorWidget (visible line rendering).
    void renderVisibleLines(const std::string& text,
                            const std::vector<int>& lineStarts,
                            int lineCount,
                            int firstLine,
                            int lastLine,
                            int currentLine,
                            float lineHeight,
                            float charAdvance,
                            float gutterWidth,
                            float textAreaWidth,
                            const ImVec2& origin,
                            const ImVec2& textBase,
                            const CodeEditorOptions& options,
                            const std::vector<TokenCategory>& charCats,
                            const std::vector<int>& bracketDepth,
                            const std::array<ImU32, 6>& rainbow,
                            int focusBracketPos,
                            int focusBracketMatch,
                            int scopeStartLine,
                            int scopeEndLine,
                            double nowSeconds,
                            bool hovered,
                            ImDrawList* drawList,
                            CodeEditorResult& result,
                            ImFont* font) {
        std::vector<bool> hiddenLines;
        if (lineCount <= 10000) {
            hiddenLines.assign(lineCount, false);
            for (const auto& f : folds_) {
                if (!f.folded) continue;
                for (int l = f.startLine + 1; l <= f.endLine && l < lineCount; ++l) {
                    hiddenLines[l] = true;
                }
            }
        }

        auto isHiddenLine = [&](int ln) {
            if (lineCount <= 10000) {
                return ln >= 0 && ln < lineCount && hiddenLines[ln];
            }
            for (const auto& f : folds_) {
                if (!f.folded) continue;
                if (ln > f.startLine && ln <= f.endLine) return true;
            }
            return false;
        };

        for (int ln = firstLine; ln <= lastLine; ++ln) {
            if (ln < 0 || ln >= lineCount) continue;
            if (isHiddenLine(ln)) continue;
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
            DiagnosticRange securityDiag;
            bool hasSecurity = options.diagnostics &&
                               securityDiagnosticAtLine(*options.diagnostics, ln, securityDiag);
            if (hasSecurity) {
                ImU32 secColor = (securityDiag.severity == 1)
                    ? ThemeEngine::instance().editorColor("diag_error", IM_COL32(220, 80, 80, 255))
                    : (securityDiag.severity == 2)
                        ? ThemeEngine::instance().editorColor("diag_warning", IM_COL32(220, 160, 60, 255))
                        : ThemeEngine::instance().editorColor("diag_info", IM_COL32(90, 160, 220, 255));
                ImVec2 center(origin.x + 12.0f, y + lineHeight * 0.5f);
                ImVec2 top(center.x, center.y - 4.0f);
                ImVec2 left(center.x - 4.0f, center.y - 1.0f);
                ImVec2 right(center.x + 4.0f, center.y - 1.0f);
                ImVec2 bottom(center.x, center.y + 4.0f);
                drawList->AddTriangleFilled(top, left, right, secColor);
                drawList->AddTriangleFilled(left, right, bottom, secColor);
            }
            if (options.diagnostics) {
                std::string msg = diagnosticMessageAtLine(*options.diagnostics, ln);
                ImVec2 gutterA(origin.x, y);
                ImVec2 gutterB(origin.x + gutterWidth, y + lineHeight);
                if (!msg.empty()) {
                    bool hover = ImGui::IsMouseHoveringRect(gutterA, gutterB);
                    renderRichTooltip("diag_gutter_" + std::to_string(ln),
                                      msg,
                                      hover,
                                      options.reduceMotion,
                                      font);
                }
                if (hasSecurity && !securityDiag.message.empty()) {
                    bool hover = ImGui::IsMouseHoveringRect(gutterA, gutterB);
                    renderRichTooltip("security_gutter_" + std::to_string(ln),
                                      securityDiag.message,
                                      hover,
                                      options.reduceMotion,
                                      font);
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
                    bool hoverSuggestion = ImGui::IsMouseHoveringRect(a, b);
                    if (hoverSuggestion) {
                        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                            result.suggestionClicked = true;
                            result.clickedSuggestion = suggestion;
                        }
                    }
                    if (hoverSuggestion) {
                        std::string tip = suggestion.label + " (" +
                            std::to_string(suggestion.confidence) + ")\n" +
                            suggestion.reason;
                        renderRichTooltip("suggestion_marker_" + std::to_string(ln),
                                          tip,
                                          hoverSuggestion,
                                          options.reduceMotion,
                                          font);
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
                    bool hoverConflict = ImGui::IsMouseHoveringRect(a, b);
                    renderRichTooltip("conflict_marker_" + std::to_string(ln),
                                      conflict.message,
                                      hoverConflict,
                                      options.reduceMotion,
                                      font);
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
            if (scopeStartLine >= 0 && ln >= scopeStartLine && ln <= scopeEndLine) {
                ImVec2 hlA(textBase.x, y);
                ImVec2 hlB(textBase.x + textAreaWidth, y + lineHeight);
                drawList->AddRectFilled(hlA, hlB,
                                        ThemeEngine::instance().editorColor("scope_highlight",
                                                                            IM_COL32(60, 60, 90, 40)));
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
            if (options.searchPulseLine >= 0 && ln == options.searchPulseLine) {
                float pulse = AnimationUtils::pulseAlpha(nowSeconds,
                                                         options.searchPulseStart,
                                                         0.6f,
                                                         options.reduceMotion);
                if (pulse > 0.0f) {
                    ImU32 base = ThemeEngine::instance().editorColor("search_pulse",
                                                                     IM_COL32(240, 200, 80, 160));
                    ImVec4 c = ImGui::ColorConvertU32ToFloat4(base);
                    c.w *= pulse;
                    ImU32 col = ImGui::ColorConvertFloat4ToU32(c);
                    ImVec2 hlA(textBase.x, y);
                    ImVec2 hlB(textBase.x + textAreaWidth, y + lineHeight);
                    drawList->AddRectFilled(hlA, hlB, col);
                }
            }

            // Selection background
            for (const auto& c : cursors_) {
                if (c.selStart < 0 || c.selEnd < 0 || c.selStart == c.selEnd) continue;
                int selA = std::min(c.selStart, c.selEnd);
                int selB = std::max(c.selStart, c.selEnd);
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

            // Bracket pair highlight
            if (focusBracketPos >= 0 && focusBracketMatch >= 0) {
                int matchPositions[2] = {focusBracketPos, focusBracketMatch};
                for (int i = 0; i < 2; ++i) {
                    int pos = matchPositions[i];
                    if (pos < start || pos >= end) continue;
                    int col = pos - start;
                    ImVec2 a(textBase.x + col * charAdvance, y);
                    ImVec2 b(textBase.x + (col + 1) * charAdvance, y + lineHeight);
                    drawList->AddRectFilled(a, b,
                                            ThemeEngine::instance().editorColor("bracket_match",
                                                                                IM_COL32(120, 140, 200, 100)));
                }
            }

            // Render text with colors
            int pos = start;
            while (pos < end) {
                TokenCategory cat = TokenCategory::Plain;
                if (pos < (int)charCats.size()) cat = charCats[pos];

                if (pos < (int)text.size() && isBracketChar(text[pos]) && bracketDepth[pos] >= 0) {
                    ImU32 col = rainbow[bracketDepth[pos] % rainbow.size()];
                    char buf[2] = {text[pos], 0};
                    ImVec2 p(textBase.x + (pos - start) * charAdvance, y);
                    drawList->AddText(font, font->FontSize, p, col, buf);
                    ++pos;
                    continue;
                }

                int spanEnd = pos + 1;
                while (spanEnd < end) {
                    if (spanEnd < (int)text.size() && isBracketChar(text[spanEnd]) && bracketDepth[spanEnd] >= 0) {
                        break;
                    }
                    TokenCategory nextCat = TokenCategory::Plain;
                    if (spanEnd < (int)charCats.size()) nextCat = charCats[spanEnd];
                    if (nextCat != cat) break;
                    ++spanEnd;
                }

                std::string chunk = text.substr(pos, spanEnd - pos);
                if (options.showWhitespace) {
                    for (auto& c : chunk) {
                        if (c == ' ') c = '.';
                        else if (c == '	') c = '>';
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
                    bool hoverDiag = !msg.empty();
                    if (hoverDiag) {
                        renderRichTooltip("diag_point",
                                          msg,
                                          hoverDiag,
                                          options.reduceMotion,
                                          font);
                    }
                }
            }
        }
    }
