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

        tools_.push_back({"whetstone_generate_taskitems",
            "Generate and annotate taskitems from normalized intake requirements.",
            {{"type", "object"}, {"properties", {
                {"normalizedRequirements", {{"type", "array"},
                    {"description", "Normalized requirements from whetstone_architect_intake."}}},
                {"conflicts", {{"type", "array"},
                    {"description", "Optional requirement conflicts from whetstone_architect_intake."}}}
            }}, {"required", json::array({"normalizedRequirements"})}}
        });
        toolHandlers_["whetstone_generate_taskitems"] =
            [this](const json& args) {
                return runGenerateTaskitems(args);
            };

        tools_.push_back({"whetstone_queue_ready",
            "Evaluate queue readiness for annotated taskitems.",
            {{"type", "object"}, {"properties", {
                {"tasks", {{"type", "array"},
                    {"description", "Annotated taskitems from whetstone_generate_taskitems."}}},
                {"normalizedRequirements", {{"type", "array"},
                    {"description", "Normalized requirements (needed for acceptance-coverage binding)."}}}
            }}, {"required", json::array({"tasks"})}}
        });
        toolHandlers_["whetstone_queue_ready"] =
            [this](const json& args) {
                return runQueueReady(args);
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

    json runGenerateTaskitems(const json& args) {
        RequirementNormalizationResult normalized;
        std::string error;
        if (!parseNormalizedInput(args, &normalized, &error)) {
            return {
                {"success", false},
                {"error", error}
            };
        }

        DecomposedScopePlan plan;
        if (!ScopeMilestoneDecomposer::decompose(normalized, &plan, &error)) {
            return {
                {"success", false},
                {"error", error}
            };
        }

        std::vector<GeneratedTaskitem> generated;
        if (!TaskitemGeneratorV2::generate(plan, &generated, &error)) {
            return {
                {"success", false},
                {"error", error}
            };
        }

        std::vector<AnnotatedTaskitem> annotated;
        if (!TaskitemConfidenceAmbiguity::annotate(generated, normalized, &annotated, &error)) {
            return {
                {"success", false},
                {"error", error}
            };
        }

        int escalateCount = 0;
        for (const auto& task : annotated) {
            if (task.escalate) ++escalateCount;
        }

        return {
            {"success", true},
            {"tasks", annotatedTaskitemsToJson(annotated)},
            {"planSummary", {
                {"milestoneCount", (int)plan.milestones.size()},
                {"overallUncertainty", plan.overallUncertainty}
            }},
            {"conflictCount", (int)normalized.conflicts.size()},
            {"ambiguousRequirementCount", countAmbiguousRequirements(normalized.requirements)},
            {"escalateCount", escalateCount}
        };
    }

    static bool parseNormalizedInput(const json& args,
                                     RequirementNormalizationResult* out,
                                     std::string* error) {
        if (!out || !error) return false;
        error->clear();
        out->requirements.clear();
        out->conflicts.clear();

        if (!args.contains("normalizedRequirements")) {
            *error = "normalized_requirements_missing";
            return false;
        }
        if (!args["normalizedRequirements"].is_array()) {
            *error = "normalized_requirements_not_array";
            return false;
        }

        for (const auto& requirementJson : args["normalizedRequirements"]) {
            if (!requirementJson.is_object()) {
                *error = "requirement_entry_invalid";
                return false;
            }
            NormalizedRequirement requirement;
            requirement.requirementId = requirementJson.value("requirementId", "");
            requirement.normalizedText = requirementJson.value("normalizedText", "");
            requirement.anchor = requirementJson.value("anchor", "");
            requirement.sourceLine = requirementJson.value("sourceLine", 0);
            requirement.ambiguous = requirementJson.value("ambiguous", false);
            if (requirement.requirementId.empty() || requirement.normalizedText.empty()) {
                *error = "requirement_entry_missing_fields";
                return false;
            }

            if (!parseRequirementKind(requirementJson.value("kind", ""), &requirement.kind)) {
                *error = "requirement_kind_invalid";
                return false;
            }
            out->requirements.push_back(requirement);
        }

        if (out->requirements.empty()) {
            *error = "normalized_requirements_empty";
            return false;
        }

        if (!args.contains("conflicts")) return true;
        if (!args["conflicts"].is_array()) {
            *error = "conflicts_not_array";
            return false;
        }
        for (const auto& conflictJson : args["conflicts"]) {
            if (!conflictJson.is_object()) {
                *error = "conflict_entry_invalid";
                return false;
            }
            RequirementConflict conflict;
            conflict.leftRequirementId = conflictJson.value("leftRequirementId", "");
            conflict.rightRequirementId = conflictJson.value("rightRequirementId", "");
            conflict.conflictType = conflictJson.value("conflictType", "");
            conflict.detail = conflictJson.value("detail", "");
            if (conflict.leftRequirementId.empty() || conflict.rightRequirementId.empty()) {
                *error = "conflict_entry_missing_fields";
                return false;
            }
            out->conflicts.push_back(conflict);
        }
        return true;
    }

    static bool parseRequirementKind(const std::string& kind,
                                     NormalizedRequirementKind* outKind) {
        if (!outKind) return false;
        if (kind == "goal") {
            *outKind = NormalizedRequirementKind::Goal;
            return true;
        }
        if (kind == "constraint") {
            *outKind = NormalizedRequirementKind::Constraint;
            return true;
        }
        if (kind == "dependency") {
            *outKind = NormalizedRequirementKind::Dependency;
            return true;
        }
        if (kind == "acceptance") {
            *outKind = NormalizedRequirementKind::Acceptance;
            return true;
        }
        return false;
    }

    static json annotatedTaskitemsToJson(const std::vector<AnnotatedTaskitem>& tasks) {
        json out = json::array();
        for (const auto& task : tasks) {
            out.push_back({
                {"taskId", task.base.taskId},
                {"title", task.base.title},
                {"milestoneId", task.base.milestoneId},
                {"dependencyTaskIds", task.base.dependencyTaskIds},
                {"prerequisiteOps", task.base.prerequisiteOps},
                {"queueReady", task.base.queueReady},
                {"confidence", task.confidence},
                {"ambiguityCount", task.ambiguityCount},
                {"escalate", task.escalate},
                {"reasons", task.reasons}
            });
        }
        return out;
    }

    json runQueueReady(const json& args) {
        std::vector<AnnotatedTaskitem> tasks;
        std::string error;
        if (!parseAnnotatedTasksInput(args, &tasks, &error)) {
            return {
                {"success", false},
                {"error", error}
            };
        }

        std::vector<NormalizedRequirement> requirements;
        if (!parseNormalizedRequirementsOptional(args, &requirements, &error)) {
            return {
                {"success", false},
                {"error", error}
            };
        }

        std::vector<BoundTaskitem> queue;
        bool bindOk = AcceptanceCriteriaBinding::bind(tasks, requirements, &queue, &error);
        int escalateCount = 0;
        for (const auto& task : tasks) {
            if (task.escalate) ++escalateCount;
        }

        json blockers = json::array();
        int readyCount = 0;
        if (!bindOk) {
            blockers.push_back(error);
        } else {
            for (const auto& item : queue) {
                if (item.task.base.queueReady && item.hasCoverage) ++readyCount;
            }
        }
        if (readyCount == 0) blockers.push_back("no_ready_tasks");
        if (escalateCount > 0) blockers.push_back("escalations_present");

        return {
            {"success", true},
            {"ready", blockers.empty()},
            {"blockers", blockers},
            {"readyCount", readyCount},
            {"escalateCount", escalateCount},
            {"queueJson", boundQueueToJson(queue)}
        };
    }

    static bool parseAnnotatedTasksInput(const json& args,
                                         std::vector<AnnotatedTaskitem>* out,
                                         std::string* error) {
        if (!out || !error) return false;
        error->clear();
        out->clear();
        if (!args.contains("tasks")) {
            *error = "tasks_missing";
            return false;
        }
        if (!args["tasks"].is_array()) {
            *error = "tasks_not_array";
            return false;
        }

        for (const auto& taskJson : args["tasks"]) {
            if (!taskJson.is_object()) {
                *error = "task_entry_invalid";
                return false;
            }
            AnnotatedTaskitem task;
            task.base.taskId = taskJson.value("taskId", "");
            task.base.title = taskJson.value("title", "");
            task.base.milestoneId = taskJson.value("milestoneId", "");
            task.base.dependencyTaskIds = taskJson.value("dependencyTaskIds", std::vector<std::string>{});
            task.base.prerequisiteOps = taskJson.value("prerequisiteOps", std::vector<std::string>{});
            task.base.queueReady = taskJson.value("queueReady", false);
            task.confidence = taskJson.value("confidence", 0);
            task.ambiguityCount = taskJson.value("ambiguityCount", 0);
            task.escalate = taskJson.value("escalate", false);
            task.reasons = taskJson.value("reasons", std::vector<std::string>{});
            if (task.base.taskId.empty() || task.base.title.empty() || task.base.milestoneId.empty()) {
                *error = "task_entry_missing_fields";
                return false;
            }
            out->push_back(task);
        }

        if (out->empty()) {
            *error = "tasks_empty";
            return false;
        }
        return true;
    }

    static bool parseNormalizedRequirementsOptional(const json& args,
                                                    std::vector<NormalizedRequirement>* out,
                                                    std::string* error) {
        if (!out || !error) return false;
        error->clear();
        out->clear();
        if (!args.contains("normalizedRequirements")) return true;
        if (!args["normalizedRequirements"].is_array()) {
            *error = "normalized_requirements_not_array";
            return false;
        }
        for (const auto& requirementJson : args["normalizedRequirements"]) {
            if (!requirementJson.is_object()) {
                *error = "requirement_entry_invalid";
                return false;
            }
            NormalizedRequirement requirement;
            requirement.requirementId = requirementJson.value("requirementId", "");
            requirement.normalizedText = requirementJson.value("normalizedText", "");
            requirement.anchor = requirementJson.value("anchor", "");
            requirement.sourceLine = requirementJson.value("sourceLine", 0);
            requirement.ambiguous = requirementJson.value("ambiguous", false);
            if (requirement.requirementId.empty() || requirement.normalizedText.empty()) {
                *error = "requirement_entry_missing_fields";
                return false;
            }
            if (!parseRequirementKind(requirementJson.value("kind", ""), &requirement.kind)) {
                *error = "requirement_kind_invalid";
                return false;
            }
            out->push_back(requirement);
        }
        return true;
    }

    static json boundQueueToJson(const std::vector<BoundTaskitem>& queue) {
        json out = json::array();
        for (const auto& item : queue) {
            json checks = json::array();
            for (const auto& check : item.checks) {
                checks.push_back({
                    {"checkId", check.checkId},
                    {"text", check.text},
                    {"testSkeleton", check.testSkeleton}
                });
            }
            out.push_back({
                {"task", {
                    {"taskId", item.task.base.taskId},
                    {"title", item.task.base.title},
                    {"milestoneId", item.task.base.milestoneId},
                    {"queueReady", item.task.base.queueReady},
                    {"escalate", item.task.escalate}
                }},
                {"checks", checks},
                {"hasCoverage", item.hasCoverage}
            });
        }
        return out;
    }
