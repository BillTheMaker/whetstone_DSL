#pragma once
#include "LayoutManager.h"

struct UIFlags {
    bool showWhitespace = false;
    bool showMinimap = false;
    bool showAnnotations = false;
    bool showOutline = true;
    bool showLineNumbers = true;
    bool showLspSettings = false;
    bool showSettingsPanel = false;
    bool showFirstRunWizard = false;
    bool showShortcutReference = false;
    bool showHelpPanel = false;
    bool showAnnotateWizard = false;
    bool showProjectWizard = false;
    bool showAgentWizard = false;
    int bottomTab = 0; // 0=Output,1=AST,2=Highlighted
    LayoutPreset layoutPreset = LayoutPreset::VSCode;
};
