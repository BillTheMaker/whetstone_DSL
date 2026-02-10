#pragma once
#include "../EditorState.h"
#include "../EditorUtils.h"
#include "../MemoryStrategyInference.h"
#include "../ASTMutationAPI.h"
#include "../CrossLanguageProjector.h"
#include "../WizardFramework.h"
#include "../AgentPermissionPolicy.h"
#include "../ast/Function.h"
#include "../ast/Annotation.h"
#include <atomic>
#include <vector>

struct AnnotateFileWizardState {
    WizardFrame frame;
    bool prepared = false;
    int index = 0;
    int accepted = 0;
    int skipped = 0;
    std::string targetPath;
    std::vector<MemoryStrategyInference::Suggestion> suggestions;
};

struct ProjectWizardState {
    WizardFrame frame;
    int targetIndex = 0;
    std::string preview;
    int sourceAnnoCount = 0;
    int projectedAnnoCount = 0;
    bool annotationsPreserved = false;
    bool previewReady = false;
    std::string targetLanguage;
};

struct ConnectAgentWizardState {
    WizardFrame frame;
    int portInput = 8765;
    int roleIndex = 0;
    bool tested = false;
    bool testOk = false;
    std::string testMessage;
};

static std::string wizardId(const char* prefix) {
    static std::atomic<int> counter{0};
    return std::string(prefix) + "_wiz_" + std::to_string(counter.fetch_add(1));
}

static ASTNode* wizardFindNodeById(ASTNode* node, const std::string& nodeId) {
    if (!node) return nullptr;
    if (node->id == nodeId) return node;
    for (auto* child : node->allChildren()) {
        if (auto* found = wizardFindNodeById(child, nodeId)) return found;
    }
    return nullptr;
}

static bool wizardHasMemoryAnnotation(const ASTNode* node) {
    for (auto* anno : node->getChildren("annotations")) {
        const auto& ct = anno->conceptType;
        if (ct == "DeallocateAnnotation" || ct == "LifetimeAnnotation" ||
            ct == "ReclaimAnnotation" || ct == "OwnerAnnotation" ||
            ct == "AllocateAnnotation") {
            return true;
        }
    }
    return false;
}

static ASTNode* wizardCreateAnnotation(const MemoryStrategyInference::Suggestion& s) {
    if (s.annotationType == "DeallocateAnnotation") {
        return new DeallocateAnnotation(wizardId("anno"), s.strategy);
    }
    if (s.annotationType == "LifetimeAnnotation") {
        return new LifetimeAnnotation(wizardId("anno"), s.strategy);
    }
    if (s.annotationType == "ReclaimAnnotation") {
        return new ReclaimAnnotation(wizardId("anno"), s.strategy);
    }
    if (s.annotationType == "OwnerAnnotation") {
        return new OwnerAnnotation(wizardId("anno"), s.strategy);
    }
    if (s.annotationType == "AllocateAnnotation") {
        return new AllocateAnnotation(wizardId("anno"), s.strategy);
    }
    return nullptr;
}

static void prepareAnnotateWizard(EditorState& state, AnnotateFileWizardState& wizard) {
    wizard.suggestions.clear();
    wizard.index = 0;
    wizard.accepted = 0;
    wizard.skipped = 0;
    wizard.prepared = true;
    wizard.targetPath = state.active() ? state.active()->path : "";

    if (!state.isStructured()) return;
    Module* ast = state.activeAST();
    if (!ast) return;

    MemoryStrategyInference infer;
    auto suggestions = infer.inferAnnotations(ast);
    for (const auto& s : suggestions) {
        ASTNode* node = wizardFindNodeById(ast, s.nodeId);
        if (!node || node->conceptType != "Function") continue;
        if (wizardHasMemoryAnnotation(node)) continue;
        wizard.suggestions.push_back(s);
    }
}

static void renderAnnotateFileWizard(EditorState& state, AnnotateFileWizardState& wizard) {
    if (!wizard.frame.open) return;
    wizard.frame.stepCount = 3;
    if (!beginWizardWindow("Annotate File Wizard", wizard.frame)) return;

    if (!wizard.prepared ||
        (state.active() && wizard.targetPath != state.active()->path)) {
        prepareAnnotateWizard(state, wizard);
    }

    if (wizard.frame.step == 0) {
        ImGui::TextUnformatted("Review suggested memory annotations.");
        ImGui::Separator();
        if (!state.isStructured() || !state.activeAST()) {
            ImGui::TextDisabled("Open a structured buffer to continue.");
        } else {
            ImGui::Text("Found %d function(s) with suggestions.",
                        (int)wizard.suggestions.size());
            ImGui::TextDisabled("You can accept or skip each suggestion.");
        }
    } else if (wizard.frame.step == 1) {
        if (!state.isStructured() || !state.activeAST()) {
            ImGui::TextDisabled("Open a structured buffer to continue.");
        } else if (wizard.suggestions.empty()) {
            ImGui::TextDisabled("No suggestions to review.");
        } else {
            if (wizard.index >= (int)wizard.suggestions.size()) {
                wizard.frame.step = 2;
            } else {
                const auto& s = wizard.suggestions[wizard.index];
                ASTNode* node = wizardFindNodeById(state.activeAST(), s.nodeId);
                std::string fnName = "(unknown)";
                if (node && node->conceptType == "Function") {
                    fnName = static_cast<Function*>(node)->name;
                }
                ImGui::Text("Function: %s", fnName.c_str());
                ImGui::Text("Suggestion: %s (%s)",
                            s.annotationType.c_str(),
                            s.strategy.c_str());
                ImGui::TextWrapped("Reason: %s", s.reason.c_str());
                ImGui::Text("Confidence: %.0f%%", s.confidence * 100.0);
                ImGui::Separator();

                if (ImGui::Button("Accept")) {
                    MemoryStrategyInference infer;
                    ASTNode* ast = state.mutationAST();
                    ASTMutationAPI mut;
                    if (ast) mut.setRoot(ast);
                    ASTNode* anno = wizardCreateAnnotation(s);
                    if (!ast || !anno) {
                        state.notify(NotificationLevel::Error,
                                     "Failed to apply annotation.");
                    } else {
                        auto res = mut.insertNode(s.nodeId, "annotations", anno);
                        if (!res.success) {
                            delete anno;
                            state.notify(NotificationLevel::Error,
                                         "Annotation insert failed: " + res.error);
                        } else {
                            if (!res.warning.empty()) {
                                state.notify(NotificationLevel::Warning, res.warning);
                            }
                            infer.recordFeedback(s, true);
                            wizard.accepted += 1;
                            state.applyOrchestratorToActive();
                        }
                    }
                    wizard.index += 1;
                }
                ImGui::SameLine();
                if (ImGui::Button("Skip")) {
                    MemoryStrategyInference infer;
                    infer.recordFeedback(s, false);
                    wizard.skipped += 1;
                    wizard.index += 1;
                }
                ImGui::SameLine();
                if (ImGui::Button("Skip All")) {
                    wizard.skipped += (int)wizard.suggestions.size() - wizard.index;
                    wizard.index = (int)wizard.suggestions.size();
                }
            }
        }
    } else if (wizard.frame.step == 2) {
        ImGui::TextUnformatted("Summary");
        ImGui::Separator();
        ImGui::Text("Accepted: %d", wizard.accepted);
        ImGui::Text("Skipped: %d", wizard.skipped);
    }

    WizardNavAction nav = renderWizardFooter(wizard.frame, "Done");
    if (nav == WizardNavAction::Finish || nav == WizardNavAction::Cancel) {
        wizard.prepared = false;
    }
    ImGui::End();
}

static void buildProjectionPreview(EditorState& state,
                                   ProjectWizardState& wizard,
                                   const std::string& targetLanguage) {
    wizard.preview.clear();
    wizard.previewReady = false;
    wizard.sourceAnnoCount = 0;
    wizard.projectedAnnoCount = 0;
    wizard.annotationsPreserved = false;
    wizard.targetLanguage = targetLanguage;

    if (!state.isStructured()) return;
    Module* ast = state.activeAST();
    if (!ast) return;

    CrossLanguageProjector projector;
    auto projected = projector.project(ast, targetLanguage);
    if (!projected) {
        wizard.preview = "Projection failed.";
        wizard.previewReady = true;
        return;
    }

    wizard.sourceAnnoCount = countAnnotationNodes(ast);
    wizard.projectedAnnoCount = countAnnotationNodes(projected.get());
    wizard.annotationsPreserved = projector.annotationsPreserved(ast, projected.get());

    std::string text = generateForLanguage(projected.get(), targetLanguage);
    int lines = 0;
    std::string snippet;
    snippet.reserve(text.size());
    for (char c : text) {
        snippet.push_back(c);
        if (c == '\n') {
            lines += 1;
            if (lines >= 18) break;
        }
    }
    wizard.preview = snippet;
    wizard.previewReady = true;
}

static void renderProjectWizard(EditorState& state, ProjectWizardState& wizard) {
    if (!wizard.frame.open) return;
    wizard.frame.stepCount = 3;
    if (!beginWizardWindow("Cross-Language Project Wizard", wizard.frame)) return;

    const char* languages[] = {"python", "cpp", "elisp", "javascript",
                               "typescript", "java", "rust", "go"};
    int languageCount = IM_ARRAYSIZE(languages);
    std::string activeLang = state.active() ? state.active()->language : "python";

    if (wizard.frame.step == 0) {
        ImGui::Text("Source language: %s", activeLang.c_str());
        ImGui::Separator();
        ImGui::TextUnformatted("Choose target language");
        for (int i = 0; i < languageCount; ++i) {
            if (ImGui::RadioButton(languages[i], wizard.targetIndex == i)) {
                wizard.targetIndex = i;
            }
        }
        if (!state.isStructured() || !state.activeAST()) {
            ImGui::TextDisabled("Structured AST required for projection.");
        }
    } else if (wizard.frame.step == 1) {
        if (!wizard.previewReady) {
            buildProjectionPreview(state, wizard, languages[wizard.targetIndex]);
        }
        ImGui::Text("Target: %s", languages[wizard.targetIndex]);
        ImGui::Text("Annotations: %d -> %d (%s)",
                    wizard.sourceAnnoCount,
                    wizard.projectedAnnoCount,
                    wizard.annotationsPreserved ? "preserved" : "lossy");
        ImGui::Separator();
        ImGui::TextUnformatted("Preview");
        ImGui::BeginChild("##projectPreview", ImVec2(0, 200), true);
        ImGui::TextUnformatted(wizard.preview.c_str());
        ImGui::EndChild();
    } else if (wizard.frame.step == 2) {
        ImGui::TextUnformatted("Confirm projection");
        ImGui::Separator();
        ImGui::TextWrapped("This will open a new tab with the projected code.");
        ImGui::Text("Target language: %s", languages[wizard.targetIndex]);
    }

    WizardNavAction nav = renderWizardFooter(wizard.frame, "Project");
    if (nav == WizardNavAction::Next && wizard.frame.step == 1) {
        wizard.previewReady = false;
    }
    if (nav == WizardNavAction::Finish) {
        if (state.isStructured() && state.activeAST()) {
            state.projectToLanguage(languages[wizard.targetIndex]);
        } else {
            state.notify(NotificationLevel::Warning,
                         "Projection requires a structured buffer.");
        }
    }
    if (nav == WizardNavAction::Cancel) {
        wizard.previewReady = false;
    }
    ImGui::End();
}

static void renderConnectAgentWizard(EditorState& state, ConnectAgentWizardState& wizard) {
    if (!wizard.frame.open) return;
    wizard.frame.stepCount = 3;
    if (!beginWizardWindow("Connect Agent Wizard", wizard.frame)) return;

    if (wizard.frame.step == 0) {
        if (wizard.portInput <= 0) wizard.portInput = state.agent.port;
        ImGui::TextUnformatted("Agent server endpoint");
        ImGui::Separator();
        ImGui::InputInt("Port", &wizard.portInput);
        wizard.portInput = std::max(1, std::min(wizard.portInput, 65535));
        ImGui::Text("ws://localhost:%d", wizard.portInput);
        ImGui::Separator();
        ImGui::TextUnformatted("Default permissions");
        const char* roleLabels[] = {"Linter", "Refactor", "Generator"};
        if (wizard.roleIndex < 0 || wizard.roleIndex > 2) {
            wizard.roleIndex = (int)state.agent.defaultRole;
        }
        if (ImGui::Combo("Role", &wizard.roleIndex, roleLabels, 3)) {
            state.agent.defaultRole = static_cast<AgentRole>(wizard.roleIndex);
        }
        if (ImGui::Button("Apply & Restart Server")) {
            state.shutdownAgentServer();
            state.agent.port = wizard.portInput;
            state.initAgentServer();
            wizard.tested = false;
        }
    } else if (wizard.frame.step == 1) {
        ImGui::TextUnformatted("Test connection");
        ImGui::Separator();
        if (ImGui::Button("Test Now")) {
            bool running = state.agent.server && state.agent.server->isRunning();
            wizard.tested = true;
            wizard.testOk = running;
            wizard.testMessage = running
                ? "Server is running and accepting connections."
                : "Server not running. Try restarting.";
        }
        if (wizard.tested) {
            ImGui::TextColored(wizard.testOk ? ImVec4(0.2f, 0.8f, 0.3f, 1.0f)
                                             : ImVec4(0.9f, 0.4f, 0.4f, 1.0f),
                               "%s", wizard.testMessage.c_str());
        }
    } else if (wizard.frame.step == 2) {
        ImGui::TextUnformatted("API capabilities");
        ImGui::Separator();
        ImGui::BulletText("getAST, findNodesByType, findNodesByAnnotation");
        ImGui::BulletText("applyMutation, applyAnnotationSuggestion");
        ImGui::BulletText("generateCode, getAnnotationSuggestions");
        ImGui::BulletText("setAgentRole, workflow recording APIs");
    }

    renderWizardFooter(wizard.frame, "Done");
    ImGui::End();
}
