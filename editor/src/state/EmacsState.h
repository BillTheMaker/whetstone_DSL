#pragma once
#include "EmacsIntegration.h"
#include "EmacsPackageBrowser.h"
#include "EmacsFunctionDiscovery.h"
#include "EmacsKeybinding.h"
#include "OrgMode.h"

struct EmacsState {
    EmacsConnection emacs;
    bool showEmacsPackagesPanel = false;
    EmacsPackageBrowserState emacsPackages;
    EmacsFunctionIndex emacsFunctionIndex;
    bool emacsFunctionIndexDirty = true;
    EmacsKeybindingState emacsKeys;
    bool showEmacsBridgePanel = false;
    bool showOrgPanel = true;
    OrgDocumentState orgDoc;
    int orgTempCounter = 0;
};
