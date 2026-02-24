// Sprint 98: MCP tools
// Included inside MCPServer class body.

    void registerSprint98Tools() {
        tools_.push_back({"whetstone_attach_multimodal_evidence", "whetstone_attach_multimodal_evidence tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_attach_multimodal_evidence"] = [this](const nlohmann::json& args) { return runWhetstoneAttachMultimodalEvidence(args); };

        tools_.push_back({"whetstone_verify_architecture_consistency", "whetstone_verify_architecture_consistency tool.", nlohmann::json::object()});
        toolHandlers_["whetstone_verify_architecture_consistency"] = [this](const nlohmann::json& args) { return runWhetstoneVerifyArchitectureConsistency(args); };

    }

    nlohmann::json runWhetstoneAttachMultimodalEvidence(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "id required"}};
        std::string id = args.value("id", "");
        if (id.empty()) return {{"success", false}, {"error", "id required"}};
        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_attach_multimodal_evidence";
        out["id"] = id;
        out["status"] = "ok";
        nlohmann::json data = nlohmann::json::object();
        data["sprint"] = 98;
        data["step"] = 1215;
        data["kind"] = "primary";
        out["data"] = data;
        return out;
    }

    nlohmann::json runWhetstoneVerifyArchitectureConsistency(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "id required"}};
        std::string id = args.value("id", "");
        if (id.empty()) return {{"success", false}, {"error", "id required"}};
        nlohmann::json out = nlohmann::json::object();
        out["success"] = true;
        out["tool"] = "whetstone_verify_architecture_consistency";
        out["id"] = id;
        out["status"] = "ok";
        nlohmann::json data = nlohmann::json::object();
        data["sprint"] = 98;
        data["step"] = 1216;
        data["kind"] = "secondary";
        out["data"] = data;
        return out;
    }

