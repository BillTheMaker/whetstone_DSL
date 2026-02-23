// Sprint 66: Runtime Pack MCP tools (Steps 895-896)
// Included inside MCPServer class body.

    void registerRuntimePackTools() {
        tools_.push_back({"whetstone_get_runtime_assumptions",
            "Extract runtime assumption hints from source patterns.",
            {{"type", "object"}, {"properties", {
                {"runtime_id", {{"type", "string"}}},
                {"patterns", {{"type", "array"}, {"items", {{"type", "string"}}}}}
            }}, {"required", nlohmann::json::array({"runtime_id"})}}
        });
        toolHandlers_["whetstone_get_runtime_assumptions"] =
            [this](const nlohmann::json& args) { return runGetRuntimeAssumptions(args); };

        tools_.push_back({"whetstone_set_runtime_profile",
            "Set the active runtime profile for a language pair.",
            {{"type", "object"}, {"properties", {
                {"pair_id", {{"type", "string"}}},
                {"runtime_id", {{"type", "string"}}},
                {"version", {{"type", "string"}}}
            }}, {"required", nlohmann::json::array({"pair_id", "runtime_id"})}}
        });
        toolHandlers_["whetstone_set_runtime_profile"] =
            [this](const nlohmann::json& args) { return runSetRuntimeProfile(args); };
    }

    nlohmann::json runGetRuntimeAssumptions(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "runtime_id required"}};
        std::string runtimeId = args.value("runtime_id", "");
        if (runtimeId.empty()) return {{"success", false}, {"error", "runtime_id required"}};
        std::vector<std::string> patterns;
        if (args.contains("patterns") && args["patterns"].is_array()) {
            for (const auto& p : args["patterns"]) {
                if (p.is_string()) patterns.push_back(p.get<std::string>());
            }
        }
        auto hints = RuntimeAssumptionExtractor::extract(runtimeId, patterns);
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& h : hints) {
            arr.push_back({{"hint_id", h.hintId}, {"runtime_id", h.runtimeId},
                           {"category", h.category}, {"evidence", h.evidence},
                           {"confidence", h.confidence}});
        }
        return {{"success", true}, {"runtime_id", runtimeId},
                {"hints", arr}, {"count", (int)hints.size()}};
    }

    nlohmann::json runSetRuntimeProfile(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "pair_id required"}};
        std::string pairId = args.value("pair_id", "");
        std::string runtimeId = args.value("runtime_id", "");
        if (pairId.empty()) return {{"success", false}, {"error", "pair_id required"}};
        if (runtimeId.empty()) return {{"success", false}, {"error", "runtime_id required"}};
        std::string version = args.value("version", "");
        RuntimeProfile profile{pairId, runtimeId, version};
        runtimeProfileStore_.set(profile);
        return {{"success", true}, {"pair_id", pairId},
                {"runtime_id", runtimeId}, {"version", version}};
    }
