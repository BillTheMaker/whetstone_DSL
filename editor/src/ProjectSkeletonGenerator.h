#pragma once
// Step 639: whetstone_generate_project MCP tool model

#include <nlohmann/json.hpp>

#include <string>
#include <vector>

using json = nlohmann::json;

struct GeneratedProjectSkeleton {
    bool success = false;
    std::string name;
    std::string cmakeLists;
    std::string mainCpp;
    std::vector<std::string> moduleHeaders;
    std::vector<std::string> dependencies;
    std::vector<std::string> errors;
};

class ProjectSkeletonGenerator {
public:
    static std::string toolName() { return "whetstone_generate_project"; }

    static GeneratedProjectSkeleton generate(const std::string& name,
                                             const std::string& description,
                                             const std::vector<std::string>& dependencies) {
        GeneratedProjectSkeleton out;
        if (name.empty()) {
            out.errors.push_back("name_required");
            return out;
        }

        out.success = true;
        out.name = name;
        out.dependencies = dependencies;
        out.moduleHeaders = {
            "include/AppCore.h",
            "include/TaskRouter.h"
        };

        out.cmakeLists = "cmake_minimum_required(VERSION 3.24)\n"
                         "project(" + name + " LANGUAGES CXX)\n"
                         "set(CMAKE_CXX_STANDARD 20)\n"
                         "add_executable(" + name + " src/main.cpp)\n";
        for (const auto& dep : dependencies) {
            out.cmakeLists += "find_package(" + dep + " CONFIG REQUIRED)\n";
        }
        out.cmakeLists += "target_include_directories(" + name + " PRIVATE include)\n";

        out.mainCpp = "#include <iostream>\n"
                      "int main(){ std::cout << \"" + description + "\\n\"; return 0; }\n";
        return out;
    }

    static json asToolResponse(const GeneratedProjectSkeleton& out) {
        return {
            {"success", out.success},
            {"name", out.name},
            {"cmakeLists", out.cmakeLists},
            {"mainCpp", out.mainCpp},
            {"moduleHeaders", out.moduleHeaders},
            {"dependencies", out.dependencies},
            {"errors", out.errors}
        };
    }
};
