#pragma once
// Step 1878: Constrained projection gate.
// Closes GR-012: ProjectionGenerator.generate() has no access to EnvironmentSpec,
// so generators emit code without checking environment constraints. This gate runs
// validateCapabilities + validateEnvAnnotations before generation and blocks on errors.

#include "EnvironmentSpec.h"
#include "ast/ASTNode.h"

#include <string>
#include <vector>

struct ProjectionConstraintResult {
    bool blocked = false;
    std::vector<CapabilityDiagnostic> diagnostics;
};

class ConstrainedProjectionGate {
public:
    // Validate the AST node against the given environment before generation.
    // Returns blocked=true when any "error" severity diagnostic is found.
    // A null env means no constraints — always passes.
    static ProjectionConstraintResult validate(const ASTNode* node,
                                               const EnvironmentSpec* env) {
        ProjectionConstraintResult result;
        if (!env) return result; // no env = no constraints

        auto capDiags = validateCapabilities(node, env);
        auto annDiags = validateEnvAnnotations(node, env);

        for (const auto& d : capDiags) result.diagnostics.push_back(d);
        for (const auto& d : annDiags) result.diagnostics.push_back(d);

        for (const auto& d : result.diagnostics) {
            if (d.severity == "error") {
                result.blocked = true;
                break;
            }
        }
        return result;
    }
};
