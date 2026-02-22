#pragma once
// Step 690: Language support tiers and gate mapping model.

#include <set>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

enum class SupportTier { Experimental, Beta, Stable, Unknown };

struct TierGateSet {
    bool parseAndProject = false;
    bool executableEquivalence = false;
    bool staticChecks = false;
    bool securityGates = false;
    bool performanceGates = false;
    bool migrationReportQuality = false;
};

struct LanguageTierStatus {
    std::string language;
    SupportTier tier = SupportTier::Experimental;
    TierGateSet gates;
    std::vector<std::string> warnings;
};

class LanguageSupportTierModel {
public:
    static std::string tierToString(SupportTier t) {
        switch (t) {
            case SupportTier::Experimental: return "experimental";
            case SupportTier::Beta: return "beta";
            case SupportTier::Stable: return "stable";
            default: return "unknown";
        }
    }

    static SupportTier tierFromString(const std::string& s) {
        if (s == "experimental") return SupportTier::Experimental;
        if (s == "beta") return SupportTier::Beta;
        if (s == "stable") return SupportTier::Stable;
        return SupportTier::Unknown;
    }

    static TierGateSet requiredGates(SupportTier tier) {
        TierGateSet g;
        if (tier == SupportTier::Unknown) return g;
        g.parseAndProject = true;
        if (tier == SupportTier::Beta || tier == SupportTier::Stable) {
            g.executableEquivalence = true;
            g.staticChecks = true;
        }
        if (tier == SupportTier::Stable) {
            g.securityGates = true;
            g.performanceGates = true;
            g.migrationReportQuality = true;
        }
        return g;
    }

    static bool gateSuperset(const TierGateSet& lhs, const TierGateSet& rhs) {
        return (!rhs.parseAndProject || lhs.parseAndProject) &&
               (!rhs.executableEquivalence || lhs.executableEquivalence) &&
               (!rhs.staticChecks || lhs.staticChecks) &&
               (!rhs.securityGates || lhs.securityGates) &&
               (!rhs.performanceGates || lhs.performanceGates) &&
               (!rhs.migrationReportQuality || lhs.migrationReportQuality);
    }

    static bool isUpgradePathValid(SupportTier from, SupportTier to) {
        return rank(to) >= rank(from) && from != SupportTier::Unknown && to != SupportTier::Unknown;
    }

    static std::vector<std::string> downgradeWarnings(SupportTier from, SupportTier to) {
        std::vector<std::string> out;
        if (rank(to) >= rank(from)) return out;
        out.push_back("tier_downgrade_detected");
        if (from == SupportTier::Stable && to != SupportTier::Stable) {
            out.push_back("stable_guarantees_removed");
        }
        if (from != SupportTier::Experimental && to == SupportTier::Experimental) {
            out.push_back("equivalence_guarantees_removed");
        }
        return out;
    }

    static nlohmann::json toJson(const LanguageTierStatus& s) {
        return {
            {"language", s.language},
            {"tier", tierToString(s.tier)},
            {"gates", {
                {"parseAndProject", s.gates.parseAndProject},
                {"executableEquivalence", s.gates.executableEquivalence},
                {"staticChecks", s.gates.staticChecks},
                {"securityGates", s.gates.securityGates},
                {"performanceGates", s.gates.performanceGates},
                {"migrationReportQuality", s.gates.migrationReportQuality}
            }},
            {"warnings", s.warnings}
        };
    }

    static bool fromJson(const nlohmann::json& j, LanguageTierStatus* out, std::string* error) {
        if (!out || !error) return false;
        *error = "";
        if (!j.is_object()) {
            *error = "status_not_object";
            return false;
        }
        LanguageTierStatus s;
        s.language = j.value("language", "");
        s.tier = tierFromString(j.value("tier", "experimental"));
        s.gates.parseAndProject = j.value("gates", nlohmann::json::object()).value("parseAndProject", false);
        s.gates.executableEquivalence = j.value("gates", nlohmann::json::object()).value("executableEquivalence", false);
        s.gates.staticChecks = j.value("gates", nlohmann::json::object()).value("staticChecks", false);
        s.gates.securityGates = j.value("gates", nlohmann::json::object()).value("securityGates", false);
        s.gates.performanceGates = j.value("gates", nlohmann::json::object()).value("performanceGates", false);
        s.gates.migrationReportQuality = j.value("gates", nlohmann::json::object()).value("migrationReportQuality", false);
        s.warnings = j.value("warnings", std::vector<std::string>{});
        if (s.language.empty()) {
            *error = "language_missing";
            return false;
        }
        if (s.tier == SupportTier::Unknown) {
            *error = "tier_unknown";
            return false;
        }
        *out = std::move(s);
        return true;
    }

    static std::vector<LanguageTierStatus> sortByLanguage(const std::vector<LanguageTierStatus>& in) {
        std::vector<LanguageTierStatus> out = in;
        std::sort(out.begin(), out.end(),
                  [](const LanguageTierStatus& a, const LanguageTierStatus& b) {
                      return a.language < b.language;
                  });
        return out;
    }

    static std::set<std::string> exportDocs(const std::vector<LanguageTierStatus>& statuses) {
        std::set<std::string> lines;
        for (const auto& s : statuses) {
            lines.insert(s.language + ":" + tierToString(s.tier));
        }
        return lines;
    }

private:
    static int rank(SupportTier t) {
        if (t == SupportTier::Experimental) return 0;
        if (t == SupportTier::Beta) return 1;
        if (t == SupportTier::Stable) return 2;
        return -1;
    }
};
