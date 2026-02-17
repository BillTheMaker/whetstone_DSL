#pragma once

#include "../EditorState.h"
#include "../EditorUtils.h"
#include "../CompletionUtils.h"
#include "../AnimationUtils.h"
#include "../RichTooltip.h"
#include "../ast/Function.h"
#include <map>

static void renderEditorPanel(EditorState& state) {
        if (state.ui.focusTarget == FocusRegion::Editor) {
            ImGui::SetNextWindowFocus();
            state.ui.focusTarget = FocusRegion::None;
        }
        if (state.ui.showAnnotations) {
            queueFeatureHint(state.featureHints,
                             "hint.annotations",
                             "Tip: Right-click a function to add memory annotations.");
        }
        // ---------------------------------------------------------------
        //  Editor (center) -- editable text area
        // ---------------------------------------------------------------
        ImGui::Begin("Editor");
        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) {
            state.ui.focusedRegion = FocusRegion::Editor;
        }
        ImGui::PushFont(state.uiFont);
        ImGui::BeginChild("##breadcrumbs", ImVec2(0, 26.0f), false, ImGuiWindowFlags_NoScrollbar);
        if (!state.active()) {
            ImGui::TextDisabled("(no file)");
        } else if (!state.isStructured()) {
            ImGui::TextDisabled("Text mode");
        } else {
            Module* ast = state.activeAST();
            int lineZero = std::max(0, state.active()->cursorLine - 1);
            int colZero = std::max(0, state.active()->cursorCol - 1);
            ASTNode* node = ast ? findNodeAtPosition(ast, lineZero, colZero) : nullptr;
            if (!node) {
                ImGui::TextDisabled("(no scope)");
            } else {
                auto crumbs = buildBreadcrumbTrail(node);
                for (size_t i = 0; i < crumbs.size(); ++i) {
                    if (i > 0) {
                        ImGui::SameLine();
                        ImGui::TextUnformatted(">");
                        ImGui::SameLine();
                    }
                    ImGui::PushID((int)i);
                    const auto& item = crumbs[i];
                    if (item.isRole) {
                        ImGui::TextDisabled("%s", item.label.c_str());
                    } else if (item.node && item.node->hasSpan()) {
                        if (ImGui::SmallButton(item.label.c_str())) {
                            state.jumpTo(state.active(), item.node->spanStartLine, item.node->spanStartCol);
                        }
                    } else {
                        ImGui::TextUnformatted(item.label.c_str());
                    }
                    ImGui::PopID();
                }
            }
        }
        ImGui::EndChild();
        ImGui::PopFont();

        ImGui::PushFont(state.monoFont);

        // Tab bar for the file
        static bool renameOpen = false;
        static char renameBuf[260] = {};
        static std::string renameTarget;
        if (ImGui::BeginTabBar("EditorTabs",
                               ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_AutoSelectNewTabs)) {
            if (state.buffers.bufferCount() == 0) {
                if (ImGui::BeginTabItem("Welcome")) {
                    RenderWelcome(state.welcome, state, state.lastDialogPath);
                    ImGui::EndTabItem();
                }
            } else {
                std::map<std::string, std::pair<int, int>> diagCounts;
                if (state.lsp) {
                    for (const auto& d : state.lsp->getDiagnostics()) {
                        std::string path = EditorState::fromFileUri(d.uri);
                        auto& counts = diagCounts[path];
                        if (d.severity == 1) counts.first++;
                        else if (d.severity == 2) counts.second++;
                    }
                }
                for (const auto& d : state.whetstoneDiagnostics) {
                    std::string path = EditorState::fromFileUri(d.uri);
                    auto& counts = diagCounts[path];
                    if (d.severity == 1) counts.first++;
                    else if (d.severity == 2) counts.second++;
                }
                auto openBuffers = state.buffers.getOpenBuffers();
                for (const auto& path : openBuffers) {
                    auto* buf = state.bufferStates[path].get();
                    if (!buf) continue;
                    std::string tabLabel = path;
                    if (buf->modified) tabLabel += " *";
                    auto it = diagCounts.find(path);
                    if (it != diagCounts.end()) {
                        int err = it->second.first;
                        int warn = it->second.second;
                        if (err > 0) tabLabel += " [E" + std::to_string(err) + "]";
                        else if (warn > 0) tabLabel += " [W" + std::to_string(warn) + "]";
                    }
                    bool open = true;
                    if (ImGui::BeginTabItem(tabLabel.c_str(), &open)) {
                        if (!state.active() || state.active()->path != path) {
                            state.switchToBuffer(path);
                        }
                        const double nowSeconds = ImGui::GetTime();
                        float tabAlpha = state.uiAnimations.tabFadeAlpha(path,
                                                                         nowSeconds,
                                                                         state.settings.getReduceMotion());
                        ImGui::PushStyleVar(ImGuiStyleVar_Alpha,
                                            ImGui::GetStyle().Alpha * tabAlpha);
                        // Editable text area fills available space
                        ImVec2 avail = ImGui::GetContentRegionAvail();
                        avail.y -= 4; // small margin

                        state.updateHighlights();
                        if (buf->bufferMode == BufferManager::BufferMode::Structured) {
                            state.updateGenerated();
                        }
                        std::vector<int> errorLines;
                        std::vector<int> warningLines;
                        std::vector<DiagnosticRange> diagRanges;
                        std::vector<AnnotationMarker> annoMarkers;
                        std::vector<SuggestionMarker> suggestionMarkers;
                        std::vector<AnnotationConflictMarker> conflictMarkers;
                        std::string activeUri = EditorState::toFileUri(buf->path);
                        if (state.lsp) {
                            auto lspDiags = state.lsp->getDiagnosticsForUri(activeUri);
                            for (const auto& d : lspDiags) {
                                if (d.severity == 1) errorLines.push_back(d.range.start.line);
                                else if (d.severity == 2) warningLines.push_back(d.range.start.line);
                                DiagnosticRange dr;
                                dr.startLine = d.range.start.line;
                                dr.startCol = d.range.start.character;
                                dr.endLine = d.range.end.line;
                                dr.endCol = d.range.end.character;
                                dr.severity = d.severity;
                                dr.message = d.message;
                                diagRanges.push_back(std::move(dr));
                            }
                        }
                        for (const auto& d : state.whetstoneDiagnostics) {
                            if (d.uri != activeUri) continue;
                            if (d.severity == 1) errorLines.push_back(d.line);
                            else if (d.severity == 2) warningLines.push_back(d.line);
                            DiagnosticRange dr;
                            dr.startLine = d.line;
                            dr.startCol = d.character;
                            dr.endLine = d.line;
                            dr.endCol = d.character + 1;
                            dr.severity = d.severity;
                            dr.message = d.message;
                            diagRanges.push_back(std::move(dr));
                        }
                        if (state.isStructured()) {
                            Module* ast = state.activeAST();
                            if (ast) {
                                collectAnnotationMarkers(ast, annoMarkers);
                                std::vector<AnnotationConflict> conflicts;
                                collectAnnotationConflicts(ast, conflicts);
                                for (const auto& c : conflicts) {
                                    AnnotationConflictMarker m;
                                    m.childLine = c.childLine;
                                    m.parentLine = c.parentLine;
                                    m.message = c.message;
                                    m.childAnnoId = c.childAnnoId;
                                    m.parentAnnoId = c.parentAnnoId;
                                    m.parentStrategy = c.parentStrategy;
                                    conflictMarkers.push_back(std::move(m));
                                }
                                for (const auto& s : state.suggestions) {
                                    if (s.confidence <= 0.5) continue;
                                    ASTNode* target = findNodeById(ast, s.nodeId);
                                    if (!target || !target->hasSpan()) continue;
                                    SuggestionMarker sm;
                                    sm.line = target->spanStartLine;
                                    sm.confidence = s.confidence;
                                    sm.reason = s.reason;
                                    sm.annotationType = s.annotationType;
                                    sm.strategy = s.strategy;
                                    sm.nodeId = s.nodeId;
                                    sm.label = "@" + s.annotationType.substr(0, s.annotationType.find("Annotation")) +
                                               "(" + s.strategy + ")";
                                    suggestionMarkers.push_back(std::move(sm));
                                }
                            }
                        }

                        CodeEditorResult res;
                        if (buf->language == "org") {
                            renderOrgDocument(state.emacsState.orgDoc,
                                              buf->editBuf,
                                              state.ui.showWhitespace,
                                              state.monoFont,
                                              state.uiFont,
                                              state.settings.getLineHeightScale(),
                                              state.settings.getLetterSpacing(),
                                              [&](const std::string& newText) {
                                                  buf->editBuf = newText;
                                                  state.onTextChanged();
                                              },
                                              [&](int, const std::string&, const std::string&) {
                                                  buf->modified = true;
                                              },
                                              [&](const std::string& lang, const std::string& code) {
                                                  return state.runOrgBlock(lang, code);
                                              });
                            res.cursorByte = buf->widget.getCursor();
                        } else {
                            CodeEditorOptions opts;
                            opts.showWhitespace = state.ui.showWhitespace;
                            opts.readOnly = buf->readOnly;
                            opts.mode = &buf->mode;
                            opts.enableFolding = true;
                            opts.showMinimap = state.ui.showMinimap;
                            opts.showAnnotations = state.ui.showAnnotations;
                            opts.showLineNumbers = state.ui.showLineNumbers;
                            opts.useAnnotationShapes = state.settings.getUseAnnotationShapes();
                            opts.lineHeightScale = state.settings.getLineHeightScale();
                            opts.letterSpacing = state.settings.getLetterSpacing();
                            opts.cursorBlinkRate = state.settings.getCursorBlinkRate();
                            opts.reduceMotion = state.settings.getReduceMotion();
                            opts.nowSeconds = nowSeconds;
                            if (state.ui.layoutPreset == LayoutPreset::Emacs) opts.annotationLayout = 1;
                            else if (state.ui.layoutPreset == LayoutPreset::JetBrains) opts.annotationLayout = 2;
                            else opts.annotationLayout = 0;
                            opts.errorLines = &errorLines;
                            opts.warningLines = &warningLines;
                            opts.diagnostics = &diagRanges;
                            opts.annotations = &annoMarkers;
                            opts.suggestions = &suggestionMarkers;
                            opts.conflicts = &conflictMarkers;
                            opts.searchPulseLine = state.search.pulseLine;
                            opts.searchPulseStart = state.search.pulseStart;

                            if (buf->bufferMode == BufferManager::BufferMode::Structured) {
                                opts.syncScrollX = &buf->splitScrollX;
                                opts.syncScrollY = &buf->splitScrollY;
                                opts.scrollMaster = true;

                                ImGui::BeginTable("##editorSplit", 2,
                                                  ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp);
                                ImGui::TableSetupColumn("Source", ImGuiTableColumnFlags_WidthStretch, 0.55f);
                                ImGui::TableSetupColumn("Generated", ImGuiTableColumnFlags_WidthStretch, 0.45f);
                                ImGui::TableNextRow();
                                ImGui::TableSetColumnIndex(0);
                                res = buf->widget.render("##editor",
                                    buf->editBuf, buf->highlights, opts, ImVec2(0, avail.y), state.monoFont);

                                ImGui::TableSetColumnIndex(1);
                                ImGui::BeginGroup();
                                ImGui::AlignTextToFramePadding();
                                ImGui::TextUnformatted("Generated");
                                ImGui::SameLine();
                                const char* targetLabels[] = {"Python", "C++", "Elisp"};
                                const char* targetValues[] = {"python", "cpp", "elisp"};
                                int targetIndex = 0;
                                for (int i = 0; i < IM_ARRAYSIZE(targetValues); ++i) {
                                    if (buf->generatedLanguage == targetValues[i]) {
                                        targetIndex = i;
                                        break;
                                    }
                                }
                                ImGui::SetNextItemWidth(120.0f);
                                if (ImGui::Combo("##targetLang", &targetIndex,
                                                 targetLabels, IM_ARRAYSIZE(targetLabels))) {
                                    buf->generatedLanguage = targetValues[targetIndex];
                                    buf->generatedMode.setLanguage(buf->generatedLanguage);
                                    buf->generatedHighlightsDirty = true;
                                    state.updateGenerated();
                                }
                                ImVec2 genAvail = ImGui::GetContentRegionAvail();
                                CodeEditorOptions genOpts;
                                genOpts.readOnly = true;
                                genOpts.showWhitespace = state.ui.showWhitespace;
                                genOpts.mode = &buf->generatedMode;
                                genOpts.showLineNumbers = state.ui.showLineNumbers;
                                genOpts.showCurrentLine = false;
                                genOpts.lineHeightScale = state.settings.getLineHeightScale();
                                genOpts.letterSpacing = state.settings.getLetterSpacing();
                                genOpts.cursorBlinkRate = state.settings.getCursorBlinkRate();
                                genOpts.reduceMotion = state.settings.getReduceMotion();
                                genOpts.nowSeconds = nowSeconds;
                                genOpts.highlightLine = buf->generatedHighlightLine;
                                genOpts.syncScrollX = &buf->splitScrollX;
                                genOpts.syncScrollY = &buf->splitScrollY;
                                genOpts.scrollMaster = false;
                                buf->generatedWidget.render("##generated",
                                    buf->generatedBuf, buf->generatedHighlights, genOpts,
                                    ImVec2(0, genAvail.y), state.monoFont);
                                ImGui::EndGroup();
                                ImGui::EndTable();
                            } else {
                                res = buf->widget.render("##editor",
                                    buf->editBuf, buf->highlights, opts, avail, state.monoFont);
                            }
                        }

                        if (buf->language != "org") {
                            state.updateCursorPos(res.cursorByte);
                            if (res.ctrlClick && state.active()) {
                                state.goToDefinitionAt(res.ctrlClickLine, res.ctrlClickCol);
                            }
                            if (res.lineClicked && buf->bufferMode == BufferManager::BufferMode::Structured) {
                                Module* ast = state.activeAST();
                                if (ast) {
                                    int genLines = countLines(buf->generatedBuf);
                                    int targetLine = std::max(0, std::min(res.clickedLine, genLines - 1));
                                    buf->generatedHighlightLine = targetLine;
                                } else {
                                    buf->generatedHighlightLine = -1;
                                }
                            }
                            if (res.changed) {
                                state.onTextChanged();
                                if (state.ui.showCompletionHelper) {
                                    state.completionPending = true;
                                    state.completionLastChange = ImGui::GetTime();
                                    state.completionVisible = false;
                                    state.completionDismissed = false;
                                    state.completionSelected = 0;
                                    if (state.lsp) state.lsp->clearCompletionItems();
                                } else {
                                    state.completionPending = false;
                                    state.completionVisible = false;
                                    state.completionDismissed = false;
                                    if (state.lsp) state.lsp->clearCompletionItems();
                                }
                                if (state.lsp && state.active() && state.active()->path.rfind("(untitled", 0) != 0) {
                                    int cursor = state.active()->widget.getCursor();
                                    if (cursor > 0 && state.active()->editBuf[cursor - 1] == '(') {
                                        int lineZero = std::max(0, state.active()->cursorLine - 1);
                                        int colZero = std::max(0, state.active()->cursorCol - 1);
                                        state.lsp->requestSignatureHelp(EditorState::toFileUri(state.active()->path),
                                                                        lineZero, colZero);
                                    } else if (cursor > 0 && state.active()->editBuf[cursor - 1] == ')') {
                                        state.lsp->clearSignatureHelp();
                                    }
                                }
                                if (buf->bufferMode == BufferManager::BufferMode::Structured) {
                                    state.analysisPending = true;
                                    state.analysisLastChange = ImGui::GetTime();
                                }
                            }
                        }

                        if (res.suggestionClicked) {
                            state.suggestionPopup.annotationType = res.clickedSuggestion.annotationType;
                            state.suggestionPopup.strategy = res.clickedSuggestion.strategy;
                            state.suggestionPopup.reason = res.clickedSuggestion.reason;
                            state.suggestionPopup.confidence = res.clickedSuggestion.confidence;
                            state.suggestionPopup.nodeId = res.clickedSuggestion.nodeId;
                            state.showSuggestionPopup = true;
                            ImGui::OpenPopup("SuggestionPopup");
                        }

                        double now = ImGui::GetTime();
                        if (state.ui.showCompletionHelper &&
                            state.completionPending &&
                            (now - state.completionLastChange) > 0.2) {
                            if (state.lsp && state.active() && state.active()->path.rfind("(untitled", 0) != 0) {
                                int lineZero = std::max(0, state.active()->cursorLine - 1);
                                int colZero = std::max(0, state.active()->cursorCol - 1);
                                state.lsp->requestCompletion(EditorState::toFileUri(state.active()->path),
                                                             lineZero, colZero);
                            }
                            state.completionPending = false;
                        }

                        if (state.symbolsPending && (now - state.symbolsLastChange) > 0.3) {
                            if (state.lsp && state.active() && state.active()->path.rfind("(untitled", 0) != 0) {
                                state.lsp->requestDocumentSymbols(EditorState::toFileUri(state.active()->path));
                            }
                            state.symbolsPending = false;
                        }

                        double diagDelay = state.settings.getDiagnosticsDebounceMs() / 1000.0;
                        if (state.analysisPending && (now - state.analysisLastChange) > diagDelay) {
                            if (state.isStructured()) {
                                auto result = state.pipeline.run(state.active()->editBuf,
                                                                 state.active()->language,
                                                                 state.active()->language);
                                if (result.success) {
                                    state.whetstoneDiagnostics =
                                        collectWhetstoneDiagnostics(result.validationDiags,
                                                                    result.violations,
                                                                    EditorState::toFileUri(state.active()->path));
                                    state.appendUnusedImportDiagnostics(state.whetstoneDiagnostics);
                                    state.appendVulnerabilityDiagnostics(state.whetstoneDiagnostics);
                                } else {
                                    state.whetstoneDiagnostics.clear();
                                }
                                Module* ast = state.activeAST();
                                if (ast) {
                                    MemoryStrategyInference inf;
                                    state.suggestions = inf.inferAnnotations(ast);
                                } else {
                                    state.suggestions.clear();
                                }
                            } else {
                                state.whetstoneDiagnostics.clear();
                                state.suggestions.clear();
                            }
                            state.analysisPending = false;
                        }

                        if (state.ui.showCompletionHelper && state.lsp && state.active()) {
                            auto items = state.lsp->getCompletionItems();
                            int cursor = state.active()->widget.getCursor();
                            std::string prefix = EditorState::wordPrefixAt(state.active()->editBuf, cursor);
                            std::string nodeId;
                            std::vector<std::string> contextTags;
                            if (state.activeAST()) {
                                ASTNode* scopeNode = findNodeAtPosition(state.activeAST(),
                                                                        std::max(0, state.active()->cursorLine - 1),
                                                                        std::max(0, state.active()->cursorCol - 1));
                                if (scopeNode) {
                                    nodeId = scopeNode->id;
                                    ASTNode* cursorNode = scopeNode;
                                    while (cursorNode && cursorNode->conceptType != "Function") {
                                        cursorNode = cursorNode->parent;
                                    }
                                    if (cursorNode && cursorNode->conceptType == "Function") {
                                        auto* fn = static_cast<Function*>(cursorNode);
                                        contextTags = state.library.semanticTags.inferTagsFromText(fn->name);
                                    }
                                }
                            }
                            state.library.primitives.setContextTags(contextTags);
                            std::vector<PrimitiveSymbol> primitives;
                            auto funcs = state.library.primitives.getAvailableFunctions(nodeId);
                            auto types = state.library.primitives.getAvailableTypes(nodeId);
                            auto consts = state.library.primitives.getAvailableConstants(nodeId);
                            primitives.insert(primitives.end(), funcs.begin(), funcs.end());
                            primitives.insert(primitives.end(), types.begin(), types.end());
                            primitives.insert(primitives.end(), consts.begin(), consts.end());

                            auto built = buildLibraryAwareCompletions(items, primitives, prefix);
                            std::vector<LSPClient::CompletionItem> filtered;
                            filtered.reserve(built.items.size());
                            for (const auto& item : built.items) {
                                const std::string& key = item.filterText.empty() ? item.label : item.filterText;
                                if (prefix.empty() || key.rfind(prefix, 0) == 0) filtered.push_back(item);
                            }

                            if (filtered.empty()) {
                                state.completionVisible = false;
                                state.completionDismissed = false;
                            } else if (!state.completionDismissed) {
                                state.completionVisible = true;
                            }
                            if (state.completionVisible) {
                                ImGui::SetCursorScreenPos(ImVec2(ImGui::GetWindowPos().x + 20.0f,
                                                                 ImGui::GetWindowPos().y + 60.0f));
                                ImGui::BeginChild("##completionPopup", ImVec2(300, 150), true);
                                bool popupHovered =
                                    ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup);
                                int maxIndex = std::max(0, (int)filtered.size() - 1);
                                state.completionSelected = std::max(0, std::min(state.completionSelected, maxIndex));
                                if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
                                    state.completionSelected = std::min(state.completionSelected + 1, maxIndex);
                                }
                                if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
                                    state.completionSelected = std::max(state.completionSelected - 1, 0);
                                }
                                bool accept = ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_Tab);
                                bool dismiss = ImGui::IsKeyPressed(ImGuiKey_Escape);

                                if (dismiss) {
                                    state.lsp->clearCompletionItems();
                                    state.completionVisible = false;
                                    state.completionDismissed = true;
                                }

                                auto deriveLibrary = [](const std::string& name) {
                                    auto pos = name.find("::");
                                    if (pos != std::string::npos) return name.substr(0, pos);
                                    pos = name.find('.');
                                    if (pos != std::string::npos) return name.substr(0, pos);
                                    return std::string();
                                };

                                for (int i = 0; i < (int)filtered.size(); ++i) {
                                    std::string itemLabel = filtered[i].label;
                                    if (filtered[i].kind != 0) {
                                        itemLabel = "[" + std::to_string(filtered[i].kind) + "] " + itemLabel;
                                    }
                                    bool selected = (i == state.completionSelected);
                                    bool preferred = built.preferredNames.count(filtered[i].label) > 0;
                                    std::string libHint = deriveLibrary(filtered[i].label);
                                    if (!preferred && !libHint.empty()) itemLabel += "  (auto-import)";
                                    if (!preferred) {
                                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
                                    }
                                    if (ImGui::Selectable(itemLabel.c_str(), selected)) {
                                        state.completionSelected = i;
                                        accept = true;
                                    }
                                    if (!preferred) {
                                        ImGui::PopStyleColor();
                                    }
                                }

                                if (accept && !filtered.empty()) {
                                    state.completionSelected =
                                        std::max(0, std::min(state.completionSelected,
                                                             (int)filtered.size() - 1));
                                    const auto& item = filtered[state.completionSelected];
                                    bool preferred = built.preferredNames.count(item.label) > 0;
                                    if (!preferred) {
                                        std::string libHint = deriveLibrary(item.label);
                                        if (!libHint.empty()) {
                                            state.ensureImportForSymbol(libHint, item.label);
                                        }
                                    }
                                    int start = EditorState::wordStartAt(state.active()->editBuf, cursor);
                                    state.applyCompletion(state.active(), item.insertText, start, cursor);
                                    state.lsp->clearCompletionItems();
                                    state.completionVisible = false;
                                    state.completionDismissed = false;
                                }
                                ImGui::EndChild();
                                if (ImGui::IsMouseClicked(0) && !popupHovered) {
                                    state.completionVisible = false;
                                    state.completionDismissed = true;
                                }
                            }
                        }
                        if (!state.ui.showCompletionHelper) {
                            state.completionPending = false;
                            state.completionVisible = false;
                            state.completionDismissed = false;
                        }

                        if (state.showSuggestionPopup) {
                            if (ImGui::BeginPopupModal("SuggestionPopup", &state.showSuggestionPopup, ImGuiWindowFlags_AlwaysAutoResize)) {
                                ImGui::Text("%s(%s)", state.suggestionPopup.annotationType.c_str(),
                                            state.suggestionPopup.strategy.c_str());
                                ImGui::Separator();
                                ImGui::TextWrapped("%s", state.suggestionPopup.reason.c_str());
                                ImGui::Text("Confidence: %.2f", state.suggestionPopup.confidence);
                                if (ImGui::Button("Apply")) {
                                    Module* ast = state.mutationAST();
                                    if (ast) {
                                        ASTNode* target = findNodeById(ast, state.suggestionPopup.nodeId);
                                        if (target) {
                                            auto* anno = createAnnotationNode(state.suggestionPopup.annotationType,
                                                                             state.suggestionPopup.strategy);
                                            if (anno) {
                                                if (target->hasSpan()) {
                                                    anno->setSpan(target->spanStartLine, target->spanStartCol,
                                                                  target->spanEndLine, target->spanEndCol);
                                                }
                                                state.mutator.setRoot(ast);
                                                auto res2 = state.mutator.insertNode(target->id, "annotations", anno);
                                                if (!res2.error.empty()) state.notify(NotificationLevel::Error, res2.error);
                                                state.applyOrchestratorToActive();
                                            }
                                        }
                                    }
                                    state.showSuggestionPopup = false;
                                    ImGui::CloseCurrentPopup();
                                }
                                ImGui::SameLine();
                                if (ImGui::Button("Cancel")) {
                                    state.showSuggestionPopup = false;
                                    ImGui::CloseCurrentPopup();
                                }
                                ImGui::EndPopup();
                            }
                        }

                        if (state.lsp && state.active()) {
                            if (ImGui::IsWindowHovered()) {
                                ImVec2 mouse = ImGui::GetMousePos();
                                if (mouse.x != state.hoverLastMouse.x || mouse.y != state.hoverLastMouse.y) {
                                    state.hoverLastMouse = mouse;
                                    state.hoverLastMove = ImGui::GetTime();
                                    state.hoverPending = true;
                                    state.lsp->clearHover();
                                }
                                if (state.hoverPending && (ImGui::GetTime() - state.hoverLastMove) > 0.5) {
                                    if (state.active()->path.rfind("(untitled", 0) != 0) {
                                        int lineZero = std::max(0, state.active()->cursorLine - 1);
                                        int colZero = std::max(0, state.active()->cursorCol - 1);
                                        state.lsp->requestHover(EditorState::toFileUri(state.active()->path),
                                                               lineZero, colZero);
                                    }
                                    state.hoverPending = false;
                                }
                                std::string hover = state.lsp->getHoverContents();
                                static std::string lastHoverText;
                                bool hoverActive = !hover.empty();
                                if (hoverActive) lastHoverText = hover;
                                if (!lastHoverText.empty()) {
                                    renderRichTooltip("lsp_hover",
                                                      lastHoverText,
                                                      hoverActive,
                                                      state.settings.getReduceMotion(),
                                                      state.monoFont);
                                }
                            }

                            if (state.definitionPending) {
                                auto defs = state.lsp->getDefinitionLocations();
                                if (!defs.empty()) {
                                    state.definitionPending = false;
                                    state.definitionPreview = defs.front();
                                    state.definitionPreviewValid = true;
                                    state.lsp->clearDefinitionLocations();
                                    state.jumpToDefinitionLocation(state.definitionPreview);
                                } else if ((ImGui::GetTime() - state.definitionLastRequest) > 0.5) {
                                    state.definitionPending = false;
                                    state.goToDefinitionFallback(state.definitionRequestLine,
                                                                 state.definitionRequestCol);
                                }
                            }

                            auto sig = state.lsp->getSignatureHelp();
                            if (!sig.label.empty()) {
                                ImGui::SetCursorScreenPos(ImVec2(ImGui::GetWindowPos().x + 20.0f,
                                                                 ImGui::GetWindowPos().y + ImGui::GetWindowHeight() - 80.0f));
                                ImGui::BeginChild("##signatureHelp", ImVec2(420, 60), true);
                                ImGui::TextUnformatted(sig.label.c_str());
                                ImGui::EndChild();
                            }
                        }

                        if (res.hoverValid && (ImGui::GetIO().KeyCtrl || ImGui::GetIO().KeySuper)) {
                            std::string preview;
                            if (state.previewDefinitionAt(res.hoverLine, res.hoverCol, preview)) {
                                renderRichTooltip("definition_preview",
                                                  preview,
                                                  true,
                                                  state.settings.getReduceMotion(),
                                                  state.monoFont);
                            } else if (state.definitionPreviewValid) {
                                std::string path = EditorState::fromFileUri(state.definitionPreview.uri);
                                std::string label = path + ":" +
                                    std::to_string(state.definitionPreview.range.start.line + 1);
                                renderRichTooltip("definition_preview_fallback",
                                                  label,
                                                  true,
                                                  state.settings.getReduceMotion(),
                                                  state.monoFont);
                            }
                        }

                        if (ImGui::BeginPopupContextWindow("AnnotationContext", ImGuiPopupFlags_MouseButtonRight)) {
                            Module* ast = state.mutationAST();
                            int lineZero = state.active() ? std::max(0, state.active()->cursorLine - 1) : 0;
                            ASTNode* target = ast ? findAnnotationTarget(ast, lineZero) : nullptr;
                            if (!target) {
                                ImGui::TextDisabled("No annotatable symbol on this line.");
                            } else {
                                bool mutated = false;
                                ImGui::Text("Target: %s", nodeDisplayName(target).c_str());
                                ImGui::Separator();

                                if (ImGui::BeginMenu("Add Annotation")) {
                                    if (ImGui::MenuItem("@Reclaim(Tracing)")) {
                                        auto* anno = createAnnotationNode("ReclaimAnnotation", "Tracing");
                                        if (anno) {
                                            anno->setSpan(target->spanStartLine, target->spanStartCol,
                                                          target->spanEndLine, target->spanEndCol);
                                            state.mutator.setRoot(ast);
                                            auto res = state.mutator.insertNode(target->id, "annotations", anno);
                                            if (!res.error.empty()) state.notify(NotificationLevel::Error, res.error);
                                            if (res.error.empty()) mutated = true;
                                        }
                                    }
                                    if (ImGui::MenuItem("@Owner(Single)")) {
                                        auto* anno = createAnnotationNode("OwnerAnnotation", "Single");
                                        if (anno) {
                                            anno->setSpan(target->spanStartLine, target->spanStartCol,
                                                          target->spanEndLine, target->spanEndCol);
                                            state.mutator.setRoot(ast);
                                            auto res = state.mutator.insertNode(target->id, "annotations", anno);
                                            if (!res.error.empty()) state.notify(NotificationLevel::Error, res.error);
                                            if (res.error.empty()) mutated = true;
                                        }
                                    }
                                    if (ImGui::MenuItem("@Deallocate(Explicit)")) {
                                        auto* anno = createAnnotationNode("DeallocateAnnotation", "Explicit");
                                        if (anno) {
                                            anno->setSpan(target->spanStartLine, target->spanStartCol,
                                                          target->spanEndLine, target->spanEndCol);
                                            state.mutator.setRoot(ast);
                                            auto res = state.mutator.insertNode(target->id, "annotations", anno);
                                            if (!res.error.empty()) state.notify(NotificationLevel::Error, res.error);
                                            if (res.error.empty()) mutated = true;
                                        }
                                    }
                                    if (ImGui::MenuItem("@Lifetime(RAII)")) {
                                        auto* anno = createAnnotationNode("LifetimeAnnotation", "RAII");
                                        if (anno) {
                                            anno->setSpan(target->spanStartLine, target->spanStartCol,
                                                          target->spanEndLine, target->spanEndCol);
                                            state.mutator.setRoot(ast);
                                            auto res = state.mutator.insertNode(target->id, "annotations", anno);
                                            if (!res.error.empty()) state.notify(NotificationLevel::Error, res.error);
                                            if (res.error.empty()) mutated = true;
                                        }
                                    }
                                    if (ImGui::MenuItem("@Allocate(Static)")) {
                                        auto* anno = createAnnotationNode("AllocateAnnotation", "Static");
                                        if (anno) {
                                            anno->setSpan(target->spanStartLine, target->spanStartCol,
                                                          target->spanEndLine, target->spanEndCol);
                                            state.mutator.setRoot(ast);
                                            auto res = state.mutator.insertNode(target->id, "annotations", anno);
                                            if (!res.error.empty()) state.notify(NotificationLevel::Error, res.error);
                                            if (res.error.empty()) mutated = true;
                                        }
                                    }
                                    ImGui::EndMenu();
                                }

                                const auto& annos = target->getChildren("annotations");
                                if (ImGui::BeginMenu("Edit Annotation", !annos.empty())) {
                                    for (auto* anno : annos) {
                                        std::string label = annotationLabel(anno);
                                        if (ImGui::BeginMenu(label.c_str())) {
                                            if (anno->conceptType == "ReclaimAnnotation") {
                                                if (ImGui::MenuItem("Tracing")) {
                                                    state.mutator.setRoot(ast);
                                                    auto res = state.mutator.setProperty(anno->id, "strategy", "Tracing");
                                                    if (!res.error.empty()) state.notify(NotificationLevel::Error, res.error);
                                                    if (res.error.empty()) mutated = true;
                                                }
                                            } else if (anno->conceptType == "OwnerAnnotation") {
                                                if (ImGui::MenuItem("Single")) {
                                                    state.mutator.setRoot(ast);
                                                    auto res = state.mutator.setProperty(anno->id, "strategy", "Single");
                                                    if (!res.error.empty()) state.notify(NotificationLevel::Error, res.error);
                                                    if (res.error.empty()) mutated = true;
                                                }
                                            } else if (anno->conceptType == "DeallocateAnnotation") {
                                                if (ImGui::MenuItem("Explicit")) {
                                                    state.mutator.setRoot(ast);
                                                    auto res = state.mutator.setProperty(anno->id, "strategy", "Explicit");
                                                    if (!res.error.empty()) state.notify(NotificationLevel::Error, res.error);
                                                    if (res.error.empty()) mutated = true;
                                                }
                                            } else if (anno->conceptType == "LifetimeAnnotation") {
                                                if (ImGui::MenuItem("RAII")) {
                                                    state.mutator.setRoot(ast);
                                                    auto res = state.mutator.setProperty(anno->id, "strategy", "RAII");
                                                    if (!res.error.empty()) state.notify(NotificationLevel::Error, res.error);
                                                    if (res.error.empty()) mutated = true;
                                                }
                                            } else if (anno->conceptType == "AllocateAnnotation") {
                                                if (ImGui::MenuItem("Static")) {
                                                    state.mutator.setRoot(ast);
                                                    auto res = state.mutator.setProperty(anno->id, "strategy", "Static");
                                                    if (!res.error.empty()) state.notify(NotificationLevel::Error, res.error);
                                                    if (res.error.empty()) mutated = true;
                                                }
                                            }
                                            ImGui::EndMenu();
                                        }
                                    }
                                    ImGui::EndMenu();
                                }

                                if (ImGui::BeginMenu("Remove Annotation", !annos.empty())) {
                                    for (auto* anno : annos) {
                                        std::string label = annotationLabel(anno);
                                        if (ImGui::MenuItem(label.c_str())) {
                                            state.mutator.setRoot(ast);
                                            auto res = state.mutator.deleteNode(anno->id);
                                            if (!res.error.empty()) state.notify(NotificationLevel::Error, res.error);
                                            if (res.error.empty()) mutated = true;
                                        }
                                    }
                                    ImGui::EndMenu();
                                }

                                std::vector<AnnotationConflict> conflicts;
                                collectAnnotationConflicts(ast, conflicts);
                                bool conflictOnLine = false;
                                AnnotationConflict lineConflict;
                                for (const auto& c : conflicts) {
                                    if (c.childLine == lineZero || c.parentLine == lineZero) {
                                        lineConflict = c;
                                        conflictOnLine = true;
                                        break;
                                    }
                                }
                                if (ImGui::BeginMenu("Resolve Conflict", conflictOnLine)) {
                                    if (conflictOnLine) {
                                        if (ImGui::MenuItem("Remove child annotation")) {
                                            state.mutator.setRoot(ast);
                                            auto res = state.mutator.deleteNode(lineConflict.childAnnoId);
                                            if (!res.error.empty()) state.notify(NotificationLevel::Error, res.error);
                                            if (res.error.empty()) mutated = true;
                                        }
                                        if (ImGui::MenuItem("Match parent strategy")) {
                                            state.mutator.setRoot(ast);
                                            auto res = state.mutator.setProperty(lineConflict.childAnnoId,
                                                                                "strategy",
                                                                                lineConflict.parentStrategy);
                                            if (!res.error.empty()) state.notify(NotificationLevel::Error, res.error);
                                            if (res.error.empty()) mutated = true;
                                        }
                                    }
                                    ImGui::EndMenu();
                                }
                                if (mutated) {
                                    state.applyOrchestratorToActive();
                                }
                            }
                            ImGui::EndPopup();
                        }

                        ImGui::PopStyleVar();
                        ImGui::EndTabItem();
                    }
                    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                        if (path.rfind("(untitled", 0) == 0) {
                            renameOpen = true;
                            renameTarget = path;
                            std::snprintf(renameBuf, sizeof(renameBuf), "%s", path.c_str());
                            ImGui::OpenPopup("Rename Buffer");
                        }
                    }
                    if (!open) {
                        state.buffers.closeBuffer(path);
                        state.bufferStates.erase(path);
                        if (path.rfind("(untitled", 0) != 0) state.watcher.unwatch(path);
                        if (state.buffers.bufferCount() > 0) {
                            state.switchToBuffer(state.buffers.getOpenBuffers().front());
                        } else {
                            state.activeBuffer = nullptr;
                        }
                    }
                }
            }
            ImGui::EndTabBar();
        }

        if (renameOpen && ImGui::BeginPopupModal("Rename Buffer", nullptr,
                                                 ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::TextUnformatted("Rename untitled buffer:");
            ImGui::SetNextItemWidth(320.0f);
            ImGui::InputText("##renameBuf", renameBuf, sizeof(renameBuf));
            if (ImGui::Button("Rename")) {
                std::string newName = renameBuf;
                if (!newName.empty() && renameTarget.rfind("(untitled", 0) == 0) {
                    state.renameBufferPath(renameTarget, newName);
                }
                renameOpen = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel")) {
                renameOpen = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        ImGui::PopFont();
        ImGui::End();
}
