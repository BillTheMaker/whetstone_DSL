#pragma once
// Step 635: Capability declaration struct generator

#include <algorithm>
#include <string>
#include <vector>

struct CapabilityDeclarationOutput {
    bool success = false;
    std::string headerCode;
    std::vector<std::string> typeNames;
};

class CapabilityDeclarationGenerator {
public:
    static CapabilityDeclarationOutput generate(const std::string& namespaceName = "whetstone") {
        CapabilityDeclarationOutput out;
        if (namespaceName.empty()) return out;

        out.typeNames = {
            "NodeCapability",
            "CapabilitySet",
            "EnergyContext",
            "JobRequirements"
        };

        std::string code;
        code += "#pragma once\n";
        code += "#include <string>\n";
        code += "#include <vector>\n\n";
        code += "namespace " + namespaceName + " {\n\n";
        code += "struct NodeCapability {\n";
        code += "    std::string name;\n";
        code += "    int level = 0;\n";
        code += "};\n\n";
        code += "struct CapabilitySet {\n";
        code += "    std::vector<NodeCapability> items;\n";
        code += "};\n\n";
        code += "struct EnergyContext {\n";
        code += "    std::string mode = \"Normal\";\n";
        code += "    int wattsAvailable = 0;\n";
        code += "};\n\n";
        code += "struct JobRequirements {\n";
        code += "    std::vector<std::string> requiredCaps;\n";
        code += "    int minLevel = 0;\n";
        code += "};\n\n";
        code += "inline bool supports(const CapabilitySet& caps, const JobRequirements& req) {\n";
        code += "    if (req.requiredCaps.empty()) return true;\n";
        code += "    for (const auto& needed : req.requiredCaps) {\n";
        code += "        bool found = false;\n";
        code += "        for (const auto& have : caps.items) {\n";
        code += "            if (have.name == needed && have.level >= req.minLevel) { found = true; break; }\n";
        code += "        }\n";
        code += "        if (!found) return false;\n";
        code += "    }\n";
        code += "    return true;\n";
        code += "}\n\n";
        code += "} // namespace " + namespaceName + "\n";

        out.success = true;
        out.headerCode = code;
        return out;
    }

    static bool containsType(const CapabilityDeclarationOutput& out, const std::string& typeName) {
        return std::find(out.typeNames.begin(), out.typeNames.end(), typeName) != out.typeNames.end();
    }
};
