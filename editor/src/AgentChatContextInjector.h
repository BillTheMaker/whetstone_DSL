#pragma once
// Step 629: Project context auto-injection

#include "AgentChatPanelModel.h"

#include <string>

struct AgentChatRuntimeSnapshot {
    std::string activeFile = "(no-active-buffer)";
    std::string language = "python";
    int astNodeCount = 0;
    int openAnnotationCount = 0;
    std::string workspacePath = ".";
};

struct AgentChatContextBlock {
    std::string activeFile;
    std::string language;
    int astNodeCount = 0;
    int openAnnotationCount = 0;
    std::string workspacePath;
};

class AgentChatContextInjector {
public:
    static AgentChatContextBlock fromSnapshot(const AgentChatRuntimeSnapshot& snapshot) {
        AgentChatContextBlock block;
        block.activeFile = snapshot.activeFile;
        block.language = snapshot.language;
        block.workspacePath = snapshot.workspacePath;
        block.astNodeCount = snapshot.astNodeCount;
        block.openAnnotationCount = snapshot.openAnnotationCount;
        return block;
    }

    static std::string toPromptText(const AgentChatContextBlock& block) {
        return "Context:\n"
               "- activeFile: " + block.activeFile + "\n"
               "- language: " + block.language + "\n"
               "- astNodeCount: " + std::to_string(block.astNodeCount) + "\n"
               "- openAnnotationCount: " + std::to_string(block.openAnnotationCount) + "\n"
               "- workspacePath: " + block.workspacePath;
    }

    static void inject(AgentChatState* chat,
                       const AgentChatRuntimeSnapshot& snapshot,
                       const std::string& timestamp) {
        if (!chat) return;
        AgentChatContextBlock block = fromSnapshot(snapshot);
        chat->systemContext = toPromptText(block);
        chat->messages.push_back({AgentChatRole::Assistant, chat->systemContext, timestamp});
        chat->autoScroll = true;
    }
};
