// Step 637: Cross-platform CMake target generator (12 tests)

#include "PlatformCMakeTargetGenerator.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; }

void test_supported_annotation_rejects_empty() {
    TEST(supported_annotation_rejects_empty);
    CHECK(!PlatformCMakeTargetGenerator::isSupportedAnnotation(""), "empty annotation should be rejected");
    PASS();
}

void test_supported_annotation_accepts_platform_target() {
    TEST(supported_annotation_accepts_platform_target);
    CHECK(PlatformCMakeTargetGenerator::isSupportedAnnotation("aarch64-apple-darwin"), "annotation should be accepted");
    PASS();
}

void test_generate_cmake_guard_requires_target_name() {
    TEST(generate_cmake_guard_requires_target_name);
    PlatformBlockInput in{"", "aarch64", "src/main.cpp", "aarch64-apple-darwin"};
    CHECK(PlatformCMakeTargetGenerator::generateCMakeGuard(in).empty(), "missing target should fail");
    PASS();
}

void test_generate_cmake_guard_includes_if_block() {
    TEST(generate_cmake_guard_includes_if_block);
    PlatformBlockInput in{"drone", "aarch64", "src/main.cpp", "aarch64-apple-darwin"};
    auto txt = PlatformCMakeTargetGenerator::generateCMakeGuard(in);
    CHECK(txt.find("if(CMAKE_SYSTEM_PROCESSOR MATCHES \"aarch64\")") != std::string::npos, "if guard missing");
    PASS();
}

void test_generate_cmake_guard_emits_target_sources() {
    TEST(generate_cmake_guard_emits_target_sources);
    PlatformBlockInput in{"drone", "aarch64", "src/main.cpp", "aarch64-apple-darwin"};
    auto txt = PlatformCMakeTargetGenerator::generateCMakeGuard(in);
    CHECK(txt.find("target_sources(drone PRIVATE src/main.cpp)") != std::string::npos, "target_sources missing");
    PASS();
}

void test_generate_cmake_guard_emits_compile_definition() {
    TEST(generate_cmake_guard_emits_compile_definition);
    PlatformBlockInput in{"drone", "aarch64", "src/main.cpp", "aarch64-apple-darwin"};
    auto txt = PlatformCMakeTargetGenerator::generateCMakeGuard(in);
    CHECK(txt.find("WHETSTONE_PLATFORM_MATCH=1") != std::string::npos, "compile definition missing");
    PASS();
}

void test_generate_ifdef_rejects_empty_macro() {
    TEST(generate_ifdef_rejects_empty_macro);
    CHECK(PlatformCMakeTargetGenerator::generateIfdef("", "int x=1;").empty(), "empty macro should fail");
    PASS();
}

void test_generate_ifdef_wraps_code() {
    TEST(generate_ifdef_wraps_code);
    auto txt = PlatformCMakeTargetGenerator::generateIfdef("WHETSTONE_X", "int x = 1;");
    CHECK(txt.find("#ifdef WHETSTONE_X") != std::string::npos, "ifdef missing");
    CHECK(txt.find("int x = 1;") != std::string::npos, "code missing");
    CHECK(txt.find("#endif") != std::string::npos, "endif missing");
    PASS();
}

void test_macro_mapping_for_aarch64_apple() {
    TEST(macro_mapping_for_aarch64_apple);
    CHECK(PlatformCMakeTargetGenerator::macroForAnnotationTarget("aarch64-apple-darwin") ==
          "WHETSTONE_AARCH64_APPLE_DARWIN", "macro mismatch");
    PASS();
}

void test_macro_mapping_for_linux() {
    TEST(macro_mapping_for_linux);
    CHECK(PlatformCMakeTargetGenerator::macroForAnnotationTarget("x86_64-linux") ==
          "WHETSTONE_X86_64_LINUX", "macro mismatch");
    PASS();
}

void test_macro_mapping_for_windows() {
    TEST(macro_mapping_for_windows);
    CHECK(PlatformCMakeTargetGenerator::macroForAnnotationTarget("x86_64-windows") ==
          "WHETSTONE_X86_64_WINDOWS", "macro mismatch");
    PASS();
}

void test_macro_mapping_falls_back_generic() {
    TEST(macro_mapping_falls_back_generic);
    CHECK(PlatformCMakeTargetGenerator::macroForAnnotationTarget("riscv64") ==
          "WHETSTONE_PLATFORM_GENERIC", "fallback macro mismatch");
    PASS();
}

int main() {
    std::cout << "Step 637: Cross-platform CMake target generator\n";

    test_supported_annotation_rejects_empty();
    test_supported_annotation_accepts_platform_target();
    test_generate_cmake_guard_requires_target_name();
    test_generate_cmake_guard_includes_if_block();
    test_generate_cmake_guard_emits_target_sources();
    test_generate_cmake_guard_emits_compile_definition();
    test_generate_ifdef_rejects_empty_macro();
    test_generate_ifdef_wraps_code();
    test_macro_mapping_for_aarch64_apple();
    test_macro_mapping_for_linux();
    test_macro_mapping_for_windows();
    test_macro_mapping_falls_back_generic();

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
