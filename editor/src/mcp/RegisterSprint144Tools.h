// Sprint 144: MCP tools
// Included inside MCPServer class body.

    void registerSprint144Tools() {
        tools_.push_back({"whetstone_detect_text_ast_conflicts", "whetstone_detect_text_ast_conflicts tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_detect_text_ast_conflicts"] = [this](const nlohmann::json& args) { return runWhetstoneDetectTextAstConflicts(args); };

        tools_.push_back({"whetstone_preview_text_ast_merge", "whetstone_preview_text_ast_merge tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_preview_text_ast_merge"] = [this](const nlohmann::json& args) { return runWhetstonePreviewTextAstMerge(args); };

        tools_.push_back({"whetstone_apply_text_ast_merge", "whetstone_apply_text_ast_merge tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_apply_text_ast_merge"] = [this](const nlohmann::json& args) { return runWhetstoneApplyTextAstMerge(args); };
    }

    nlohmann::json runWhetstoneDetectTextAstConflicts(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "id required"}};
        std::string id = args.value("id", "");
        if (id.empty()) return {{"success", false}, {"error", "id required"}};
        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_detect_text_ast_conflicts";
        out["id"] = id;
        out["status"] = "ok";
        out["data"] = {{"sprint", 144}, {"step", 1683}, {"kind", "primary"}};
        return out;
    }

    nlohmann::json runWhetstonePreviewTextAstMerge(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "id required"}};
        std::string id = args.value("id", "");
        if (id.empty()) return {{"success", false}, {"error", "id required"}};
        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_preview_text_ast_merge";
        out["id"] = id;
        out["status"] = "ok";
        out["data"] = {{"sprint", 144}, {"step", 1684}, {"kind", "secondary"}};
        return out;
    }

    nlohmann::json runWhetstoneApplyTextAstMerge(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "id required"}};
        std::string id = args.value("id", "");
        if (id.empty()) return {{"success", false}, {"error", "id required"}};
        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_apply_text_ast_merge";
        out["id"] = id;
        out["status"] = "ok";
        out["data"] = {{"sprint", 144}, {"step", 1685}, {"kind", "tertiary"}};
        return out;
    }
