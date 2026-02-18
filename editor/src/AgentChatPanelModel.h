#pragma once
// Step 624: Agent Chat Panel model

#include <cctype>
#include <string>
#include <vector>

enum class AgentChatRole {
    User,
    Assistant,
    Tool
};

struct AgentChatMessage {
    AgentChatRole role = AgentChatRole::User;
    std::string content;
    std::string timestamp;
};

struct AgentChatState {
    std::vector<AgentChatMessage> messages;
    std::string draftInput;
    bool autoScroll = true;
    bool open = false;
};

class AgentChatPanelModel {
public:
    static bool sendUserMessage(AgentChatState* state, const std::string& timestamp) {
        if (!state) return false;
        std::string text = trimCopy(state->draftInput);
        if (text.empty()) return false;
        state->messages.push_back({AgentChatRole::User, text, timestamp});
        state->draftInput.clear();
        state->autoScroll = true;
        return true;
    }

    static void addAssistantMessage(AgentChatState* state,
                                    const std::string& content,
                                    const std::string& timestamp) {
        if (!state) return;
        state->messages.push_back({AgentChatRole::Assistant, content, timestamp});
        state->autoScroll = true;
    }

    static void addToolMessage(AgentChatState* state,
                               const std::string& content,
                               const std::string& timestamp) {
        if (!state) return;
        state->messages.push_back({AgentChatRole::Tool, content, timestamp});
        state->autoScroll = true;
    }

    static const char* roleLabel(AgentChatRole role) {
        if (role == AgentChatRole::User) return "user";
        if (role == AgentChatRole::Assistant) return "assistant";
        return "tool";
    }

    static std::string trimCopy(const std::string& value) {
        std::size_t begin = 0;
        while (begin < value.size() &&
               std::isspace(static_cast<unsigned char>(value[begin]))) {
            ++begin;
        }
        std::size_t end = value.size();
        while (end > begin &&
               std::isspace(static_cast<unsigned char>(value[end - 1]))) {
            --end;
        }
        return value.substr(begin, end - begin);
    }
};
