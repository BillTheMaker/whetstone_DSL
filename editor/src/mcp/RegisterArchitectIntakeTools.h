    void registerArchitectIntakeTools() {
        tools_.push_back({"whetstone_architect_intake",
            "Parse a markdown project spec into structured sections and normalized requirements with conflict signals.",
            {{"type", "object"}, {"properties", {
                {"markdown", {{"type", "string"},
                    {"description", "Markdown specification text."}}}
            }}, {"required", json::array({"markdown"})}}
        });
        toolHandlers_["whetstone_architect_intake"] =
            [this](const json& args) {
                return runArchitectIntake(args);
            };
    }

    json runArchitectIntake(const json& args) {
        if (!args.contains("markdown")) {
            return {
                {"success", false},
                {"error", "markdown_missing"}
            };
        }
        if (!args["markdown"].is_string()) {
            return {
                {"success", false},
                {"error", "markdown_not_string"}
            };
        }

        ParsedMarkdownSpec parsed;
        std::string error;
        std::string markdown = args.value("markdown", "");
        if (!MarkdownSpecParser::parse(markdown, &parsed, &error)) {
            return {
                {"success", false},
                {"error", error}
            };
        }

        RequirementNormalizationResult normalized;
        if (!RequirementNormalizationConflictDetector::normalize(parsed, &normalized, &error)) {
            return {
                {"success", false},
                {"error", error},
                {"parsedSpec", parsedSpecToJson(parsed)}
            };
        }

        const int ambiguousCount = countAmbiguousRequirements(normalized.requirements);
        return {
            {"success", true},
            {"parsedSpec", parsedSpecToJson(parsed)},
            {"normalizedRequirements", normalizedRequirementsToJson(normalized.requirements)},
            {"conflicts", conflictsToJson(normalized.conflicts)},
            {"conflictSignals", {
                {"hasConflicts", !normalized.conflicts.empty()},
                {"conflictCount", (int)normalized.conflicts.size()},
                {"ambiguousRequirementCount", ambiguousCount}
            }}
        };
    }

    static std::string requirementKindToString(NormalizedRequirementKind kind) {
        if (kind == NormalizedRequirementKind::Goal) return "goal";
        if (kind == NormalizedRequirementKind::Constraint) return "constraint";
        if (kind == NormalizedRequirementKind::Dependency) return "dependency";
        return "acceptance";
    }

    static json parsedSpecToJson(const ParsedMarkdownSpec& parsed) {
        json sections = json::array();
        for (const auto& section : parsed.sections) {
            sections.push_back({
                {"title", section.title},
                {"anchor", section.anchor},
                {"headingLine", section.headingLine}
            });
        }

        auto itemsToJson = [](const std::vector<ParsedRequirementItem>& items) {
            json out = json::array();
            for (const auto& item : items) {
                out.push_back({
                    {"text", item.text},
                    {"sectionTitle", item.sectionTitle},
                    {"anchor", item.anchor},
                    {"line", item.line}
                });
            }
            return out;
        };

        return {
            {"sections", sections},
            {"goals", itemsToJson(parsed.goals)},
            {"constraints", itemsToJson(parsed.constraints)},
            {"dependencies", itemsToJson(parsed.dependencies)},
            {"acceptanceCriteria", itemsToJson(parsed.acceptanceCriteria)}
        };
    }

    static json normalizedRequirementsToJson(const std::vector<NormalizedRequirement>& requirements) {
        json out = json::array();
        for (const auto& requirement : requirements) {
            out.push_back({
                {"requirementId", requirement.requirementId},
                {"kind", requirementKindToString(requirement.kind)},
                {"normalizedText", requirement.normalizedText},
                {"anchor", requirement.anchor},
                {"sourceLine", requirement.sourceLine},
                {"ambiguous", requirement.ambiguous}
            });
        }
        return out;
    }

    static json conflictsToJson(const std::vector<RequirementConflict>& conflicts) {
        json out = json::array();
        for (const auto& conflict : conflicts) {
            out.push_back({
                {"leftRequirementId", conflict.leftRequirementId},
                {"rightRequirementId", conflict.rightRequirementId},
                {"conflictType", conflict.conflictType},
                {"detail", conflict.detail}
            });
        }
        return out;
    }

    static int countAmbiguousRequirements(const std::vector<NormalizedRequirement>& requirements) {
        int count = 0;
        for (const auto& requirement : requirements) {
            if (requirement.ambiguous) ++count;
        }
        return count;
    }
