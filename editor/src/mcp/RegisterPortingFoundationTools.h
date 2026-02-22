// Sprint 46 foundations: language matrix + porting contract MCP tools (Steps 694-695)
//
// This file is #included inside MCPServer class body.

    void registerPortingFoundationTools() {
        tools_.push_back({"whetstone_get_language_matrix",
            "Return language capability matrix rows with support tier and gate status.",
            {{"type", "object"}, {"properties", {
                {"language", {{"type", "string"},
                    {"description", "Optional single-language filter."}}},
                {"strict", {{"type", "boolean"},
                    {"description", "Validate required capability fields strictly."}}}
            }}}
        });
        toolHandlers_["whetstone_get_language_matrix"] =
            [this](const json& args) { return runGetLanguageMatrix(args); };

        tools_.push_back({"whetstone_get_porting_contract",
            "Return migration contract thresholds and required gates for a source/target pair.",
            {{"type", "object"}, {"properties", {
                {"sourceLanguage", {{"type", "string"}}},
                {"targetLanguage", {{"type", "string"}}},
                {"overrides", {{"type", "object"}}}
            }}, {"required", json::array({"sourceLanguage", "targetLanguage"})}}
        });
        toolHandlers_["whetstone_get_porting_contract"] =
            [this](const json& args) { return runGetPortingContract(args); };
    }

    json runGetLanguageMatrix(const json& args) {
        const std::string filter = args.value("language", "");
        const bool strict = args.value("strict", false);

        auto rows = LanguageCapabilityMatrix::defaultRows();
        json out = json::array();
        for (const auto& r : LanguageCapabilityMatrix::sorted(rows)) {
            if (!filter.empty() && r.language != filter) continue;
            std::string err;
            if (!LanguageCapabilityMatrix::validateRow(r, strict, &err)) {
                return {{"success", false}, {"error", err}, {"language", r.language}};
            }
            SupportTier tier = r.experimental ? SupportTier::Experimental : SupportTier::Beta;
            if (r.language == "cpp" || r.language == "rust") tier = SupportTier::Stable;
            auto gates = LanguageSupportTierModel::requiredGates(tier);
            json row = LanguageCapabilityMatrix::rowToJson(r);
            row["tier"] = LanguageSupportTierModel::tierToString(tier);
            row["gateStatus"] = {
                {"parseAndProject", gates.parseAndProject},
                {"executableEquivalence", gates.executableEquivalence},
                {"staticChecks", gates.staticChecks},
                {"securityGates", gates.securityGates},
                {"performanceGates", gates.performanceGates},
                {"migrationReportQuality", gates.migrationReportQuality}
            };
            out.push_back(row);
        }
        json result = {{"success", true}, {"rows", out}};
        if (!filter.empty() && out.empty()) result["warning"] = "unknown_language";
        return result;
    }

    json runGetPortingContract(const json& args) {
        if (!args.contains("sourceLanguage") || !args["sourceLanguage"].is_string()) {
            return {{"success", false}, {"error", "source_language_missing"}};
        }
        if (!args.contains("targetLanguage") || !args["targetLanguage"].is_string()) {
            return {{"success", false}, {"error", "target_language_missing"}};
        }

        const std::string src = args.value("sourceLanguage", "");
        const std::string tgt = args.value("targetLanguage", "");

        MigrationGateThresholds th;
        bool pairFound = false;
        if (src == "rust" && tgt == "cpp") {
            pairFound = true;
            th.maxPerfRegressionPct = 7.5f;
        } else if (src == "c" && tgt == "rust") {
            pairFound = true;
            th.maxPerfRegressionPct = 12.5f;
        }

        if (args.contains("overrides") && args["overrides"].is_object()) {
            const auto& o = args["overrides"];
            th.minimumTestPassRate = o.value("minimumTestPassRate", th.minimumTestPassRate);
            th.maxHighSeverityFindings = o.value("maxHighSeverityFindings", th.maxHighSeverityFindings);
            th.maxPerfRegressionPct = o.value("maxPerfRegressionPct", th.maxPerfRegressionPct);
            th.requireChecklistComplete = o.value("requireChecklistComplete", th.requireChecklistComplete);
        }

        return {
            {"success", true},
            {"sourceLanguage", src},
            {"targetLanguage", tgt},
            {"pairFound", pairFound},
            {"thresholds", {
                {"minimumTestPassRate", th.minimumTestPassRate},
                {"maxHighSeverityFindings", th.maxHighSeverityFindings},
                {"maxPerfRegressionPct", th.maxPerfRegressionPct},
                {"requireChecklistComplete", th.requireChecklistComplete}
            }},
            {"requiredGates", json::array({
                "build_success",
                "test_pass_rate",
                "high_severity_findings",
                "performance_regression",
                "checklist_complete"
            })}
        };
    }
