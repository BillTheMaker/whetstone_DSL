// Step 638: Phase 38a integration (8 tests)

#include "Phase38aIntegration.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; }

void test_schema_generation_is_successful() {
    TEST(schema_generation_is_successful);
    auto result = Phase38aIntegration::run();
    CHECK(result.schemaGenerated, "schema generation should succeed");
    PASS();
}

void test_capability_generation_is_successful() {
    TEST(capability_generation_is_successful);
    auto result = Phase38aIntegration::run();
    CHECK(result.capabilityGenerated, "capability generation should succeed");
    PASS();
}

void test_error_hierarchy_generation_is_successful() {
    TEST(error_hierarchy_generation_is_successful);
    auto result = Phase38aIntegration::run();
    CHECK(result.errorsGenerated, "error synthesis should succeed");
    PASS();
}

void test_platform_guard_generation_is_successful() {
    TEST(platform_guard_generation_is_successful);
    auto result = Phase38aIntegration::run();
    CHECK(result.platformGuardsGenerated, "platform guards should be generated");
    PASS();
}

void test_main_entry_is_generated() {
    TEST(main_entry_is_generated);
    auto result = Phase38aIntegration::run();
    CHECK(result.mainEntryGenerated, "main entry should be generated");
    PASS();
}

void test_main_entry_includes_shared_headers() {
    TEST(main_entry_includes_shared_headers);
    auto result = Phase38aIntegration::run();
    CHECK(result.includesSharedHeaders, "shared headers should be included");
    PASS();
}

void test_cmake_interface_target_is_emitted() {
    TEST(cmake_interface_target_is_emitted);
    auto result = Phase38aIntegration::run();
    CHECK(result.cmakeHasInterfaceTarget, "interface target should be emitted");
    PASS();
}

void test_host_skeleton_marked_buildable() {
    TEST(host_skeleton_marked_buildable);
    auto result = Phase38aIntegration::run();
    CHECK(result.buildableHostSkeleton, "host skeleton should be buildable");
    PASS();
}

int main() {
    std::cout << "Step 638: Phase 38a integration\n";

    test_schema_generation_is_successful();
    test_capability_generation_is_successful();
    test_error_hierarchy_generation_is_successful();
    test_platform_guard_generation_is_successful();
    test_main_entry_is_generated();
    test_main_entry_includes_shared_headers();
    test_cmake_interface_target_is_emitted();
    test_host_skeleton_marked_buildable();

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
