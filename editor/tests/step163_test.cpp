// Step 163 TDD Test: Telemetry opt-in logging
#include "Telemetry.h"
#include <iostream>
#include <filesystem>
#include <fstream>

static void expect(bool cond, const std::string& name, int& passed, int& failed) {
    if (cond) {
        std::cout << "Test " << (passed + failed + 1) << " PASS: " << name << "\n";
        ++passed;
    } else {
        std::cout << "Test " << (passed + failed + 1) << " FAIL: " << name << "\n";
        ++failed;
    }
}

static std::string readFile(const std::string& path) {
    std::ifstream in(path);
    if (!in.is_open()) return "";
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

int main() {
    int passed = 0;
    int failed = 0;

    namespace fs = std::filesystem;
    fs::path root = fs::temp_directory_path() / "whetstone_telemetry_test";
    fs::create_directories(root);

    Telemetry telemetry;
    telemetry.initialize(root.string(), false);
    telemetry.recordEvent("nope", json::object());
    expect(!fs::exists(root / "telemetry.log"), "opt-out no log", passed, failed);

    Telemetry telemetry2;
    telemetry2.initialize(root.string(), true);
    telemetry2.recordEvent("test_event", json::object());
    auto logPath = root / "telemetry.log";
    expect(fs::exists(logPath), "log written", passed, failed);
    std::string content = readFile(logPath.string());
    expect(content.find("test_event") != std::string::npos,
           "event recorded", passed, failed);

    std::cout << "\n=== Step 163 Results: " << passed << " passed, "
              << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
