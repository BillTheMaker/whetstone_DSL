// Step 644: .whetstone.json for self-hosting editor project (12 tests)

#include "SelfHostingProjectConfig.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; }

static SelfHostingProjectConfig sample() {
    return {
        "whetstone_DSL",
        "/workspace/whetstone_DSL",
        {"editor/src", "editor/tests"},
        {"step644_test", "step645_test"},
        true
    };
}

void test_to_json_emits_project_name() {
    TEST(to_json_emits_project_name);
    auto j = SelfHostingProjectConfigModel::toJson(sample());
    CHECK(j.value("projectName", "") == "whetstone_DSL", "project name mismatch");
    PASS();
}

void test_to_json_emits_workspace_root() {
    TEST(to_json_emits_workspace_root);
    auto j = SelfHostingProjectConfigModel::toJson(sample());
    CHECK(j.value("workspaceRoot", "") == "/workspace/whetstone_DSL", "workspace mismatch");
    PASS();
}

void test_to_json_emits_source_roots() {
    TEST(to_json_emits_source_roots);
    auto j = SelfHostingProjectConfigModel::toJson(sample());
    CHECK(j["sourceRoots"].size() == 2, "source root count mismatch");
    PASS();
}

void test_to_json_emits_test_targets() {
    TEST(to_json_emits_test_targets);
    auto j = SelfHostingProjectConfigModel::toJson(sample());
    CHECK(j["testTargets"].size() == 2, "test target count mismatch");
    PASS();
}

void test_from_json_rejects_non_object() {
    TEST(from_json_rejects_non_object);
    SelfHostingProjectConfig out;
    std::string err;
    CHECK(!SelfHostingProjectConfigModel::fromJson(json::array(), &out, &err), "array should fail");
    PASS();
}

void test_from_json_rejects_missing_project_name() {
    TEST(from_json_rejects_missing_project_name);
    SelfHostingProjectConfig out;
    std::string err;
    CHECK(!SelfHostingProjectConfigModel::fromJson({{"workspaceRoot", "/ws"}}, &out, &err), "missing name should fail");
    PASS();
}

void test_from_json_parses_valid_config() {
    TEST(from_json_parses_valid_config);
    SelfHostingProjectConfig out;
    std::string err;
    CHECK(SelfHostingProjectConfigModel::fromJson(SelfHostingProjectConfigModel::toJson(sample()), &out, &err),
          "valid config should parse");
    CHECK(out.projectName == "whetstone_DSL", "parsed name mismatch");
    PASS();
}

void test_from_json_parses_self_hosting_flag() {
    TEST(from_json_parses_self_hosting_flag);
    SelfHostingProjectConfig out;
    std::string err;
    CHECK(SelfHostingProjectConfigModel::fromJson(SelfHostingProjectConfigModel::toJson(sample()), &out, &err),
          "valid config should parse");
    CHECK(out.selfHosting, "selfHosting flag mismatch");
    PASS();
}

void test_can_self_host_true_for_valid_self_hosting_config() {
    TEST(can_self_host_true_for_valid_self_hosting_config);
    CHECK(SelfHostingProjectConfigModel::canSelfHost(sample()), "should self-host");
    PASS();
}

void test_can_self_host_false_when_flag_disabled() {
    TEST(can_self_host_false_when_flag_disabled);
    auto cfg = sample();
    cfg.selfHosting = false;
    CHECK(!SelfHostingProjectConfigModel::canSelfHost(cfg), "disabled flag should fail");
    PASS();
}

void test_can_self_host_false_when_workspace_missing() {
    TEST(can_self_host_false_when_workspace_missing);
    auto cfg = sample();
    cfg.workspaceRoot.clear();
    CHECK(!SelfHostingProjectConfigModel::canSelfHost(cfg), "missing workspace should fail");
    PASS();
}

void test_can_self_host_false_when_source_roots_empty() {
    TEST(can_self_host_false_when_source_roots_empty);
    auto cfg = sample();
    cfg.sourceRoots.clear();
    CHECK(!SelfHostingProjectConfigModel::canSelfHost(cfg), "missing sources should fail");
    PASS();
}

int main() {
    std::cout << "Step 644: .whetstone.json self-hosting config\n";

    test_to_json_emits_project_name();
    test_to_json_emits_workspace_root();
    test_to_json_emits_source_roots();
    test_to_json_emits_test_targets();
    test_from_json_rejects_non_object();
    test_from_json_rejects_missing_project_name();
    test_from_json_parses_valid_config();
    test_from_json_parses_self_hosting_flag();
    test_can_self_host_true_for_valid_self_hosting_config();
    test_can_self_host_false_when_flag_disabled();
    test_can_self_host_false_when_workspace_missing();
    test_can_self_host_false_when_source_roots_empty();

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
