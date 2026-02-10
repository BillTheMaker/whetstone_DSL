#pragma once
#include "EmacsIntegration.h"
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
                                          std::string& logOut) {
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
        logOut += "[emacs] Unbound: " + sequence + "\n";
        return true;
    }
    state.lastCommand = cmd;
    std::string result = emacs.sendCommand(ElispCommandBuilder::callInteractive(cmd));
    if (!emacs.getLastError().empty() && result == "error") {
        logOut += "[emacs] " + emacs.getLastError() + "\n";
    } else {
        logOut += "[emacs] " + cmd + "\n";
    }
    return true;
}

static inline bool emacsExecuteMinibuffer(EmacsKeybindingState& state,
                                          EmacsConnection& emacs,
                                          std::string& logOut) {
    std::string cmd = emacsTrim(state.minibufferBuf);
    if (cmd.empty()) {
        state.minibufferActive = false;
        return false;
    }
    std::string result = emacs.sendCommand(ElispCommandBuilder::executeExtendedCommand(cmd));
    if (!emacs.getLastError().empty() && result == "error") {
        logOut += "[emacs] " + emacs.getLastError() + "\n";
    } else {
        logOut += "[emacs] M-x " + cmd + "\n";
    }
    state.minibufferActive = false;
    state.minibufferBuf[0] = '\0';
    return true;
}

static inline void updateEmacsModeLine(EmacsKeybindingState& state,
                                       EmacsConnection& emacs,
                                       double nowSeconds,
                                       std::string& logOut) {
    if (nowSeconds - state.lastModeQuery < 1.0) return;
    std::string mode = emacs.sendCommand(ElispCommandBuilder::modeLine());
    if (!emacs.getLastError().empty() && mode == "error") {
        logOut += "[emacs] " + emacs.getLastError() + "\n";
        return;
    }
    if (!mode.empty()) state.modeLine = mode;
    state.lastModeQuery = nowSeconds;
}
