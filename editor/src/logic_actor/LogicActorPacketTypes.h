#pragma once
// Shared packet types for Sprint 55 logic+actor family.

#include <string>

struct LogicActorLoweringPacket {
    std::string sourceLanguage;
    std::string irSummary;
    int queryArity = 0;
    bool backtracking = false;
    bool actorModel = false;
    bool supervision = false;
    bool macroSurface = false;
};

struct LogicActorProjectionPacket {
    std::string targetLanguage;
    std::string policy;
    bool reviewRequired = false;
};
