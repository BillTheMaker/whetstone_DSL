#pragma once
// Step 357: Status bar model.
// Headless representation used for UI rendering/tests.

#include <string>
#include "../ThemeData.h"

struct StatusBarInput {
    std::string language = "python";
    std::string filePath;
    int line = 1;
    int column = 1;
    std::string encoding = "UTF-8";
    std::string lineEnding = "LF";
    bool workflowActive = false;
    int workflowCurrent = 0;
    int workflowTotal = 0;
    bool mcpConnected = false;
    int errorCount = 0;
    int warningCount = 0;
    int infoCount = 0;
    int hintCount = 0;
};

struct StatusBarModel {
    std::string languageBadge;
    std::string leftText;
    std::string centerText;
    std::string rightText;
    std::string workflowBadge;
    std::string diagnosticsText;
    std::string mcpText;
    bool overflow = false;

    Color4 bgColor;
    Color4 textColor;
    Color4 dimTextColor;
    Color4 languageBadgeColor;
    Color4 diagnosticsColor;
    Color4 workflowColor;
};

inline Color4 statusLanguageColor(const std::string& language, const WhetstoneTheme& theme) {
    if (language == "python") return theme.syntaxFunction;
    if (language == "cpp") return theme.syntaxType;
    if (language == "kotlin") return theme.syntaxAnnotation;
    if (language == "rust") return theme.accent;
    if (language == "go") return theme.syntaxKeyword;
    return theme.textAccent;
}

inline StatusBarModel buildStatusBar(const StatusBarInput& in,
                                     const WhetstoneTheme& theme,
                                     size_t maxChars = 140) {
    StatusBarModel m;
    m.bgColor = theme.bgPanel;
    m.textColor = theme.text;
    m.dimTextColor = theme.textDim;
    m.languageBadgeColor = statusLanguageColor(in.language, theme);
    m.workflowColor = theme.textAccent;
    m.diagnosticsColor = (in.errorCount > 0) ? theme.diagError :
                         (in.warningCount > 0 ? theme.diagWarning : theme.diagInfo);

    m.languageBadge = in.language;
    m.leftText = in.language + "  " + in.filePath;
    m.centerText = "Ln " + std::to_string(in.line) + ", Col " + std::to_string(in.column) +
                   " | " + in.encoding + " | " + in.lineEnding;
    m.diagnosticsText = std::to_string(in.errorCount) + " errors, " +
                        std::to_string(in.warningCount) + " warnings";
    m.mcpText = in.mcpConnected ? "MCP: connected" : "MCP: offline";

    if (in.workflowActive && in.workflowTotal > 0) {
        m.workflowBadge = "Executing " + std::to_string(in.workflowCurrent) +
                          "/" + std::to_string(in.workflowTotal);
    } else {
        m.workflowBadge = "";
    }

    m.rightText = m.diagnosticsText;
    if (!m.workflowBadge.empty()) m.rightText += " | " + m.workflowBadge;
    m.rightText += " | " + m.mcpText;

    const size_t total = m.leftText.size() + m.centerText.size() + m.rightText.size();
    m.overflow = total > maxChars;
    return m;
}

inline std::string statusBarClickDiagnosticsTarget() {
    return "diagnostics";
}
