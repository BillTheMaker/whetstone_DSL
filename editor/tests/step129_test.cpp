// Step 129 TDD Test: Dependency management UI writeback
#include "DependencyPanel.h"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <nlohmann/json.hpp>

static void expect(bool cond, const std::string& name, int& passed, int& failed) {
    if (cond) {
        std::cout << "Test " << (passed + failed + 1) << " PASS: " << name << "\n";
        ++passed;
    } else {
        std::cout << "Test " << (passed + failed + 1) << " FAIL: " << name << "\n";
        ++failed;
    }
}

static std::string readFile(const std::filesystem::path& path) {
    std::ifstream in(path);
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

int main() {
    int passed = 0;
    int failed = 0;

    std::filesystem::path dir = std::filesystem::current_path() / "dep_panel_tmp";
    std::filesystem::remove_all(dir);
    std::filesystem::create_directories(dir);

    // requirements.txt writeback
    {
        std::ofstream out(dir / "requirements.txt");
        out << "numpy==1.26.0\n";
        out.close();

        DependencyPanelState panel;
        std::string log;
        refreshDependencies(panel, dir.string(), log);
        expect(panel.deps.size() == 1, "requirements parsed", passed, failed);

        DependencySpec dep;
        dep.name = "requests";
        dep.version = "2.0.0";
        dep.source = "requirements.txt";
        panel.deps.push_back(dep);
        bool ok = writeDependenciesForSource("requirements.txt", dir.string(), panel.deps, log);
        expect(ok, "requirements writeback ok", passed, failed);
        std::string content = readFile(dir / "requirements.txt");
        expect(content.find("numpy==1.26.0") != std::string::npos, "requirements keep numpy", passed, failed);
        expect(content.find("requests==2.0.0") != std::string::npos, "requirements add requests", passed, failed);
    }

    // package.json writeback with devDependencies
    {
        std::ofstream out(dir / "package.json");
        out << "{ \"dependencies\": { \"react\": \"^18.2.0\" }, "
               "\"devDependencies\": { \"vite\": \"^5.0.0\" } }";
        out.close();

        DependencyPanelState panel;
        std::string log;
        refreshDependencies(panel, dir.string(), log);

        for (auto it = panel.deps.begin(); it != panel.deps.end(); ) {
            if (it->name == "react") {
                it->version = "^19.0.0";
                ++it;
            } else if (it->name == "vite") {
                it = panel.deps.erase(it);
            } else {
                ++it;
            }
        }
        bool ok = writeDependenciesForSource("package.json", dir.string(), panel.deps, log);
        expect(ok, "package.json writeback ok", passed, failed);

        nlohmann::json j;
        {
            std::ifstream in(dir / "package.json");
            in >> j;
        }
        expect(j["dependencies"]["react"] == "^19.0.0", "package.json updates dependency", passed, failed);
        bool hasVite = j.contains("devDependencies") && j["devDependencies"].contains("vite");
        expect(!hasVite, "package.json removes devDependency", passed, failed);
    }

    // edge case: unsupported writeback
    {
        std::string log;
        std::vector<DependencySpec> deps;
        deps.push_back({"Boost", "", "CMakeLists.txt"});
        bool ok = writeDependenciesForSource("CMakeLists.txt", dir.string(), deps, log);
        expect(!ok, "unsupported writeback returns false", passed, failed);
    }

    std::filesystem::remove_all(dir);

    std::cout << "\n=== Step 129 Results: " << passed << " passed, " << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
