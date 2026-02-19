// Step 669-670: MCP wiring for modeling tools
//
// Registers:
//   whetstone_generate_project        (Step 639 — ProjectSkeletonGenerator)
//   whetstone_generate_inference_job  (Step 660 — InferenceJobGenerator)
//
// This file is #included inside the MCPServer class body.

#include "../ProjectSkeletonGenerator.h"
#include "../InferenceJobGenerator.h"

    void registerModelingTools() {
        // ---------------------------------------------------------------
        //  whetstone_generate_project
        // ---------------------------------------------------------------
        tools_.push_back({"whetstone_generate_project",
            "Generate a C++ project skeleton from a name, description, and dependencies. "
            "Returns CMakeLists.txt content, main.cpp stub, and module header paths.",
            {{"type", "object"}, {"properties", {
                {"name", {{"type", "string"},
                    {"description", "Project name."}}},
                {"description", {{"type", "string"},
                    {"description", "Short project description."}}},
                {"dependencies", {{"type", "array"},
                    {"items", {{"type", "string"}}}}}
            }}, {"required", json::array({"name", "description"})}}
        });
        toolHandlers_["whetstone_generate_project"] =
            [this](const json& args) {
                return runGenerateProject(args);
            };

        // ---------------------------------------------------------------
        //  whetstone_generate_inference_job
        // ---------------------------------------------------------------
        tools_.push_back({"whetstone_generate_inference_job",
            "Generate a HiveMind inference job payload from an entropy observation. "
            "Returns a structured job suitable for dispatch to the nexus.",
            {{"type", "object"}, {"properties", {
                {"goal", {{"type", "string"}}},
                {"entropy_score", {{"type", "integer"}}},
                {"files", {{"type", "array"}, {"items", {{"type", "string"}}}}},
                {"bounty", {{"type", "string"}}}
            }}, {"required", json::array({"goal", "entropy_score", "files"})}}
        });
        toolHandlers_["whetstone_generate_inference_job"] =
            [this](const json& args) {
                return runGenerateInferenceJob(args);
            };
    }

    json runGenerateProject(const json& args) {
        if (!args.contains("name") || !args["name"].is_string() || args["name"].get<std::string>().empty()) {
            return {{"success", false}, {"errors", json::array({"name_required"})}};
        }
        if (!args.contains("description") || !args["description"].is_string() ||
            args["description"].get<std::string>().empty()) {
            return {{"success", false}, {"errors", json::array({"description_required"})}};
        }

        std::vector<std::string> deps;
        if (args.contains("dependencies")) {
            if (!args["dependencies"].is_array()) {
                return {{"success", false}, {"errors", json::array({"dependencies_must_be_array"})}};
            }
            for (const auto& dep : args["dependencies"]) {
                if (!dep.is_string()) {
                    return {{"success", false}, {"errors", json::array({"dependency_not_string"})}};
                }
                deps.push_back(dep.get<std::string>());
            }
        }

        auto out = ProjectSkeletonGenerator::generate(
            args["name"].get<std::string>(),
            args["description"].get<std::string>(),
            deps
        );
        return ProjectSkeletonGenerator::asToolResponse(out);
    }

    json runGenerateInferenceJob(const json& args) {
        if (!args.contains("goal") || !args["goal"].is_string()) {
            return {{"success", false}, {"error", "goal_required"}};
        }
        if (!args.contains("entropy_score") || !args["entropy_score"].is_number_integer()) {
            return {{"success", false}, {"error", "entropy_score_required"}};
        }
        if (!args.contains("files") || !args["files"].is_array()) {
            return {{"success", false}, {"error", "files_required"}};
        }

        std::vector<std::string> files;
        for (const auto& f : args["files"]) {
            if (!f.is_string()) {
                return {{"success", false}, {"error", "files_must_be_strings"}};
            }
            files.push_back(f.get<std::string>());
        }

        const std::string bounty = args.value("bounty", "normal");
        return InferenceJobGenerator::generate(
            args["goal"].get<std::string>(),
            args["entropy_score"].get<int>(),
            files,
            bounty
        );
    }
