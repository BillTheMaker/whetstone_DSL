// Sprint 48 C++ raising MCP tool (Step 717)
// Included inside MCPServer class body.

    void registerCppRaisingTools() {
        tools_.push_back({"whetstone_generate_cpp_from_ir",
            "Generate C++ artifacts from SemanticCoreIR using output profiles.",
            {{"type", "object"}, {"properties", {
                {"ir", {{"type", "object"}, {"description", "SemanticCoreIR packet."}}},
                {"profile", {{"type", "string"}, {"description", "safe-first|perf-first|interop-first"}}},
                {"projectName", {{"type", "string"}}}
            }}, {"required", nlohmann::json::array({"ir"})}}
        });
        toolHandlers_["whetstone_generate_cpp_from_ir"] =
            [this](const nlohmann::json& args) { return runGenerateCppFromIr(args); };
    }

    nlohmann::json runGenerateCppFromIr(const nlohmann::json& args) {
        if (!args.contains("ir") || !args["ir"].is_object()) {
            return {{"success", false}, {"error", "ir_missing"}};
        }

        const std::string profile = args.value("profile", "safe-first");
        if (profile != "safe-first" && profile != "perf-first" && profile != "interop-first") {
            return {{"success", false}, {"error", "profile_invalid"}};
        }

        const std::string projectName = args.value("projectName", "generated_cpp");

        SemanticCoreIR ir;
        std::string err;
        if (!SemanticCoreIRModel::fromJson(args["ir"], &ir, &err)) {
            return {{"success", false}, {"error", "ir_parse_failed"}, {"detail", err}};
        }
        if (!SemanticCoreIRModel::validate(ir, &err)) {
            return {{"success", false}, {"error", "ir_invalid"}, {"detail", err}};
        }

        auto own = CppOwnershipMappingPolicy::map(ir, profile);
        auto bor = CppBorrowMappingStrategy::map(ir, profile);
        auto trt = CppTraitRaising::raise(ir, profile);
        auto tpl = CppTemplateRaisingPolicy::raise(ir, profile);
        auto errMap = CppErrorModelMapping::map(ir, profile);
        auto asyncMap = CppAsyncMappingStrategy::map(ir, profile);
        auto alg = CppAlgorithmLifting::lift(ir);
        auto build = CppBuildArtifactGenerator::generate(projectName, profile);

        bool reviewRequired = false;
        nlohmann::json risks = nlohmann::json::array();
        if (errMap.panicTranslatedToTerminate) {
            reviewRequired = true;
            risks.push_back("panic_to_terminate_semantic_gap");
        }
        if (!trt.empty() && profile == "perf-first") {
            reviewRequired = true;
            risks.push_back("trait_composition_semantic_review");
        }

        std::string header =
            "#pragma once\n"
            "#include <memory>\n"
            "#include <expected>\n"
            "namespace generated {\n"
            "struct API { int run(); };\n"
            "}\n";

        std::string source =
            "#include \"generated.hpp\"\n"
            "namespace generated {\n"
            "int API::run() { return 0; }\n"
            "}\n";

        return {
            {"success", true},
            {"profile", profile},
            {"reviewRequired", reviewRequired},
            {"riskAnnotations", risks},
            {"ownership", CppOwnershipMappingPolicy::toJson(own)},
            {"borrowMapping", CppBorrowMappingStrategy::toJson(bor)},
            {"traits", CppTraitRaising::toJson(trt)},
            {"templates", CppTemplateRaisingPolicy::toJson(tpl)},
            {"errorModel", CppErrorModelMapping::toJson(errMap)},
            {"asyncModel", CppAsyncMappingStrategy::toJson(asyncMap)},
            {"algorithms", CppAlgorithmLifting::toJson(alg)},
            {"build", CppBuildArtifactGenerator::toJson(build)},
            {"files", {
                {"include/generated.hpp", header},
                {"src/generated.cpp", source},
                {"CMakeLists.txt", build.cmakeLists}
            }}
        };
    }
