#pragma once
#include "DependencyPanel.h"
#include "LibraryBrowserPanel.h"
#include "CompositionPanel.h"
#include "LibraryIndexer.h"
#include "PrimitivesRegistry.h"
#include "VulnerabilityDatabase.h"

struct LibraryState {
    bool showDependencyPanel = true;
    DependencyPanelState dependencyPanel;
    bool showLibraryBrowserPanel = true;
    LibraryBrowserState libraryBrowser;
    bool showCompositionPanel = false;
    CompositionPanelState compositionPanel;

    struct LibraryIndexRequest {
        std::string name;
        std::string version;
        std::string source;
        int symbolsRequestId = -1;
        int completionRequestId = -1;
    };

    std::vector<LibraryIndexRequest> libraryIndexRequests;
    LibraryIndexData libraryIndex;
    PrimitivesRegistry primitives;
    VulnerabilityDatabase vulnDb;
};
