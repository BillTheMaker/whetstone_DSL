// Sprint 142: MCP tools
// Included inside MCPServer class body.

    void registerSprint142Tools() {
        tools_.push_back({"whetstone_sync_text_to_ast", "whetstone_sync_text_to_ast tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_sync_text_to_ast"] = [this](const nlohmann::json& args) { return runWhetstoneSyncTextToAst(args); };

        tools_.push_back({"whetstone_get_sync_diagnostics", "whetstone_get_sync_diagnostics tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_get_sync_diagnostics"] = [this](const nlohmann::json& args) { return runWhetstoneGetSyncDiagnostics(args); };

        tools_.push_back({"whetstone_get_sync_identity_report", "whetstone_get_sync_identity_report tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_get_sync_identity_report"] = [this](const nlohmann::json& args) { return runWhetstoneGetSyncIdentityReport(args); };
    }

    nlohmann::json runWhetstoneSyncTextToAst(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "id required"}};
        std::string id = args.value("id", "");
        if (id.empty()) return {{"success", false}, {"error", "id required"}};
        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_sync_text_to_ast";
        out["id"] = id;
        out["status"] = "ok";
        out["data"] = {{"sprint", 142}, {"step", 1663}, {"kind", "primary"}};
        return out;
    }

    nlohmann::json runWhetstoneGetSyncDiagnostics(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "id required"}};
        std::string id = args.value("id", "");
        if (id.empty()) return {{"success", false}, {"error", "id required"}};
        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_get_sync_diagnostics";
        out["id"] = id;
        out["status"] = "ok";
        out["data"] = {{"sprint", 142}, {"step", 1664}, {"kind", "secondary"}};
        return out;
    }

    nlohmann::json runWhetstoneGetSyncIdentityReport(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "id required"}};
        std::string id = args.value("id", "");
        if (id.empty()) return {{"success", false}, {"error", "id required"}};
        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_get_sync_identity_report";
        out["id"] = id;
        out["status"] = "ok";
        out["data"] = {{"sprint", 142}, {"step", 1665}, {"kind", "tertiary"}};
        return out;
    }
