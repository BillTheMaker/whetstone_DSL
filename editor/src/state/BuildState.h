#pragma once
#include "TerminalPanel.h"
#include "BuildSystem.h"

struct BuildState {
    bool showTerminalPanel = false;
    TerminalPanel terminal;
    bool runInProgress = false;
    bool hasRunResult = false;
    int lastRunExitCode = 0;
    std::string lastRunCommand;

    BuildSystem::Type buildType = BuildSystem::Type::None;
    std::vector<BuildError> buildErrors;
    std::string lastBuildOutput;
    std::string lastBuildCommand;
};
