#pragma once
#include "ProjectSearch.h"

struct SearchState {
    bool showFind = false;
    char findBuf[256] = {};
    char replaceBuf[256] = {};
    int lastFindPos = 0;
    int pulseLine = -1;
    double pulseStart = 0.0;

    bool showProjectSearch = false;
    char searchQuery[256] = {};
    char searchInclude[256] = {};
    char searchExclude[256] = {};
    bool searchUseRegex = true;
    std::vector<ProjectSearch::FileResult> searchResults;

    bool showGoToLine = false;
    char goToLineBuf[64] = {};
    bool goToLineError = false;

    ProjectSearch projectSearch;
};
