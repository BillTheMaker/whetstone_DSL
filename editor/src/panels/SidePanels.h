#pragma once
#include "../EditorState.h"
#include "../EditorUtils.h"

static void renderOutlinePanel(EditorState& state) {
    if (!state.ui.showOutline) return;
    ImGui::Begin("Outline", &state.ui.showOutline);
    ImGui::PushFont(state.uiFont);
    ImGui::SetNextItemWidth(-1.0f);
    ImGui::InputText("Filter", state.outlineFilter, sizeof(state.outlineFilter));
    ImGui::Separator();

    std::string filter = toLowerCopy(state.outlineFilter);
    if (!state.active()) {
        ImGui::TextDisabled("(no file)");
    } else {
        std::vector<LSPClient::DocumentSymbol> lspSymbols =
            state.lsp ? state.lsp->getDocumentSymbols() : std::vector<LSPClient::DocumentSymbol>{};

        if (!lspSymbols.empty()) {
            int outlineId = 0;
            std::function<void(const LSPClient::DocumentSymbol&)> renderLsp;
            renderLsp = [&](const LSPClient::DocumentSymbol& sym) {
                if (!lspSymbolMatchesFilter(sym, filter)) return;
                ImGui::PushID(outlineId++);
                std::string label = std::string(lspSymbolKindLabel(sym.kind)) + ": " + sym.name;
                ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanFullWidth;
                if (sym.children.empty()) flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
                bool open = ImGui::TreeNodeEx(label.c_str(), flags);
                if (ImGui::IsItemClicked()) {
                    state.jumpTo(state.active(), sym.selectionRange.start.line,
                                 sym.selectionRange.start.character);
                }
                if (!sym.children.empty() && open) {
                    for (const auto& child : sym.children) {
                        renderLsp(child);
                    }
                    ImGui::TreePop();
                }
                ImGui::PopID();
            };
            for (const auto& sym : lspSymbols) renderLsp(sym);
        } else if (state.isStructured()) {
            std::vector<OutlineItem> outline;
            collectOutlineFromAST(state.activeAST(), outline);
            if (outline.empty()) {
                ImGui::TextDisabled("(no symbols)");
            } else {
                int outlineId = 0;
                std::function<void(const OutlineItem&)> renderItem;
                renderItem = [&](const OutlineItem& item) {
                    if (!outlineMatchesFilter(item, filter)) return;
                    ImGui::PushID(outlineId++);
                    std::string label = item.kind + ": " + item.name;
                    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanFullWidth;
                    if (item.children.empty()) flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
                    bool open = ImGui::TreeNodeEx(label.c_str(), flags);
                    if (ImGui::IsItemClicked() && item.line >= 0) {
                        state.jumpTo(state.active(), item.line, item.col);
                    }
                    if (!item.children.empty() && open) {
                        for (const auto& child : item.children) {
                            renderItem(child);
                        }
                        ImGui::TreePop();
                    }
                    ImGui::PopID();
                };
                for (const auto& item : outline) renderItem(item);
            }
        } else {
            ImGui::TextDisabled("(no symbols)");
        }
    }
    ImGui::PopFont();
    ImGui::End();
}

static void renderDependenciesPanel(EditorState& state) {
    if (!state.library.showDependencyPanel) return;
    ImGui::Begin("Dependencies", &state.library.showDependencyPanel);
    ImGui::PushFont(state.uiFont);
    renderDependencyPanel(state.library.dependencyPanel, state.workspaceRoot, state.notifications);
    ImGui::PopFont();
    ImGui::End();
    if (state.library.dependencyPanel.needsIndex) {
        state.requestLibraryIndex();
        state.library.dependencyPanel.needsIndex = false;
    }
}

static void renderLibrariesPanel(EditorState& state) {
    if (!state.library.showLibraryBrowserPanel) return;
    ImGui::Begin("Libraries", &state.library.showLibraryBrowserPanel);
    ImGui::PushFont(state.uiFont);
    std::string insertText;
    std::string insertLibrary;
    if (renderLibraryBrowser(state.library.libraryBrowser,
                             state.activeAST(),
                             state.library.libraryIndex,
                             insertText,
                             insertLibrary,
                             state.notifications)) {
        state.ensureImportForSymbol(insertLibrary, insertText);
        state.insertTextAtCursor(insertText);
    }
    ImGui::PopFont();
    ImGui::End();
}

static void renderCompositionPanel(EditorState& state) {
    if (!state.library.showCompositionPanel) return;
    ImGui::Begin("Compose", &state.library.showCompositionPanel);
    ImGui::PushFont(state.uiFont);
    std::string nodeId;
    if (state.activeAST()) {
        ASTNode* scopeNode = findNodeAtPosition(state.activeAST(),
                                                std::max(0, state.active()->cursorLine - 1),
                                                std::max(0, state.active()->cursorCol - 1));
        if (scopeNode) nodeId = scopeNode->id;
    }
    std::vector<PrimitiveSymbol> primitives;
    auto funcs = state.library.primitives.getAvailableFunctions(nodeId);
    primitives.insert(primitives.end(), funcs.begin(), funcs.end());
    std::string code;
    if (renderCompositionPanel(state.library.compositionPanel, primitives, code, state.notifications)) {
        state.insertTextAtCursor(code);
    }
    ImGui::PopFont();
    ImGui::End();
}

static void renderEmacsPackagesPanel(EditorState& state) {
    if (!state.emacsState.showEmacsPackagesPanel) return;
    ImGui::Begin("Emacs Packages", &state.emacsState.showEmacsPackagesPanel);
    ImGui::PushFont(state.uiFont);
    if (renderEmacsPackageBrowser(state.emacsState.emacsPackages,
                                  state.emacsState.emacs,
                                  state.notifications)) {
        state.emacsState.emacsFunctionIndexDirty = true;
    }
    ImGui::PopFont();
    ImGui::End();
}

static void renderEmacsBridgePanel(EditorState& state) {
    if (!state.emacsState.showEmacsBridgePanel) return;
    ImGui::Begin("Emacs Bridge", &state.emacsState.showEmacsBridgePanel);
    ImGui::PushFont(state.uiFont);
    if (state.active()) {
        ImGui::Text("Active file: %s", state.active()->path.c_str());
    } else {
        ImGui::TextDisabled("(no active buffer)");
    }
    ImGui::Separator();
    if (ImGui::Button("Open Emacs Frame")) {
        state.openInEmacsFrame();
    }
    ImGui::SameLine();
    if (ImGui::Button("Pull From Emacs")) {
        state.pullFromEmacs();
    }
    ImGui::SameLine();
    if (ImGui::Button("Push To Emacs")) {
        state.pushToEmacs();
    }
    ImGui::PopFont();
    ImGui::End();
}

static void renderMinibuffer(EditorState& state) {
    if (!(state.ui.layoutPreset == LayoutPreset::Emacs && state.emacsState.emacsKeys.minibufferActive))
        return;
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGuiWindowFlags mbFlags = ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoNavFocus;
    float mbHeight = ImGui::GetFrameHeight() + 10;
    ImVec2 mbPos(viewport->WorkPos.x,
                 viewport->WorkPos.y + viewport->WorkSize.y - mbHeight - 24.0f);
    ImVec2 mbSize(viewport->WorkSize.x, mbHeight);
    ImGui::SetNextWindowPos(mbPos);
    ImGui::SetNextWindowSize(mbSize);
    ImGui::Begin("##Minibuffer", nullptr, mbFlags);
    ImGui::PushFont(state.uiFont);
    ImGui::TextUnformatted(state.emacsState.emacsKeys.minibufferPrompt.c_str());
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-1.0f);
    if (ImGui::InputText("##minibuffer",
                         state.emacsState.emacsKeys.minibufferBuf,
                         sizeof(state.emacsState.emacsKeys.minibufferBuf),
                         ImGuiInputTextFlags_EnterReturnsTrue)) {
        emacsExecuteMinibuffer(state.emacsState.emacsKeys,
                               state.emacsState.emacs,
                               state.notifications);
    }
    ImGui::PopFont();
    ImGui::End();
}

static void renderMemoryStrategiesPanel(EditorState& state) {
    ImGui::Begin("Memory Strategies");
    ImGui::PushFont(state.uiFont);
    Module* ast = state.active() ? state.active()->sync.getAST() : nullptr;
    if (state.active() && state.active()->bufferMode == BufferManager::BufferMode::Text) {
        ImGui::TextDisabled("(disabled in Text mode)");
    } else if (!ast) {
        ImGui::TextDisabled("(no AST)");
    } else {
        std::vector<AnnotationEntry> entries;
        collectAnnotationEntries(ast, entries);
        std::map<std::string, int> counts;
        for (const auto& e : entries) counts[e.label]++;

        ImGui::Text("Annotations");
        ImGui::Separator();
        for (const auto& [label, count] : counts) {
            ImGui::Text("%s: %d", label.c_str(), count);
        }
        ImGui::Spacing();

        ImGui::Text("Details");
        ImGui::Separator();
        for (const auto& e : entries) {
            std::string lineInfo = e.line >= 0 ? ("L" + std::to_string(e.line + 1)) : "-";
            std::string row = e.label + " — " + e.nodeName + " (" + lineInfo + ")";
            if (ImGui::Selectable(row.c_str())) {
                if (state.active()) state.jumpTo(state.active(), e.line, 0);
            }
        }

        ImGui::Spacing();
        ImGui::Text("Suggestions");
        ImGui::Separator();
        for (const auto& s : state.suggestions) {
            if (s.confidence < 0.5) continue;
            std::string row = s.annotationType + "(" + s.strategy + ") — " +
                              s.nodeId + " (" + std::to_string(s.confidence) + ")";
            ImGui::TextUnformatted(row.c_str());
        }

        if (ImGui::Button("Export JSON")) {
            auto j = buildAnnotationSummaryJson(entries);
            state.notify(NotificationLevel::Info,
                         "Annotation summary:\\n" + j.dump(2));
        }
    }
    ImGui::PopFont();
    ImGui::End();
}
