#pragma once
#include "EmacsIntegration.h"
#include "NotificationSystem.h"
#include <string>
#include <algorithm>
#include <cctype>

struct EmacsKeybindingState {
    std::string prefix;
    std::string lastCommand;
    std::string lastMessage;
    bool minibufferActive = false;
    char minibufferBuf[256] = {};
    std::string minibufferPrompt = "M-x";
    std::string modeLine;
    double lastModeQuery = 0.0;
};

static inline std::string emacsTrim(std::string value) {
    auto notSpace = [](unsigned char c) { return !std::isspace(c); };
    value.erase(value.begin(), std::find_if(value.begin(), value.end(), notSpace));
    value.erase(std::find_if(value.rbegin(), value.rend(), notSpace).base(), value.end());
    return value;
}

static inline bool emacsIsPrefixKey(const std::string& chord) {
    return chord == "C-x" || chord == "C-c";
}

static inline bool emacsHandleKeySequence(EmacsKeybindingState& state,
                                          EmacsConnection& emacs,
                                          const std::string& chord,
                                          NotificationSystem& notifications) {
    if (chord.empty()) return false;
    if (chord == "M-x") {
        state.minibufferActive = true;
        state.minibufferPrompt = "M-x";
        state.minibufferBuf[0] = '\0';
        state.prefix.clear();
        return true;
    }
    if (emacsIsPrefixKey(chord)) {
        state.prefix = chord;
        state.lastMessage = chord;
        return true;
    }

    std::string sequence = state.prefix.empty() ? chord : (state.prefix + " " + chord);
    state.prefix.clear();

    std::string cmd = emacs.sendCommand(ElispCommandBuilder::keyBinding(sequence));
    if (cmd.empty()) {
        notifications.notify(NotificationLevel::Warning,
                             "[emacs] Unbound: " + sequence);
        return true;
    }
    state.lastCommand = cmd;
    std::string result = emacs.sendCommand(ElispCommandBuilder::callInteractive(cmd));
    if (!emacs.getLastError().empty() && result == "error") {
        notifications.notify(NotificationLevel::Error,
                             "[emacs] " + emacs.getLastError());
    } else {
        notifications.notify(NotificationLevel::Info,
                             "[emacs] " + cmd);
    }
    return true;
}

static inline bool emacsExecuteMinibuffer(EmacsKeybindingState& state,
                                          EmacsConnection& emacs,
                                          NotificationSystem& notifications) {
    std::string cmd = emacsTrim(state.minibufferBuf);
    if (cmd.empty()) {
        state.minibufferActive = false;
        return false;
    }
    std::string result = emacs.sendCommand(ElispCommandBuilder::executeExtendedCommand(cmd));
    if (!emacs.getLastError().empty() && result == "error") {
        notifications.notify(NotificationLevel::Error,
                             "[emacs] " + emacs.getLastError());
    } else {
        notifications.notify(NotificationLevel::Info,
                             "[emacs] M-x " + cmd);
    }
    state.minibufferActive = false;
    state.minibufferBuf[0] = '\0';
    return true;
}

static inline void updateEmacsModeLine(EmacsKeybindingState& state,
                                       EmacsConnection& emacs,
                                       double nowSeconds,
                                       NotificationSystem& notifications) {
    if (nowSeconds - state.lastModeQuery < 1.0) return;
    std::string mode = emacs.sendCommand(ElispCommandBuilder::modeLine());
    if (!emacs.getLastError().empty() && mode == "error") {
        notifications.notify(NotificationLevel::Error,
                             "[emacs] " + emacs.getLastError());
        return;
    }
    if (!mode.empty()) state.modeLine = mode;
    state.lastModeQuery = nowSeconds;
}
