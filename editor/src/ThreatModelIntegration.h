#pragma once

// Step 490: Threat Model Integration
// Models trust boundaries, data flows, and validation diagnostics.

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

struct TrustBoundary {
    std::string name;  // external-api, database, user-input
    std::string level; // high, medium, low
};

struct ThreatDataFlow {
    std::string fromBoundary;
    std::string toBoundary;
    bool hasInputValidation = false;
    std::string validationKind; // sanitized/raw/escaped
};

struct ThreatModelInput {
    std::string moduleName;
    std::vector<TrustBoundary> boundaries;
    std::vector<ThreatDataFlow> flows;
};

struct ThreatDiagnostic {
    int code = 0; // E1300 range
    std::string message;
    std::string severity;
    std::string boundaryFrom;
    std::string boundaryTo;
};

struct ThreatModelOutput {
    std::vector<ThreatDiagnostic> diagnostics;
    std::vector<std::string> requiredAnnotations;
    std::vector<std::string> overlayNodes; // for GUI trust overlay
    std::vector<std::string> notes;
};

class ThreatModelIntegration {
public:
    static ThreatModelOutput analyze(const ThreatModelInput& in) {
        ThreatModelOutput out;
        out.requiredAnnotations.push_back("@ThreatModel(module=" + in.moduleName + ")");

        for (const auto& b : in.boundaries) {
            out.overlayNodes.push_back(b.name + ":" + b.level);
            out.requiredAnnotations.push_back("@TrustBoundary(" + b.name + "," + b.level + ")");
        }

        for (const auto& f : in.flows) {
            analyzeFlow(f, out);
        }

        if (in.boundaries.empty()) {
            out.diagnostics.push_back({1302, "No trust boundaries defined for module",
                                       "medium", "", ""});
        }
        if (in.flows.empty()) {
            out.notes.push_back("No data flows provided for threat analysis");
        }
        out.notes.push_back("Trust boundary overlay data prepared");
        return out;
    }

private:
    static std::string lower(const std::string& s) {
        std::string out = s;
        std::transform(out.begin(), out.end(), out.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return out;
    }

    static bool isExternalBoundary(const std::string& name) {
        const std::string n = lower(name);
        return n.find("external") != std::string::npos || n.find("user") != std::string::npos;
    }

    static void analyzeFlow(const ThreatDataFlow& flow, ThreatModelOutput& out) {
        const bool crossing = flow.fromBoundary != flow.toBoundary;
        if (crossing) {
            out.requiredAnnotations.push_back(
                "@DataFlow(" + flow.fromBoundary + "->" + flow.toBoundary + ")");
        }

        const bool highRiskCrossing = crossing &&
            (isExternalBoundary(flow.fromBoundary) || isExternalBoundary(flow.toBoundary));

        if (highRiskCrossing && !flow.hasInputValidation) {
            out.diagnostics.push_back({
                1300,
                "Missing input validation at trust boundary crossing",
                "high",
                flow.fromBoundary,
                flow.toBoundary
            });
            out.requiredAnnotations.push_back("@InputValidation(required)");
            return;
        }

        if (flow.hasInputValidation) {
            const std::string kind = lower(flow.validationKind);
            if (kind.empty() || kind == "raw") {
                out.diagnostics.push_back({
                    1301,
                    "Boundary flow validation exists but remains raw",
                    "medium",
                    flow.fromBoundary,
                    flow.toBoundary
                });
            } else if (kind != "sanitized" && kind != "escaped") {
                out.diagnostics.push_back({
                    1303,
                    "Unknown validation kind at trust boundary flow",
                    "low",
                    flow.fromBoundary,
                    flow.toBoundary
                });
            }
        }
    }
};
