#pragma once
// Step 645: CMake generator from project AST

#include <string>
#include <vector>

struct ProjectAstDescription {
    std::string projectName;
    std::vector<std::string> sources;
    std::vector<std::string> tests;
    std::vector<std::string> dependencies;
    bool includeInstallRules = true;
};

class ProjectAstCMakeGenerator {
public:
    static std::string generate(const ProjectAstDescription& ast) {
        if (ast.projectName.empty()) return "";
        std::string cmake;
        cmake += "cmake_minimum_required(VERSION 3.24)\n";
        cmake += "project(" + ast.projectName + " LANGUAGES CXX)\n";
        cmake += "set(CMAKE_CXX_STANDARD 20)\n";
        cmake += "find_package(vcpkg CONFIG QUIET)\n";
        for (const auto& dep : ast.dependencies) {
            cmake += "find_package(" + dep + " CONFIG REQUIRED)\n";
        }
        cmake += "add_executable(" + ast.projectName;
        for (const auto& src : ast.sources) cmake += " " + src;
        cmake += ")\n";
        for (const auto& test : ast.tests) {
            cmake += "add_executable(" + test + " tests/" + test + ".cpp)\n";
        }
        if (ast.includeInstallRules) {
            cmake += "install(TARGETS " + ast.projectName + " DESTINATION bin)\n";
        }
        return cmake;
    }
};
