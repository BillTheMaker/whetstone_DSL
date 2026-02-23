// Sprint 63: Adapter Hints MCP tools (Steps 865-866)
// Included inside MCPServer class body.

    void registerAdapterHintsTools() {
        tools_.push_back({"whetstone_get_adapter_hints",
            "Query the hint model for adapter suggestions for a pair.",
            {{"type", "object"}, {"properties", {
                {"pair_id", {{"type", "string"}}},
                {"features", {{"type", "array"}}}
            }}, {"required", nlohmann::json::array({"pair_id"})}}
        });
        toolHandlers_["whetstone_get_adapter_hints"] =
            [this](const nlohmann::json& args) { return runGetAdapterHints(args); };

        tools_.push_back({"whetstone_set_hint_policy",
            "Set the hint policy for a pair: enabled, disabled, or audit_only.",
            {{"type", "object"}, {"properties", {
                {"pair_id", {{"type", "string"}}},
                {"policy", {{"type", "string"}}},
                {"reason", {{"type", "string"}}}
            }}, {"required", nlohmann::json::array({"pair_id", "policy"})}}
        });
        toolHandlers_["whetstone_set_hint_policy"] =
            [this](const nlohmann::json& args) { return runSetHintPolicy(args); };
    }

    nlohmann::json runGetAdapterHints(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "pair_id required"}};
        std::string pairId = args.value("pair_id", "");
        std::vector<std::string> features;
        if (args.contains("features") && args["features"].is_array()) {
            for (const auto& f : args["features"])
                features.push_back(f.get<std::string>());
        }
        if (pairId.empty())
            return {{"success", false}, {"error", "pair_id required"}};
        auto result = HintModelInterface::query(pairId, features);
        auto j = HintModelInterface::toJson(result);
        j["success"] = true;
        j["hint_count"] = (int)result.hints.size();
        return j;
    }

    nlohmann::json runSetHintPolicy(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "pair_id required"}};
        std::string pairId = args.value("pair_id", "");
        std::string policy = args.value("policy", "");
        std::string reason = args.value("reason", "");
        if (pairId.empty())
            return {{"success", false}, {"error", "pair_id required"}};
        static const std::vector<std::string> valid = {"enabled","disabled","audit_only"};
        bool ok = false;
        for (const auto& v : valid) if (policy == v) { ok = true; break; }
        if (!ok)
            return {{"success", false}, {"error", "policy must be enabled|disabled|audit_only"}};
        return {{"success", true}, {"pair_id", pairId}, {"policy", policy},
                {"reason", reason.empty() ? "not_specified" : reason}};
    }
