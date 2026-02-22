#pragma once
// Step 774: Macro boundary + hygienic expansion packet model.

#include <string>

#include <nlohmann/json.hpp>

#include "SExpressionCanonicalLowering.h"

struct MacroBoundaryPacket {
    bool macroBoundaryPresent = false;
    bool hygienicByDefault = false;
    std::string expansionPolicy;
    int expansionRisk = 0;
};

class MacroHygieneBoundaryModel {
public:
    static MacroBoundaryPacket classify(const ASTNativeLoweringPacket& p, const std::string& sourceLanguage) {
        MacroBoundaryPacket out;
        out.macroBoundaryPresent = p.macroLike;
        out.hygienicByDefault = (sourceLanguage == "scheme");
        if (!p.macroLike) {
            out.expansionPolicy = "no_macro_expansion";
            out.expansionRisk = 0;
        } else if (out.hygienicByDefault) {
            out.expansionPolicy = "hygienic_expansion";
            out.expansionRisk = 1;
        } else {
            out.expansionPolicy = "guarded_expansion";
            out.expansionRisk = 2;
        }
        return out;
    }

    static nlohmann::json toJson(const MacroBoundaryPacket& p) {
        return {
            {"macro_boundary_present", p.macroBoundaryPresent},
            {"hygienic_by_default", p.hygienicByDefault},
            {"expansion_policy", p.expansionPolicy},
            {"expansion_risk", p.expansionRisk}
        };
    }
};
