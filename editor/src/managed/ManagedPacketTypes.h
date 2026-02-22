#pragma once
// Shared packet contracts for Sprint 53 managed family adapters.

#include <string>

struct ManagedLoweringPacket {
    std::string sourceLanguage;
    std::string irSummary;
    bool hasNullableSyntax = false;
    bool hasOptionalType = false;
    int asyncSignalCount = 0;
    bool adtLike = false;
};

struct ManagedRaisingPacket {
    std::string targetLanguage;
    std::string codePreview;
    std::string nullabilityModel;
    std::string asyncModel;
};
