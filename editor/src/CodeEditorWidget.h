#pragma once
// Step 77: Custom code editor renderer
//
// Immediate-mode code editor widget that renders text with syntax colors,
// handles cursor/selection/input, and supports a visible whitespace toggle.
// This is intentionally minimal; line numbers, gutters, folding, and
// advanced selection behaviors are added in later steps.

#include "imgui.h"
#include "SyntaxHighlighter.h"
#include "EditorMode.h"
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <cstring>

struct CodeEditorOptions {
    bool showWhitespace = false;
    bool readOnly = false;
    EditorMode* mode = nullptr;
    bool enableFolding = false;
    bool showMinimap = false;
    bool showAnnotations = false;
    bool showCurrentLine = true;
    int annotationLayout = 0; // 0=above, 1=margin, 2=beside
    const std::vector<int>* errorLines = nullptr;
    const std::vector<int>* warningLines = nullptr;
    const std::vector<struct DiagnosticRange>* diagnostics = nullptr;
    const std::vector<struct AnnotationMarker>* annotations = nullptr;
    const std::vector<struct SuggestionMarker>* suggestions = nullptr;
    const std::vector<struct AnnotationConflictMarker>* conflicts = nullptr;
    int highlightLine = -1;
    const std::vector<int>* highlightLines = nullptr;
    ImU32 highlightLineColor = IM_COL32(90, 70, 30, 120);
    float* syncScrollX = nullptr;
    float* syncScrollY = nullptr;
    bool scrollMaster = false;
};

struct CodeEditorResult {
    bool changed = false;
    int cursorByte = 0;
    int lineCount = 0;
    float gutterWidth = 0.0f;
    int currentLine = 0;
    int foldCount = 0;
    bool anyFolded = false;
    bool minimapEnabled = false;
    float minimapWidth = 0.0f;
    float minimapViewportStart = 0.0f;
    float minimapViewportEnd = 0.0f;
    bool suggestionClicked = false;
    struct SuggestionMarker clickedSuggestion;
    bool lineClicked = false;
    int clickedLine = -1;
    bool ctrlClick = false;
    int ctrlClickLine = -1;
    int ctrlClickCol = -1;
    int ctrlClickPos = -1;
    bool hoverValid = false;
    int hoverLine = -1;
    int hoverCol = -1;
    int hoverPos = -1;
};

struct DiagnosticRange {
    int startLine = 0;
    int startCol = 0;
    int endLine = 0;
    int endCol = 0;
    int severity = 0; // 1=error, 2=warning
    std::string message;
};

struct AnnotationMarker {
    int line = 0;
    ImU32 color = 0;
    std::string message;
};

struct SuggestionMarker {
    int line = 0;
    double confidence = 0.0;
    std::string label;
    std::string reason;
    std::string annotationType;
    std::string strategy;
    std::string nodeId;
};

struct AnnotationConflictMarker {
    int childLine = -1;
    int parentLine = -1;
    std::string message;
    std::string childAnnoId;
    std::string parentAnnoId;
    std::string parentStrategy;
};

struct FoldRegion {
    int startLine = 0;
    int endLine = 0;
    bool folded = false;
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
        const float gutterWidth = calcGutterWidth(lineCount, font, charAdvance);
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
            drawList->AddRectFilled(gutterA, gutterB, IM_COL32(20, 20, 20, 255));

            // Line number (right-aligned)
            char numBuf[16];
            snprintf(numBuf, sizeof(numBuf), "%d", ln + 1);
            ImVec2 numSize = font->CalcTextSizeA(font->FontSize, FLT_MAX, -1.0f, numBuf);
            ImVec2 numPos(origin.x + gutterWidth - 4.0f - numSize.x, y);
            drawList->AddText(font, font->FontSize, numPos, IM_COL32(120, 120, 120, 255), numBuf);

            // Diagnostics marker
            bool hasError = lineIn(options.errorLines, ln);
            bool hasWarn = lineIn(options.warningLines, ln);
            if (hasError || hasWarn) {
                ImU32 color = hasError ? IM_COL32(220, 80, 80, 255) : IM_COL32(220, 160, 60, 255);
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
                    ImU32 color = IM_COL32(240, 200, 40, 255);
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
                    drawList->AddCircle(center, 5.0f, IM_COL32(220, 80, 80, 255), 12, 1.5f);
                    if (ln == std::min(conflict.childLine, conflict.parentLine) &&
                        conflict.childLine >= 0 && conflict.parentLine >= 0) {
                        float y1 = textBase.y + conflict.childLine * lineHeight + lineHeight * 0.5f;
                        float y2 = textBase.y + conflict.parentLine * lineHeight + lineHeight * 0.5f;
                        drawList->AddLine(ImVec2(origin.x + 12.0f, y1),
                                          ImVec2(origin.x + 12.0f, y2),
                                          IM_COL32(220, 80, 80, 180), 1.0f);
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
                    drawTriangle(drawList, triCenter, IM_COL32(160, 160, 160, 255), true);
                } else {
                    drawTriangle(drawList, triCenter, IM_COL32(160, 160, 160, 255), false);
                }
            }

            // Current line highlight (text area only)
            if (options.showCurrentLine && ln == currentLine) {
                ImVec2 hlA(textBase.x, y);
                ImVec2 hlB(textBase.x + textAreaWidth, y + lineHeight);
                drawList->AddRectFilled(hlA, hlB, IM_COL32(40, 40, 40, 120));
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
                    drawList->AddRectFilled(bgA, bgB, IM_COL32(30, 30, 30, 220), 2.0f);
                    drawList->AddText(font, font->FontSize, ImVec2(tagPos.x + 2.0f, tagPos.y),
                                      marker.color, marker.message.c_str());
                }
            }

            // Folded placeholder
            if (fold && fold->folded) {
                ImVec2 p(textBase.x + len * charAdvance + 6.0f, y);
                drawList->AddText(font, font->FontSize, p, IM_COL32(140, 140, 140, 255), "{...}");
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
                drawList->AddLine(ImVec2(x, y), ImVec2(x, y + lineHeight), IM_COL32(240, 240, 240, 255), 1.0f);
            }
        }

        // Minimap
        if (options.showMinimap && minimapWidth > 0.0f) {
            float miniHeight = lineCount * lineHeight;
            ImVec2 miniA(minimapBaseX, minimapBaseY);
            ImVec2 miniB(minimapBaseX + minimapWidth, minimapBaseY + miniHeight);
            drawList->AddRectFilled(miniA, miniB, IM_COL32(18, 18, 18, 255));
            for (int ln = 0; ln < lineCount; ++ln) {
                float y = minimapBaseY + ln * lineHeight;
                ImU32 color = IM_COL32(80, 80, 80, 255);
                if (ln == currentLine) color = IM_COL32(120, 120, 160, 255);
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
            drawList->AddRect(vA, vB, IM_COL32(120, 160, 220, 180));
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

    static int lineFromMouseY(float mouseY, float baseY, float lineHeight, int lineCount) {
        int line = (int)((mouseY - baseY) / lineHeight);
        line = std::max(0, std::min(line, lineCount - 1));
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

    static float calcGutterWidth(int lineCount, ImFont* font, float charAdvance) {
        int digits = 1;
        int n = std::max(1, lineCount);
        while (n >= 10) { n /= 10; ++digits; }
        const float pad = 6.0f;
        float digitsWidth = font->CalcTextSizeA(font->FontSize, FLT_MAX, -1.0f,
                                                std::string(digits, '0').c_str()).x;
        float width = digitsWidth + pad * 2.0f;
        return std::max(width, charAdvance * 2.0f + pad * 2.0f);
    }

    const FoldRegion* findFoldAtLine(int line) const {
        for (const auto& f : folds_) {
            if (f.startLine == line) return &f;
        }
        return nullptr;
    }

    static void drawTriangle(ImDrawList* drawList, const ImVec2& center, ImU32 color, bool down) {
        float s = 4.0f;
        if (down) {
            drawList->AddTriangleFilled(
                ImVec2(center.x - s, center.y - s * 0.6f),
                ImVec2(center.x + s, center.y - s * 0.6f),
                ImVec2(center.x, center.y + s),
                color);
        } else {
            drawList->AddTriangleFilled(
                ImVec2(center.x - s, center.y - s),
                ImVec2(center.x - s, center.y + s),
                ImVec2(center.x + s, center.y),
                color);
        }
    }

    static bool lineIn(const std::vector<int>* lines, int line) {
        if (!lines) return false;
        return std::find(lines->begin(), lines->end(), line) != lines->end();
    }

    static bool annotationAtLine(const std::vector<AnnotationMarker>& markers,
                                 int line,
                                 AnnotationMarker& out) {
        for (const auto& m : markers) {
            if (m.line == line) {
                out = m;
                return true;
            }
        }
        return false;
    }

    static bool suggestionAtLine(const std::vector<SuggestionMarker>& markers,
                                 int line,
                                 SuggestionMarker& out) {
        for (const auto& m : markers) {
            if (m.line == line) {
                out = m;
                return true;
            }
        }
        return false;
    }

    static bool conflictAtLine(const std::vector<AnnotationConflictMarker>& markers,
                               int line,
                               AnnotationConflictMarker& out) {
        for (const auto& m : markers) {
            if (m.childLine == line || m.parentLine == line) {
                out = m;
                return true;
            }
        }
        return false;
    }

    static std::string diagnosticMessageAtLine(const std::vector<DiagnosticRange>& diags, int line) {
        for (const auto& d : diags) {
            if (line >= d.startLine && line <= d.endLine) return d.message;
        }
        return "";
    }

    static void renderSquiggles(const std::vector<DiagnosticRange>& diags,
                                int line,
                                float y,
                                float lineHeight,
                                float charAdvance,
                                float textBaseX,
                                int lineLen,
                                ImDrawList* drawList) {
        float baseY = y + lineHeight - 2.0f;
        for (const auto& d : diags) {
            if (line < d.startLine || line > d.endLine) continue;
            int startCol = (line == d.startLine) ? d.startCol : 0;
            int endCol = (line == d.endLine) ? d.endCol : lineLen;
            startCol = std::max(0, startCol);
            endCol = std::max(startCol, endCol);
            float xStart = textBaseX + startCol * charAdvance;
            float xEnd = textBaseX + endCol * charAdvance;
            ImU32 color = (d.severity == 1) ? IM_COL32(220, 80, 80, 255) : IM_COL32(220, 160, 60, 255);
            float x = xStart;
            float amp = 2.0f;
            bool up = true;
            while (x < xEnd) {
                float x2 = std::min(x + 4.0f, xEnd);
                float y1 = baseY + (up ? -amp : amp);
                float y2 = baseY + (up ? amp : -amp);
                drawList->AddLine(ImVec2(x, y1), ImVec2(x2, y2), color, 1.0f);
                up = !up;
                x = x2;
            }
        }
    }

    static std::string diagnosticMessageAtPoint(const std::vector<DiagnosticRange>& diags,
                                                int line,
                                                const ImVec2& mouse,
                                                float y,
                                                float lineHeight,
                                                float charAdvance,
                                                float textBaseX) {
        if (mouse.y < y || mouse.y > y + lineHeight) return "";
        for (const auto& d : diags) {
            if (line < d.startLine || line > d.endLine) continue;
            int startCol = (line == d.startLine) ? d.startCol : 0;
            int endCol = (line == d.endLine) ? d.endCol : startCol + 1;
            float xStart = textBaseX + startCol * charAdvance;
            float xEnd = textBaseX + endCol * charAdvance;
            if (mouse.x >= xStart && mouse.x <= xEnd) return d.message;
        }
        return "";
    }

    void updateFolds(const std::string& text, const std::string& language) {
        if (text == lastFoldText_ && language == lastFoldLang_) return;
        lastFoldText_ = text;
        lastFoldLang_ = language;

        std::vector<FoldRegion> newFolds;
        if (text.empty()) { folds_.clear(); return; }

        const TSLanguage* lang = nullptr;
        if (language == "python") lang = tree_sitter_python();
        else if (language == "cpp") lang = tree_sitter_cpp();
        else if (language == "elisp") lang = tree_sitter_elisp();
        if (!lang) { folds_.clear(); return; }

        TSParser* parser = ts_parser_new();
        ts_parser_set_language(parser, lang);
        TSTree* tree = ts_parser_parse_string(parser, nullptr, text.c_str(), (uint32_t)text.size());
        TSNode root = ts_tree_root_node(tree);

        collectFoldNodes(root, language, newFolds);

        // Preserve folded state by startLine
        for (auto& f : newFolds) {
            for (const auto& old : folds_) {
                if (old.startLine == f.startLine && old.endLine == f.endLine) {
                    f.folded = old.folded;
                    break;
                }
            }
        }

        folds_ = std::move(newFolds);
        if (applyDesiredFoldState_) {
            for (auto& f : folds_) {
                f.folded = std::find(desiredFoldedLines_.begin(),
                                     desiredFoldedLines_.end(),
                                     f.startLine) != desiredFoldedLines_.end();
            }
            applyDesiredFoldState_ = false;
        }

        ts_tree_delete(tree);
        ts_parser_delete(parser);
    }

    static bool isFoldNodeType(const std::string& language, const char* type) {
        if (language == "python") {
            return strcmp(type, "function_definition") == 0 ||
                   strcmp(type, "class_definition") == 0 ||
                   strcmp(type, "if_statement") == 0 ||
                   strcmp(type, "for_statement") == 0 ||
                   strcmp(type, "while_statement") == 0 ||
                   strcmp(type, "with_statement") == 0 ||
                   strcmp(type, "try_statement") == 0;
        } else if (language == "cpp") {
            return strcmp(type, "function_definition") == 0 ||
                   strcmp(type, "class_specifier") == 0 ||
                   strcmp(type, "struct_specifier") == 0 ||
                   strcmp(type, "namespace_definition") == 0 ||
                   strcmp(type, "if_statement") == 0 ||
                   strcmp(type, "for_statement") == 0 ||
                   strcmp(type, "while_statement") == 0 ||
                   strcmp(type, "compound_statement") == 0;
        } else if (language == "elisp") {
            return strcmp(type, "function_definition") == 0 ||
                   strcmp(type, "lambda_expression") == 0 ||
                   strcmp(type, "if_expression") == 0 ||
                   strcmp(type, "while_expression") == 0 ||
                   strcmp(type, "cond_expression") == 0;
        }
        return false;
    }

    static void collectFoldNodes(TSNode node, const std::string& language, std::vector<FoldRegion>& out) {
        if (ts_node_is_null(node)) return;
        const char* type = ts_node_type(node);
        if (isFoldNodeType(language, type)) {
            TSPoint start = ts_node_start_point(node);
            TSPoint end = ts_node_end_point(node);
            if (end.row > start.row) {
                FoldRegion f;
                f.startLine = (int)start.row;
                f.endLine = (int)end.row;
                f.folded = false;
                out.push_back(f);
            }
        }
        uint32_t count = ts_node_child_count(node);
        for (uint32_t i = 0; i < count; ++i) {
            collectFoldNodes(ts_node_child(node, i), language, out);
        }
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
};
