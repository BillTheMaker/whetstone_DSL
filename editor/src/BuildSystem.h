#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include <sstream>

struct BuildCommand {
    std::string label;
    std::string command;
};

struct BuildError {
    std::string file;
    int line = 0;
    int col = 0;
    std::string message;
};

class BuildSystem {
public:
    enum class Type {
        None,
        CMake,
        Python,
        Cargo,
        Npm,
        Make,
        Go
    };

    static Type detect(const std::string& root) {
        std::filesystem::path p(root);
        if (std::filesystem::exists(p / "CMakeLists.txt")) return Type::CMake;
        if (std::filesystem::exists(p / "setup.py")) return Type::Python;
        if (std::filesystem::exists(p / "Cargo.toml")) return Type::Cargo;
        if (std::filesystem::exists(p / "package.json")) return Type::Npm;
        if (std::filesystem::exists(p / "Makefile")) return Type::Make;
        if (std::filesystem::exists(p / "go.mod")) return Type::Go;
        return Type::None;
    }

    static std::vector<BuildCommand> commandsFor(Type type) {
        switch (type) {
            case Type::CMake:
                return {{"Build", "cmake --build ."}};
            case Type::Python:
                return {{"Build", "python setup.py build"}};
            case Type::Cargo:
                return {{"Build", "cargo build"}};
            case Type::Npm:
                return {{"Build", "npm run build"}};
            case Type::Make:
                return {{"Build", "make"}};
            case Type::Go:
                return {{"Build", "go build ./..."}};
            default:
                return {};
        }
    }

    static const char* typeName(Type type) {
        switch (type) {
            case Type::CMake:  return "CMake";
            case Type::Python: return "Python";
            case Type::Cargo:  return "Cargo";
            case Type::Npm:    return "NPM";
            case Type::Make:   return "Make";
            case Type::Go:     return "Go";
            default:           return "None";
        }
    }

    static std::vector<BuildError> parseErrors(const std::string& output) {
        std::vector<BuildError> errors;
        std::istringstream iss(output);
        std::string line;
        while (std::getline(iss, line)) {
            BuildError err;
            if (parseGccClang(line, err) || parseMsvc(line, err)) {
                errors.push_back(err);
            }
        }
        return errors;
    }

private:
    static bool parseGccClang(const std::string& line, BuildError& out) {
        size_t p1 = line.find(':');
        size_t p2 = p1 == std::string::npos ? std::string::npos : line.find(':', p1 + 1);
        size_t p3 = p2 == std::string::npos ? std::string::npos : line.find(':', p2 + 1);
        if (p1 == std::string::npos || p2 == std::string::npos || p3 == std::string::npos) return false;
        std::string file = line.substr(0, p1);
        std::string linePart = line.substr(p1 + 1, p2 - p1 - 1);
        std::string colPart = line.substr(p2 + 1, p3 - p2 - 1);
        if (!isNumber(linePart) || !isNumber(colPart)) return false;
        int lineNum = std::atoi(linePart.c_str());
        int colNum = std::atoi(colPart.c_str());
        std::string rest = line.substr(p3 + 1);
        if (rest.find("error") == std::string::npos) return false;
        out.file = file;
        out.line = lineNum;
        out.col = colNum;
        out.message = rest;
        return true;
    }

    static bool parseMsvc(const std::string& line, BuildError& out) {
        size_t p1 = line.find('(');
        size_t p2 = line.find(')', p1 + 1);
        size_t p3 = line.find("error", p2 == std::string::npos ? 0 : p2);
        if (p1 == std::string::npos || p2 == std::string::npos || p3 == std::string::npos) return false;
        std::string file = line.substr(0, p1);
        std::string linePart = line.substr(p1 + 1, p2 - p1 - 1);
        size_t comma = linePart.find(',');
        if (comma != std::string::npos) linePart = linePart.substr(0, comma);
        int lineNum = std::atoi(linePart.c_str());
        out.file = file;
        out.line = lineNum;
        out.col = 0;
        out.message = line.substr(p3);
        return true;
    }

    static bool isNumber(const std::string& s) {
        if (s.empty()) return false;
        for (char c : s) {
            if (c < '0' || c > '9') return false;
        }
        return true;
    }
};
