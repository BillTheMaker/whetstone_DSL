#pragma once
// Step 575: Requirement Normalization and Conflict Detection

#include "IntakeTextUtil.h"
#include "MarkdownSpecParser.h"

#include <cctype>
#include <map>
#include <set>
#include <string>
#include <vector>

enum class NormalizedRequirementKind {
    Goal,
    Constraint,
    Dependency,
    Acceptance
};

struct NormalizedRequirement {
    std::string requirementId;
    NormalizedRequirementKind kind = NormalizedRequirementKind::Goal;
    std::string normalizedText;
    std::string anchor;
    int sourceLine = 0;
    bool ambiguous = false;
};

struct RequirementConflict {
    std::string leftRequirementId;
    std::string rightRequirementId;
    std::string conflictType;
    std::string detail;
};

struct RequirementNormalizationResult {
    std::vector<NormalizedRequirement> requirements;
    std::vector<RequirementConflict> conflicts;
};

class RequirementNormalizationConflictDetector {
public:
    static bool normalize(const ParsedMarkdownSpec& spec,
                          RequirementNormalizationResult* outResult,
                          std::string* error) {
        if (!outResult || !error) return false;
        error->clear();
        RequirementNormalizationResult result;

        appendCategory(spec.goals, NormalizedRequirementKind::Goal, "goal", &result);
        appendCategory(spec.constraints, NormalizedRequirementKind::Constraint, "constraint", &result);
        appendCategory(spec.dependencies, NormalizedRequirementKind::Dependency, "dependency", &result);
        appendCategory(spec.acceptanceCriteria, NormalizedRequirementKind::Acceptance, "acceptance", &result);

        if (result.requirements.empty()) {
            *error = "no_requirements_found";
            return false;
        }

        result.conflicts = detectConflicts(result.requirements);
        *outResult = result;
        return true;
    }

private:
    static void appendCategory(const std::vector<ParsedRequirementItem>& items,
                               NormalizedRequirementKind kind,
                               const std::string& idPrefix,
                               RequirementNormalizationResult* out) {
        for (std::size_t i = 0; i < items.size(); ++i) {
            const auto& item = items[i];
            NormalizedRequirement requirement;
            requirement.requirementId = idPrefix + "-" + std::to_string(i + 1);
            requirement.kind = kind;
            requirement.normalizedText = normalizeText(item.text);
            requirement.anchor = item.anchor;
            requirement.sourceLine = item.line;
            requirement.ambiguous = isAmbiguous(requirement.normalizedText);
            out->requirements.push_back(requirement);
        }
    }

    static std::string normalizeText(const std::string& text) {
        return intakeCompactLowerAlnumSpaces(text);
    }

    static bool isAmbiguous(const std::string& normalizedText) {
        static const char* ambiguousTokens[] = {"maybe", "possibly", "somehow", "etc", "nice"};
        for (const auto* token : ambiguousTokens) {
            if (normalizedText.find(token) != std::string::npos) return true;
        }
        return false;
    }

    static std::set<std::string> tokenSet(const std::string& text) {
        std::set<std::string> tokens;
        std::string current;
        for (char c : text) {
            if (std::isalnum(static_cast<unsigned char>(c))) {
                current.push_back(c);
            } else if (!current.empty()) {
                tokens.insert(current);
                current.clear();
            }
        }
        if (!current.empty()) tokens.insert(current);
        return tokens;
    }

    static bool hasNegation(const std::set<std::string>& tokens) {
        return tokens.count("not") != 0 ||
               tokens.count("never") != 0 ||
               tokens.count("forbid") != 0 ||
               tokens.count("forbidden") != 0 ||
               tokens.count("disallow") != 0 ||
               tokens.count("disable") != 0;
    }

    static std::vector<RequirementConflict> detectConflicts(
        const std::vector<NormalizedRequirement>& requirements) {
        std::vector<RequirementConflict> conflicts;
        for (std::size_t i = 0; i < requirements.size(); ++i) {
            for (std::size_t j = i + 1; j < requirements.size(); ++j) {
                const auto& a = requirements[i];
                const auto& b = requirements[j];
                if (a.kind != NormalizedRequirementKind::Constraint ||
                    b.kind != NormalizedRequirementKind::Constraint) continue;

                const auto tokensA = tokenSet(a.normalizedText);
                const auto tokensB = tokenSet(b.normalizedText);
                if (tokensA.empty() || tokensB.empty()) continue;

                std::size_t overlap = 0;
                for (const auto& token : tokensA) {
                    if (tokensB.count(token) != 0) ++overlap;
                }
                if (overlap == 0) continue;

                const bool negationA = hasNegation(tokensA);
                const bool negationB = hasNegation(tokensB);
                if (negationA == negationB) continue;

                RequirementConflict conflict;
                conflict.leftRequirementId = a.requirementId;
                conflict.rightRequirementId = b.requirementId;
                conflict.conflictType = "constraint_contradiction";
                conflict.detail = "overlap_tokens=" + std::to_string(overlap);
                conflicts.push_back(conflict);
            }
        }
        return conflicts;
    }
};
