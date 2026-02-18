#pragma once
// Step 633: Sprint 37 integration + summary

#include "AgentChatContextInjector.h"
#include "AgentChatPanelModel.h"
#include "AgentChatSessionPersistence.h"
#include "AgentMutationApproval.h"
#include "AgentTaskSlots.h"
#include "AgentTaskStatusOverlay.h"

#include <string>

struct Sprint37IntegrationResult {
    bool chatPanelOpened = false;
    bool contextInjected = false;
    bool userMessageSent = false;
    bool taskQueued = false;
    bool overlayVisible = false;
    bool previewShown = false;
    bool mutationAccepted = false;
    bool codeUpdated = false;
    bool sessionSaved = false;
    bool sessionRestored = false;
    int runningTasks = 0;
    int pendingTasks = 0;
    std::size_t restoredMessageCount = 0;
    std::string activeCode;
};

class Sprint37IntegrationSummary {
public:
    static Sprint37IntegrationResult run(const std::string& workspaceRoot,
                                         const std::string& projectFile,
                                         const std::string& taskDescription,
                                         const std::string& beforeCode,
                                         const std::string& generatedRust,
                                         const std::string& generatedCpp) {
        Sprint37IntegrationResult out;
        AgentChatState chat;
        AgentTaskSlotsState slots;

        out.chatPanelOpened = true;

        AgentChatRuntimeSnapshot snapshot;
        snapshot.activeFile = projectFile;
        snapshot.language = "rust";
        snapshot.astNodeCount = 42;
        snapshot.openAnnotationCount = 3;
        snapshot.workspacePath = workspaceRoot.empty() ? "." : workspaceRoot;
        AgentChatContextInjector::inject(&chat, snapshot, "t0");
        out.contextInjected = !chat.systemContext.empty() && !chat.messages.empty();

        chat.draftInput = taskDescription;
        out.userMessageSent = AgentChatPanelModel::sendUserMessage(&chat, "t1");

        std::string taskId = AgentTaskSlots::enqueueTask(&slots, taskDescription);
        out.taskQueued = !taskId.empty();
        (void)AgentTaskSlots::assignPendingToSlotsAt(&slots, 100);
        (void)AgentTaskSlots::updateTaskProgress(&slots, taskId, 2, "generate drone/nexus modules");

        AgentTaskStatusSummary summary = AgentTaskStatusOverlay::buildSummary(slots);
        out.overlayVisible = summary.hasWork;
        out.runningTasks = summary.running;
        out.pendingTasks = summary.pending;

        std::string generatedCode = "// Rust\n" + generatedRust + "\n// C++\n" + generatedCpp;
        AgentChatPanelModel::addToolCall(&chat,
                                         "whetstone_generate_code",
                                         {{"task", taskDescription}},
                                         {{"rust", generatedRust}, {"cpp", generatedCpp}},
                                         "t2");
        AgentChatPanelModel::addMutationPreview(&chat,
                                                "preview-37",
                                                "{\"type\":\"replaceModule\"}",
                                                beforeCode,
                                                generatedCode,
                                                "t3");
        out.previewShown = !chat.mutationPreviews.empty();

        out.mutationAccepted = AgentMutationApproval::accept(&chat.mutationApprovals, "preview-37");
        out.activeCode = beforeCode;
        if (out.mutationAccepted) {
            out.activeCode = generatedCode;
            out.codeUpdated = true;
        }

        std::string saveError;
        out.sessionSaved = AgentChatSessionPersistence::saveSession(workspaceRoot,
                                                                    projectFile,
                                                                    "sprint37-demo",
                                                                    chat,
                                                                    &saveError);
        AgentChatSavedSession loaded;
        std::string loadError;
        out.sessionRestored = AgentChatSessionPersistence::loadLatestSession(workspaceRoot,
                                                                             projectFile,
                                                                             &loaded,
                                                                             &loadError);
        if (out.sessionRestored) {
            out.restoredMessageCount = loaded.chat.messages.size();
        }
        return out;
    }
};
