#pragma once
// Step 77: Custom code editor renderer
//
// Immediate-mode code editor widget that renders text with syntax colors,
// handles cursor/selection/input, and supports a visible whitespace toggle.
// This is intentionally minimal; line numbers, gutters, folding, and
// advanced selection behaviors are added in later steps.

#include "imgui.h"
#include "SyntaxHighlighter.h"
#include "ThemeEngine.h"
#include "EditorMode.h"
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <cstring>

struct CodeEditorOptions {
    bool showWhitespace = false;
    bool readOnly = false;
    EditorMode* mode = nullptr;
    bool enableFolding = false;
    bool showMinimap = false;
    bool showAnnotations = false;
    bool showLineNumbers = true;
    bool showCurrentLine = true;
    int annotationLayout = 0; // 0=above, 1=margin, 2=beside
    const std::vector<int>* errorLines = nullptr;
    const std::vector<int>* warningLines = nullptr;
    const std::vector<struct DiagnosticRange>* diagnostics = nullptr;
    const std::vector<struct AnnotationMarker>* annotations = nullptr;
    const std::vector<struct SuggestionMarker>* suggestions = nullptr;
    const std::vector<struct AnnotationConflictMarker>* conflicts = nullptr;
    int highlightLine = -1;
    const std::vector<int>* highlightLines = nullptr;
    ImU32 highlightLineColor = IM_COL32(90, 70, 30, 120);
    float* syncScrollX = nullptr;
    float* syncScrollY = nullptr;
    bool scrollMaster = false;
};

struct DiagnosticRange {
    int startLine = 0;
    int startCol = 0;
    int endLine = 0;
    int endCol = 0;
    int severity = 0; // 1=error, 2=warning
    std::string message;
};

struct AnnotationMarker {
    int line = 0;
    ImU32 color = 0;
    std::string message;
};

struct SuggestionMarker {
    int line = 0;
    double confidence = 0.0;
    std::string label;
    std::string reason;
    std::string annotationType;
    std::string strategy;
    std::string nodeId;
};

struct CodeEditorResult {
    bool changed = false;
    int cursorByte = 0;
    int lineCount = 0;
    float gutterWidth = 0.0f;
    int currentLine = 0;
    int foldCount = 0;
    bool anyFolded = false;
    bool minimapEnabled = false;
    float minimapWidth = 0.0f;
    float minimapViewportStart = 0.0f;
    float minimapViewportEnd = 0.0f;
    bool suggestionClicked = false;
    SuggestionMarker clickedSuggestion;
    bool lineClicked = false;
    int clickedLine = -1;
    bool ctrlClick = false;
    int ctrlClickLine = -1;
    int ctrlClickCol = -1;
    int ctrlClickPos = -1;
    bool hoverValid = false;
    int hoverLine = -1;
    int hoverCol = -1;
    int hoverPos = -1;
};

struct AnnotationConflictMarker {
    int childLine = -1;
    int parentLine = -1;
    std::string message;
    std::string childAnnoId;
    std::string parentAnnoId;
    std::string parentStrategy;
};

struct FoldRegion {
    int startLine = 0;
    int endLine = 0;
    bool folded = false;
};

class CodeEditorWidget {
#include "CodeEditorRendering.h"
#include "CodeEditorSelection.h"
};
