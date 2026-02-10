#pragma once
#include "ProjectSearch.h"
#include "../SearchUtils.h"

struct SearchState {
    bool showFind = false;
    bool showReplace = false;
    char findBuf[256] = {};
    char replaceBuf[256] = {};
    int lastFindPos = 0;
    bool matchCase = false;
    bool wholeWord = false;
    bool useRegex = false;
    bool findInSelection = false;
    int currentMatchIndex = -1;
    std::vector<SearchMatch> matches;
    std::vector<std::string> findHistory;
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
