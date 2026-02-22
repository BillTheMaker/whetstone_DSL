#pragma once
// Step 716: build artifact generator.

#include <string>
#include <vector>

#include <nlohmann/json.hpp>

struct CppBuildArtifacts {
    std::string cmakeLists;
    std::vector<std::string> headers;
    std::vector<std::string> sources;
};

class CppBuildArtifactGenerator {
public:
    static CppBuildArtifacts generate(const std::string& projectName,
                                      const std::string& profile = "safe-first") {
        CppBuildArtifacts a;
        const std::string target = projectName.empty() ? "generated_cpp" : projectName;
        a.cmakeLists =
            "cmake_minimum_required(VERSION 3.20)\n"
            "project(" + target + " LANGUAGES CXX)\n"
            "set(CMAKE_CXX_STANDARD 20)\n"
            "add_library(" + target + " STATIC src/generated.cpp)\n"
            "target_include_directories(" + target + " PUBLIC include)\n";
        if (profile == "safe-first") {
            a.cmakeLists += "target_compile_definitions(" + target + " PUBLIC CPP_SAFE_FIRST=1)\n";
        }
        a.headers = {"include/generated.hpp"};
        a.sources = {"src/generated.cpp"};
        return a;
    }

    static nlohmann::json toJson(const CppBuildArtifacts& a) {
        return {{"cmakeLists", a.cmakeLists}, {"headers", a.headers}, {"sources", a.sources}};
    }
};
