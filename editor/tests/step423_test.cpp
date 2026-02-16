// Step 423: Model Profile Registry Tests (12 tests)

#include "ModelProfileRegistry.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static std::string makeTempDir(const std::string& name) {
    auto dir = std::filesystem::temp_directory_path() / ("whetstone_step423_" + name);
    std::filesystem::create_directories(dir);
    return dir.string();
}

void test_default_profiles_exist() {
    TEST(default_profiles_exist);
    ModelProfileRegistry reg;
    CHECK(reg.hasProfile("claude-opus"), "missing claude-opus");
    CHECK(reg.hasProfile("claude-sonnet"), "missing claude-sonnet");
    CHECK(reg.hasProfile("claude-haiku"), "missing claude-haiku");
    CHECK(reg.hasProfile("local-slm"), "missing local-slm");
    PASS();
}

void test_default_llm_mapping_to_sonnet() {
    TEST(default_llm_mapping_to_sonnet);
    ModelProfileRegistry reg;
    CHECK(reg.resolveProfileForWorker("llm") == "claude-sonnet",
          "llm should default to claude-sonnet");
    PASS();
}

void test_default_slm_mapping_to_haiku() {
    TEST(default_slm_mapping_to_haiku);
    ModelProfileRegistry reg;
    CHECK(reg.resolveProfileForWorker("slm") == "claude-haiku",
          "slm should default to claude-haiku");
    PASS();
}

void test_resolve_for_worker_returns_profile() {
    TEST(resolve_for_worker_returns_profile);
    ModelProfileRegistry reg;
    auto p = reg.resolveForWorker("llm");
    CHECK(p.name == "claude-sonnet", "expected sonnet profile");
    CHECK(p.contextWindow >= 100000, "expected large context window");
    PASS();
}

void test_register_custom_profile() {
    TEST(register_custom_profile);
    ModelProfileRegistry reg;
    reg.registerProfile({"my-model", 32000, 0.001, 0.002, "fast", {"coding"}});
    CHECK(reg.hasProfile("my-model"), "custom profile not registered");
    CHECK(reg.getProfile("my-model").contextWindow == 32000, "custom context mismatch");
    PASS();
}

void test_set_worker_mapping_requires_existing_profile() {
    TEST(set_worker_mapping_requires_existing_profile);
    ModelProfileRegistry reg;
    reg.setWorkerProfileMapping("llm", "missing-profile");
    CHECK(reg.resolveProfileForWorker("llm") == "claude-sonnet",
          "should not override with missing profile");
    PASS();
}

void test_set_worker_mapping_overrides_default() {
    TEST(set_worker_mapping_overrides_default);
    ModelProfileRegistry reg;
    reg.setWorkerProfileMapping("llm", "claude-opus");
    CHECK(reg.resolveProfileForWorker("llm") == "claude-opus",
          "llm override should map to opus");
    PASS();
}

void test_apply_overrides_from_json_profiles() {
    TEST(apply_overrides_from_json_profiles);
    ModelProfileRegistry reg;
    json cfg = {
        {"profiles", json::array({
            {{"name", "claude-sonnet"}, {"contextWindow", 120000}, {"speedClass", "medium-fast"}},
            {{"name", "custom-x"}, {"contextWindow", 64000}, {"costPer1kInput", 0.002}}
        })}
    };
    reg.applyOverrides(cfg);
    CHECK(reg.getProfile("claude-sonnet").contextWindow == 120000, "sonnet override failed");
    CHECK(reg.hasProfile("custom-x"), "custom profile from json missing");
    PASS();
}

void test_apply_overrides_worker_mapping() {
    TEST(apply_overrides_worker_mapping);
    ModelProfileRegistry reg;
    json cfg = {{"workerMapping", {{"llm", "claude-opus"}, {"slm", "local-slm"}}}};
    reg.applyOverrides(cfg);
    CHECK(reg.resolveProfileForWorker("llm") == "claude-opus", "llm mapping override failed");
    CHECK(reg.resolveProfileForWorker("slm") == "local-slm", "slm mapping override failed");
    PASS();
}

void test_load_overrides_from_file() {
    TEST(load_overrides_from_file);
    ModelProfileRegistry reg;
    auto dir = std::filesystem::path(makeTempDir("cfg_file"));
    auto path = dir / "config.json";
    std::ofstream out(path);
    out << R"({
  "profiles": [{"name":"claude-haiku","contextWindow":16000}],
  "workerMapping": {"slm":"claude-haiku"}
})";
    out.close();

    std::string err;
    CHECK(reg.loadOverridesFromFile(path.string(), err), "load override failed");
    CHECK(reg.getProfile("claude-haiku").contextWindow == 16000, "haiku override missing");
    CHECK(reg.resolveProfileForWorker("slm") == "claude-haiku", "mapping not applied");
    PASS();
}

void test_load_overrides_invalid_json_fails() {
    TEST(load_overrides_invalid_json_fails);
    ModelProfileRegistry reg;
    auto dir = std::filesystem::path(makeTempDir("bad_cfg"));
    auto path = dir / "config.json";
    std::ofstream(path) << "{ bad json";
    std::string err;
    CHECK(!reg.loadOverridesFromFile(path.string(), err), "invalid json should fail");
    CHECK(!err.empty(), "error message should be present");
    PASS();
}

void test_registry_json_export_contains_mapping() {
    TEST(registry_json_export_contains_mapping);
    ModelProfileRegistry reg;
    auto j = reg.toJson();
    CHECK(j.contains("profiles"), "missing profiles");
    CHECK(j.contains("workerMapping"), "missing workerMapping");
    CHECK(j["workerMapping"].value("llm", "") == "claude-sonnet", "llm mapping missing");
    PASS();
}

int main() {
    std::cout << "Step 423: Model Profile Registry Tests\n";

    test_default_profiles_exist();                    // 1
    test_default_llm_mapping_to_sonnet();            // 2
    test_default_slm_mapping_to_haiku();             // 3
    test_resolve_for_worker_returns_profile();       // 4
    test_register_custom_profile();                  // 5
    test_set_worker_mapping_requires_existing_profile(); // 6
    test_set_worker_mapping_overrides_default();     // 7
    test_apply_overrides_from_json_profiles();       // 8
    test_apply_overrides_worker_mapping();           // 9
    test_load_overrides_from_file();                 // 10
    test_load_overrides_invalid_json_fails();        // 11
    test_registry_json_export_contains_mapping();    // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
