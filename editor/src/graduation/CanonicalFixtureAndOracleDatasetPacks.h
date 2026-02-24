#pragma once
// Step 1152: Canonical fixture and oracle dataset packs.
#include <string>
#include <nlohmann/json.hpp>

struct CanonicalFixtureAndOracleDatasetPacks {
    std::string packId;
    int fixtureCount = 0;
    int oracleCount = 0;
    std::string version;
    bool checksummed = false;
    bool publishable = false;
};

class CanonicalFixtureAndOracleDatasetPacksFactory {
public:
    static CanonicalFixtureAndOracleDatasetPacks make(const std::string& packId,
                                                      int fixtureCount,
                                                      int oracleCount,
                                                      const std::string& version,
                                                      bool checksummed) {
        bool publishable = !packId.empty() && fixtureCount > 0 && oracleCount > 0 &&
                           !version.empty() && checksummed;
        return {packId, fixtureCount, oracleCount, version, checksummed, publishable};
    }

    static nlohmann::json toJson(const CanonicalFixtureAndOracleDatasetPacks& p) {
        return {{"pack_id", p.packId},
                {"fixture_count", p.fixtureCount},
                {"oracle_count", p.oracleCount},
                {"version", p.version},
                {"checksummed", p.checksummed},
                {"publishable", p.publishable}};
    }
};
