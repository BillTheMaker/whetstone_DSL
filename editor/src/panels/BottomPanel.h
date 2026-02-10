#pragma once
#include "../EditorState.h"
#include "../EditorUtils.h"
#include <map>
#include <algorithm>

struct SimpleDiagnostic {
    int severity = 0; // 1=error,2=warning,3=info,4=hint
    std::string source;
    std::string file;
    int line = -1;
    int col = -1;
    std::string message;
    bool fixable = false;
};

static std::vector<SimpleDiagnostic> collectDiagnostics(const EditorState& state) {
    std::vector<SimpleDiagnostic> out;
    if (state.lsp) {
        for (const auto& d : state.lsp->getDiagnostics()) {
            SimpleDiagnostic sd;
            sd.severity = d.severity;
            sd.source = d.source.empty() ? "LSP" : d.source;
            sd.file = EditorState::fromFileUri(d.uri);
            sd.line = d.range.start.line;
            sd.col = d.range.start.character;
            sd.message = d.message;
            out.push_back(std::move(sd));
        }
    }
    for (const auto& d : state.whetstoneDiagnostics) {
        SimpleDiagnostic sd;
        sd.severity = d.severity;
        sd.source = d.source.empty() ? "Whetstone" : d.source;
        sd.file = EditorState::fromFileUri(d.uri);
        sd.line = d.line;
        sd.col = d.character;
        sd.message = d.message;
        out.push_back(std::move(sd));
    }
    for (const auto& d : state.emacsDiagnostics) {
        SimpleDiagnostic sd;
        sd.severity = d.severity > 0 ? d.severity : 1;
        sd.source = "Emacs";
        sd.file = d.uri;
        sd.line = -1;
        sd.col = -1;
        sd.message = d.message;
        out.push_back(std::move(sd));
    }
    return out;
}

static void renderBottomPanel(EditorState& state) {
    if (state.ui.focusTarget == FocusRegion::Bottom) {
        ImGui::SetNextWindowFocus();
        state.ui.focusTarget = FocusRegion::None;
    }
    // ---------------------------------------------------------------
    //  Bottom panel — Output / AST / Highlighted Preview / Terminal
    // ---------------------------------------------------------------
    ImGui::Begin("Panel");
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) {
        state.ui.focusedRegion = FocusRegion::Bottom;
    }

    if (ImGui::BeginTabBar("PanelTabs")) {
        // Output log
        if (ImGui::BeginTabItem("Output")) {
            ImGui::PushFont(state.monoFont);
            ImGui::BeginChild("##outputScroll", ImVec2(0, 0), false);
            for (const auto& note : state.notifications.getHistory()) {
                const char* label = "Info";
                ImVec4 color = ImVec4(0.40f, 0.70f, 0.95f, 1.0f);
                switch (note.level) {
                case NotificationLevel::Success:
                    label = "Success";
                    color = ImVec4(0.25f, 0.78f, 0.35f, 1.0f);
                    break;
                case NotificationLevel::Warning:
                    label = "Warning";
                    color = ImVec4(0.95f, 0.72f, 0.28f, 1.0f);
                    break;
                case NotificationLevel::Error:
                    label = "Error";
                    color = ImVec4(0.90f, 0.35f, 0.35f, 1.0f);
                    break;
                case NotificationLevel::Info:
                default:
                    break;
                }
                ImGui::TextColored(color, "[%s]", label);
                ImGui::SameLine();
                ImGui::TextWrapped("%s", note.message.c_str());
            }
            if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 20)
                ImGui::SetScrollHereY(1.0f);
            ImGui::EndChild();
            ImGui::PopFont();
            ImGui::EndTabItem();
        }

        if (state.build.showTerminalPanel && ImGui::BeginTabItem("Terminal")) {
            state.build.terminal.render(state.workspaceRoot, state.monoFont);
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Agents")) {
            ImGui::PushFont(state.monoFont);
            queueFeatureHint(state.featureHints,
                             "hint.agents",
                             "Tip: Agents can query and modify your AST via the JSON-RPC API.");
            bool running = state.agent.server && state.agent.server->isRunning();
            ImGui::Text("Server: %s", running ? "Running" : "Stopped");
            ImGui::SameLine(0, 20);
            ImGui::Text("Port: %d", state.agent.port);
            ImGui::Separator();

            ImGui::TextUnformatted("Active Sessions");
            if (!state.agent.server || state.agent.server->getActiveSessionCount() == 0) {
                ImGui::TextDisabled("(none)");
            } else {
                for (const auto& s : state.agent.server->getActiveSessions()) {
                    std::string label = s.sessionId;
                    if (!s.agentName.empty()) label += " (" + s.agentName + ")";
                    ImGui::TextUnformatted(label.c_str());
                    ImGui::SameLine(260.0f);
                    AgentRole role = state.getAgentRole(s.sessionId);
                    int roleIndex = static_cast<int>(role);
                    const char* roleLabels[] = {"Linter", "Refactor", "Generator"};
                    std::string comboId = "Role##" + s.sessionId;
                    if (ImGui::Combo(comboId.c_str(), &roleIndex, roleLabels, 3)) {
                        state.setAgentRole(s.sessionId, static_cast<AgentRole>(roleIndex));
                    }
                    ImGui::SameLine(420.0f);
                    ImGui::Text("Msgs: %d", s.messageCount);
                    ImGui::SameLine(520.0f);
                    ImGui::Text("Last: %llu", (unsigned long long)s.lastMessageAtMs);
                    ImGui::SameLine(680.0f);
                    std::string btnId = "Disconnect##" + s.sessionId;
                    if (ImGui::Button(btnId.c_str())) {
                        if (state.agent.transport) {
                            state.agent.transport->simulateDisconnect(s.sessionId);
                        } else {
                            state.logAgentEvent("STUB: disconnect not supported for real transport");
                        }
                    }
                    ImGui::Separator();
                }
            }

            ImGui::Separator();
            ImGui::TextUnformatted("Activity Log");
            ImGui::BeginChild("##agentLog", ImVec2(0, 0), false);
            for (const auto& line : state.agent.log) {
                ImGui::TextUnformatted(line.c_str());
            }
            if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 20)
                ImGui::SetScrollHereY(1.0f);
            ImGui::EndChild();

            ImGui::PopFont();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Marketplace")) {
            ImGui::PushFont(state.monoFont);
            renderAgentMarketplace(state.agent.marketplace,
                                   state.agent.registry,
                                   state.notifications,
                                   state.workspaceRoot);
            ImGui::PopFont();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Build")) {
            ImGui::PushFont(state.uiFont);
            ImGui::Text("Detected: %s", BuildSystem::typeName(state.build.buildType));
            auto cmds = BuildSystem::commandsFor(state.build.buildType);
            if (cmds.empty()) {
                ImGui::TextDisabled("(no build system detected)");
            } else {
                for (const auto& cmd : cmds) {
                    if (ImGui::Button(cmd.label.c_str())) {
                        state.runBuildCommand(cmd);
                    }
                    ImGui::SameLine();
                    ImGui::TextUnformatted(cmd.command.c_str());
                }
            }

            ImGui::Separator();
            ImGui::TextUnformatted("Errors");
            if (state.build.buildErrors.empty()) {
                ImGui::TextDisabled("(none)");
            } else {
                for (const auto& err : state.build.buildErrors) {
                    std::string label = err.file + ":" + std::to_string(err.line);
                    if (err.col > 0) label += ":" + std::to_string(err.col);
                    label += " " + err.message;
                    if (ImGui::Selectable(label.c_str())) {
                        std::filesystem::path p(err.file);
                        std::string path = p.is_absolute()
                            ? p.string()
                            : (std::filesystem::path(state.workspaceRoot) / p).string();
                        if (state.buffers.hasBuffer(path)) state.switchToBuffer(path);
                        else state.doOpen(path);
                        if (state.active()) {
                            state.jumpTo(state.active(),
                                         std::max(0, err.line - 1),
                                         std::max(0, err.col - 1));
                        }
                    }
                }
            }

            ImGui::PopFont();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Help")) {
            ImGui::PushFont(state.uiFont);
            renderHelpPanel(state.helpPanel, state.workspaceRoot, state.monoFont);
            ImGui::PopFont();
            ImGui::EndTabItem();
        }

        // Problems (diagnostics)
        if (ImGui::BeginTabItem("Problems")) {
            ImGui::PushFont(state.monoFont);
            ImGui::BeginChild("##problemsScroll", ImVec2(0, 0), false);
            static int sortIndex = 0;
            const char* sortLabels[] = {"Severity", "File", "Source"};
            ImGui::SetNextItemWidth(140.0f);
            ImGui::Combo("Sort By", &sortIndex, sortLabels, IM_ARRAYSIZE(sortLabels));
            ImGui::SameLine();
            bool anyFixable = false;
            if (!anyFixable) ImGui::BeginDisabled();
            if (ImGui::Button("Fix All")) {
                state.notify(NotificationLevel::Info, "No quick fixes available yet.");
            }
            if (!anyFixable) ImGui::EndDisabled();
            ImGui::Separator();

            auto diags = collectDiagnostics(state);
            if (diags.empty()) {
                ImGui::TextDisabled("(no diagnostics)");
            } else {
                std::sort(diags.begin(), diags.end(),
                          [&](const SimpleDiagnostic& a, const SimpleDiagnostic& b) {
                              if (sortIndex == 0) return a.severity < b.severity;
                              if (sortIndex == 1) return a.file < b.file;
                              return a.source < b.source;
                          });
                std::map<std::string, std::vector<SimpleDiagnostic>> byFile;
                for (const auto& d : diags) {
                    byFile[d.file].push_back(d);
                }

                for (const auto& [file, list] : byFile) {
                    std::string header = file.empty() ? "(unknown)" : file;
                    header += " (" + std::to_string(list.size()) + ")";
                    if (ImGui::TreeNode(header.c_str())) {
                        if (ImGui::BeginTable("##diagTable", 5,
                                              ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                            ImGui::TableSetupColumn("Severity");
                            ImGui::TableSetupColumn("Source");
                            ImGui::TableSetupColumn("Message");
                            ImGui::TableSetupColumn("Location");
                            ImGui::TableSetupColumn("Action");
                            ImGui::TableHeadersRow();

                            for (const auto& d : list) {
                                const char* sev = "Info";
                                if (d.severity == 1) sev = "Error";
                                else if (d.severity == 2) sev = "Warning";
                                else if (d.severity == 3) sev = "Info";
                                else if (d.severity == 4) sev = "Hint";

                                ImGui::TableNextRow();
                                ImGui::TableSetColumnIndex(0);
                                ImGui::TextUnformatted(sev);
                                ImGui::TableSetColumnIndex(1);
                                ImGui::TextUnformatted(d.source.c_str());
                                ImGui::TableSetColumnIndex(2);
                                if (ImGui::Selectable(d.message.c_str(), false, ImGuiSelectableFlags_SpanAllColumns)) {
                                    if (!d.file.empty() && d.line >= 0) {
                                        if (state.buffers.hasBuffer(d.file)) state.switchToBuffer(d.file);
                                        else state.doOpen(d.file);
                                        state.jumpTo(state.active(), d.line, d.col);
                                    }
                                }
                                ImGui::TableSetColumnIndex(3);
                                if (d.line >= 0) {
                                    ImGui::Text("%d:%d", d.line + 1, d.col + 1);
                                } else {
                                    ImGui::TextUnformatted("-");
                                }
                                ImGui::TableSetColumnIndex(4);
                                if (d.fixable) {
                                    if (ImGui::SmallButton("Apply")) {
                                        state.notify(NotificationLevel::Info, "Quick fix not wired yet.");
                                    }
                                } else {
                                    ImGui::BeginDisabled();
                                    ImGui::SmallButton("Apply");
                                    ImGui::EndDisabled();
                                }
                            }
                            ImGui::EndTable();
                        }
                        ImGui::TreePop();
                    }
                }
            }
            ImGui::EndChild();
            ImGui::PopFont();
            ImGui::EndTabItem();
        }

        // Optimization controls
        if (ImGui::BeginTabItem("Optimize")) {
            ImGui::PushFont(state.uiFont);
            Module* ast = state.mutationAST();
            if (!state.active()) {
                ImGui::TextDisabled("(no active buffer)");
            } else if (state.active()->bufferMode == BufferManager::BufferMode::Text) {
                ImGui::TextDisabled("(disabled in Text mode)");
            } else if (!ast) {
                ImGui::TextDisabled("(no AST)");
            } else if (state.active()->readOnly) {
                ImGui::TextDisabled("(read-only buffer)");
            } else {
                ImGui::Checkbox("Preview changes", &state.optimizePreview);
                std::string blockReason = findOptimizationBlockReason(ast);
                bool blocked = !blockReason.empty();

                auto showBlockedTooltip = [&](const std::string& reason) {
                    if (!reason.empty() &&
                        ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
                        ImGui::BeginTooltip();
                        ImGui::TextUnformatted(reason.c_str());
                        ImGui::EndTooltip();
                    }
                };

                if (blocked) ImGui::BeginDisabled();
                if (ImGui::Button("Constant Fold")) {
                    std::string beforeText = state.active()->editBuf;
                    if (state.optimizePreview) {
                        auto previewAst = cloneModule(ast);
                        TransformEngine engine;
                        engine.setRoot(previewAst.get());
                        engine.constantFolding();
                        std::string afterText = generateForLanguage(previewAst.get(),
                                                                    state.active()->language);
                        state.openDiff(beforeText, afterText, true, 1, {});
                    } else {
                        auto& inc = state.active()->incrementalOptimizer;
                        inc.setRoot(ast);
                        std::string tid = inc.applyTransform("constant-fold");
                        auto history = inc.getTransformHistory();
                        size_t nodes = 0;
                        for (const auto& h : history) {
                            if (h.transformId == tid) {
                                nodes = h.affectedNodeIds.size();
                                break;
                            }
                        }
                        std::string warning = findOptimizationLockWarning(ast);
                        std::string summary = "Constant Fold: ";
                        if (nodes > 0) {
                            summary += "applied (" + std::to_string(nodes) + " nodes)";
                        } else {
                            summary += "no changes";
                        }
                        if (!warning.empty()) {
                            summary += " — warning: " + warning;
                            state.notify(NotificationLevel::Warning, warning);
                        }
                        state.optimizeFoldSummary = summary;
                        state.notify(NotificationLevel::Info, summary);
                        if (nodes > 0) state.applyOrchestratorToActive();
                        std::string afterText = state.active()->editBuf;
                        state.openDiff(beforeText, afterText, false, 1, {tid});
                    }
                }
                showBlockedTooltip(blockReason);
                if (blocked) ImGui::EndDisabled();
                if (!state.optimizeFoldSummary.empty()) {
                    ImGui::TextWrapped("%s", state.optimizeFoldSummary.c_str());
                }

                ImGui::Separator();

                if (blocked) ImGui::BeginDisabled();
                if (ImGui::Button("Dead Code Elimination")) {
                    std::string beforeText = state.active()->editBuf;
                    if (state.optimizePreview) {
                        auto previewAst = cloneModule(ast);
                        TransformEngine engine;
                        engine.setRoot(previewAst.get());
                        engine.deadCodeElimination();
                        std::string afterText = generateForLanguage(previewAst.get(),
                                                                    state.active()->language);
                        state.openDiff(beforeText, afterText, true, 2, {});
                    } else {
                        auto& inc = state.active()->incrementalOptimizer;
                        inc.setRoot(ast);
                        std::string tid = inc.applyTransform("dead-code-elim");
                        auto history = inc.getTransformHistory();
                        size_t nodes = 0;
                        for (const auto& h : history) {
                            if (h.transformId == tid) {
                                nodes = h.affectedNodeIds.size();
                                break;
                            }
                        }
                        std::string summary = "Dead Code Elimination: ";
                        if (nodes > 0) {
                            summary += "applied (" + std::to_string(nodes) + " nodes)";
                        } else {
                            summary += "no changes";
                        }
                        state.optimizeDeadCodeSummary = summary;
                        state.notify(NotificationLevel::Info, summary);
                        if (nodes > 0) state.applyOrchestratorToActive();
                        std::string afterText = state.active()->editBuf;
                        state.openDiff(beforeText, afterText, false, 2, {tid});
                    }
                }
                showBlockedTooltip(blockReason);
                if (blocked) ImGui::EndDisabled();
                if (!state.optimizeDeadCodeSummary.empty()) {
                    ImGui::TextWrapped("%s", state.optimizeDeadCodeSummary.c_str());
                }

                ImGui::Separator();

                if (blocked) ImGui::BeginDisabled();
                if (ImGui::Button("Apply All")) {
                    std::string beforeText = state.active()->editBuf;
                    if (state.optimizePreview) {
                        auto previewAst = cloneModule(ast);
                        TransformEngine engine;
                        engine.setRoot(previewAst.get());
                        engine.applyAll();
                        std::string afterText = generateForLanguage(previewAst.get(),
                                                                    state.active()->language);
                        state.openDiff(beforeText, afterText, true, 3, {});
                    } else {
                        auto& inc = state.active()->incrementalOptimizer;
                        inc.setRoot(ast);
                        std::string tidFold = inc.applyTransform("constant-fold");
                        std::string tidDce = inc.applyTransform("dead-code-elim");
                        auto history = inc.getTransformHistory();
                        size_t totalNodes = 0;
                        for (const auto& h : history) {
                            if (h.transformId == tidFold || h.transformId == tidDce) {
                                totalNodes += h.affectedNodeIds.size();
                            }
                        }
                        std::string warning = findOptimizationLockWarning(ast);
                        std::string summary = "Apply All: ";
                        if (totalNodes > 0) {
                            summary += "applied (" + std::to_string(totalNodes) + " nodes)";
                        } else {
                            summary += "no changes";
                        }
                        if (!warning.empty()) {
                            summary += " — warning: " + warning;
                            state.notify(NotificationLevel::Warning, warning);
                        }
                        state.optimizeApplySummary = summary;
                        state.notify(NotificationLevel::Info, summary);
                        if (totalNodes > 0) state.applyOrchestratorToActive();
                        std::string afterText = state.active()->editBuf;
                        state.openDiff(beforeText, afterText, false, 3, {tidFold, tidDce});
                    }
                }
                showBlockedTooltip(blockReason);
                if (blocked) ImGui::EndDisabled();
                if (!state.optimizeApplySummary.empty()) {
                    ImGui::TextWrapped("%s", state.optimizeApplySummary.c_str());
                }
            }
            ImGui::PopFont();
            ImGui::EndTabItem();
        }

        // Transform history
        if (ImGui::BeginTabItem("Transforms")) {
            ImGui::PushFont(state.uiFont);
            auto* buf = state.active();
            Module* ast = state.mutationAST();
            if (!buf) {
                ImGui::TextDisabled("(no active buffer)");
            } else if (buf->bufferMode == BufferManager::BufferMode::Text) {
                ImGui::TextDisabled("(disabled in Text mode)");
            } else if (!ast) {
                ImGui::TextDisabled("(no AST)");
            } else {
                auto& inc = buf->incrementalOptimizer;
                inc.setRoot(ast);
                auto history = inc.getTransformHistory();

                const bool hasHistory = !history.empty();
                if (!hasHistory) ImGui::BeginDisabled();
                if (ImGui::Button("Undo All")) {
                    bool any = false;
                    while (inc.undoLast()) {
                        any = true;
                    }
                    if (any) state.applyOrchestratorToActive();
                }
                if (!hasHistory) ImGui::EndDisabled();
                ImGui::Separator();

                if (!hasHistory) {
                    ImGui::TextDisabled("(no transforms)");
                } else {
                    for (const auto& h : history) {
                        ImGui::PushID(h.transformId.c_str());
                        ImVec4 color = transformColorForName(h.transformName);
                        ImGui::TextColored(color, "%s", h.transformName.c_str());
                        ImGui::SameLine();
                        ImGui::TextDisabled("@ %s", h.timestamp.c_str());
                        if (!h.actor.empty()) {
                            ImGui::SameLine();
                            ImGui::TextDisabled("by %s", h.actor.c_str());
                        }
                        ImGui::SameLine();
                        if (!h.reversible) ImGui::BeginDisabled();
                        if (ImGui::Button("Undo")) {
                            if (inc.undoTransform(h.transformId)) {
                                state.applyOrchestratorToActive();
                            }
                        }
                        if (!h.reversible) ImGui::EndDisabled();
                        ImGui::Text("Affected: %d", (int)h.affectedNodeIds.size());
                        std::string nodeList;
                        for (size_t i = 0; i < h.affectedNodeIds.size() && i < 5; ++i) {
                            if (!nodeList.empty()) nodeList += ", ";
                            nodeList += h.affectedNodeIds[i];
                        }
                        if (h.affectedNodeIds.size() > 5) nodeList += ", ...";
                        if (nodeList.empty()) nodeList = "(none)";
                        ImGui::TextWrapped("Nodes: %s", nodeList.c_str());
                        ImGui::Separator();
                        ImGui::PopID();
                    }
                }
            }
            ImGui::PopFont();
            ImGui::EndTabItem();
        }

        // Diff view
        if (ImGui::BeginTabItem("Diff")) {
            ImGui::PushFont(state.uiFont);
            auto* buf = state.active();
            Module* ast = state.mutationAST();
            if (!buf) {
                ImGui::TextDisabled("(no active buffer)");
            } else if (buf->bufferMode == BufferManager::BufferMode::Text) {
                ImGui::TextDisabled("(diff view disabled in Text mode)");
            } else if (!state.diff.active) {
                ImGui::TextDisabled("(no diff)");
            } else {
                if (state.diff.preview) {
                    if (ImGui::Button("Apply")) {
                        if (ast && !buf->readOnly) {
                            if (state.diff.batch) {
                                BatchMutationAPI batch;
                                batch.setRoot(ast);
                                auto res = batch.applySequence(state.diff.batchMutations);
                                if (!res.success) {
                                    state.notify(NotificationLevel::Error, res.error);
                                } else {
                                    state.notify(NotificationLevel::Success,
                                                 "Refactor applied (" +
                                                 std::to_string(res.appliedCount) + " mutations).");
                                    state.applyOrchestratorToActive();
                                }
                            } else {
                                buf->incrementalOptimizer.setRoot(ast);
                                if (state.diff.action == 1) {
                                    buf->incrementalOptimizer.applyTransform("constant-fold");
                                } else if (state.diff.action == 2) {
                                    buf->incrementalOptimizer.applyTransform("dead-code-elim");
                                } else if (state.diff.action == 3) {
                                    buf->incrementalOptimizer.applyTransform("constant-fold");
                                    buf->incrementalOptimizer.applyTransform("dead-code-elim");
                                }
                                state.applyOrchestratorToActive();
                            }
                        }
                        state.diff.active = false;
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Cancel")) {
                        state.diff.active = false;
                    }
                } else {
                    if (ImGui::Button("Keep")) {
                        state.diff.active = false;
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Undo")) {
                        if (ast && !buf->readOnly) {
                            buf->incrementalOptimizer.setRoot(ast);
                            for (auto it = state.diff.transformIds.rbegin();
                                 it != state.diff.transformIds.rend(); ++it) {
                                buf->incrementalOptimizer.undoTransform(*it);
                            }
                            state.applyOrchestratorToActive();
                        }
                        state.diff.active = false;
                    }
                }

                ImGui::Separator();
                ImGui::BeginTable("##diffSplit", 2,
                                  ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp);
                ImGui::TableSetupColumn("Before", ImGuiTableColumnFlags_WidthStretch, 0.5f);
                ImGui::TableSetupColumn("After", ImGuiTableColumnFlags_WidthStretch, 0.5f);
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                CodeEditorOptions leftOpts;
                leftOpts.readOnly = true;
                leftOpts.showWhitespace = state.ui.showWhitespace;
                leftOpts.showLineNumbers = state.ui.showLineNumbers;
                leftOpts.showCurrentLine = false;
                leftOpts.useAnnotationShapes = state.settings.getUseAnnotationShapes();
                leftOpts.lineHeightScale = state.settings.getLineHeightScale();
                leftOpts.letterSpacing = state.settings.getLetterSpacing();
                leftOpts.highlightLines = &state.diff.beforeLines;
                leftOpts.highlightLineColor = IM_COL32(160, 80, 80, 120);
                leftOpts.syncScrollX = &state.diffScrollX;
                leftOpts.syncScrollY = &state.diffScrollY;
                leftOpts.scrollMaster = true;
                state.diffLeftWidget.render("##diffBefore",
                    state.diff.beforeText, std::vector<HighlightSpan>{}, leftOpts,
                    ImVec2(0, 0), state.monoFont);

                ImGui::TableSetColumnIndex(1);
                CodeEditorOptions rightOpts;
                rightOpts.readOnly = true;
                rightOpts.showWhitespace = state.ui.showWhitespace;
                rightOpts.showLineNumbers = state.ui.showLineNumbers;
                rightOpts.showCurrentLine = false;
                rightOpts.useAnnotationShapes = state.settings.getUseAnnotationShapes();
                rightOpts.lineHeightScale = state.settings.getLineHeightScale();
                rightOpts.letterSpacing = state.settings.getLetterSpacing();
                rightOpts.highlightLines = &state.diff.afterLines;
                rightOpts.highlightLineColor = IM_COL32(80, 160, 80, 120);
                rightOpts.syncScrollX = &state.diffScrollX;
                rightOpts.syncScrollY = &state.diffScrollY;
                rightOpts.scrollMaster = false;
                state.diffRightWidget.render("##diffAfter",
                    state.diff.afterText, std::vector<HighlightSpan>{}, rightOpts,
                    ImVec2(0, 0), state.monoFont);
                ImGui::EndTable();
            }
            ImGui::PopFont();
            ImGui::EndTabItem();
        }

        // AST view
        if (ImGui::BeginTabItem("AST")) {
            ImGui::PushFont(state.monoFont);
            ImGui::BeginChild("##astScroll", ImVec2(0, 0), false);
            Module* ast = state.active() ? state.active()->sync.getAST() : nullptr;
            if (state.active() && state.active()->bufferMode == BufferManager::BufferMode::Text) {
                ImGui::TextDisabled("(disabled in Text mode)");
            } else if (ast) {
                std::map<std::string, std::string> transformNames;
                std::map<std::string, std::string> transformActors;
                if (state.active()) {
                    auto history = state.active()->incrementalOptimizer.getTransformHistory();
                    for (const auto& h : history) {
                        transformNames[h.transformId] = h.transformName;
                        transformActors[h.transformId] = h.actor;
                    }
                }
                // Show basic AST info
                ImGui::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f),
                                   "Module: %s  [%s]",
                                   ast->name.c_str(), ast->targetLanguage.c_str());
                ImGui::Separator();
                auto functions = ast->getChildren("functions");
                for (size_t i = 0; i < functions.size(); ++i) {
                    auto* fn = static_cast<Function*>(functions[i]);
                    std::string tid;
                    std::string tname;
                    if (state.active()) {
                        tid = state.active()->incrementalOptimizer.getProvenance(fn->id);
                        auto it = transformNames.find(tid);
                        if (it != transformNames.end()) tname = it->second;
                    }
                    ImVec4 fnColor = tname.empty() ?
                        ImVec4(0.86f, 0.86f, 0.55f, 1.0f) :
                        transformColorForName(tname);
                    ImGui::TextColored(fnColor, "  Function: %s", fn->name.c_str());
                    if (!tname.empty() && ImGui::IsItemHovered()) {
                        ImGui::BeginTooltip();
                        std::string actor = transformActors[tid];
                        if (actor.empty()) {
                            ImGui::Text("Transform: %s (%s)", tname.c_str(), tid.c_str());
                        } else {
                            ImGui::Text("Transform: %s (%s) by %s",
                                        tname.c_str(), tid.c_str(), actor.c_str());
                        }
                        ImGui::EndTooltip();
                    }
                    auto params = fn->getChildren("parameters");
                    for (auto* p : params) {
                        auto* param = static_cast<Parameter*>(p);
                        std::string pid;
                        std::string pname;
                        if (state.active()) {
                            pid = state.active()->incrementalOptimizer.getProvenance(param->id);
                            auto it = transformNames.find(pid);
                            if (it != transformNames.end()) pname = it->second;
                        }
                        ImVec4 pColor = pname.empty() ?
                            ImVec4(0.6f, 0.78f, 0.9f, 1.0f) :
                            transformColorForName(pname);
                        ImGui::TextColored(pColor, "    param: %s", param->name.c_str());
                        if (!pname.empty() && ImGui::IsItemHovered()) {
                            ImGui::BeginTooltip();
                            std::string actor = transformActors[pid];
                            if (actor.empty()) {
                                ImGui::Text("Transform: %s (%s)", pname.c_str(), pid.c_str());
                            } else {
                                ImGui::Text("Transform: %s (%s) by %s",
                                            pname.c_str(), pid.c_str(), actor.c_str());
                            }
                            ImGui::EndTooltip();
                        }
                    }
                    auto body = fn->getChildren("body");
                    ImGui::Text("    body: %d statement(s)", (int)body.size());
                    auto annos = fn->getChildren("annotations");
                    for (auto* a : annos) {
                        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f),
                                           "    @%s", a->conceptType.c_str());
                    }
                }
            } else {
                ImGui::TextDisabled("(no AST — enter some code)");
            }
            ImGui::EndChild();
            ImGui::PopFont();
            ImGui::EndTabItem();
        }

        // Syntax-highlighted preview
        if (ImGui::BeginTabItem("Highlighted")) {
            ImGui::PushFont(state.monoFont);
            ImGui::BeginChild("##hlScroll", ImVec2(0, 0), false);
            state.updateHighlights();
            if (state.active())
                RenderHighlightedText(state.active()->editBuf, state.active()->highlights);
            ImGui::EndChild();
            ImGui::PopFont();
            ImGui::EndTabItem();
        }

        // Generated code preview
        if (ImGui::BeginTabItem("Generated")) {
            ImGui::PushFont(state.monoFont);
            ImGui::BeginChild("##genScroll", ImVec2(0, 0), false);
            Module* ast = state.active() ? state.active()->sync.getAST() : nullptr;
            if (state.active() && state.active()->bufferMode == BufferManager::BufferMode::Text) {
                ImGui::TextDisabled("(disabled in Text mode)");
            } else {
                state.updateGenerated();
                if (ast && state.active()) {
                    ImGui::TextUnformatted(state.active()->generatedBuf.c_str());
                } else {
                    ImGui::TextDisabled("(no AST)");
                }
            }
            ImGui::EndChild();
            ImGui::PopFont();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}
