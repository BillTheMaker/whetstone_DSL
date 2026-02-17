// Step 488: Security-Preserving Translation Tests (12 tests)

#include "SecurityPreservingTranslation.h"

#include <iostream>
#include <string>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static SecurityTranslationOutput run(const std::string& target,
                                     std::initializer_list<SecuritySourceAnnotation> anns) {
    SecurityTranslationInput in;
    in.sourceLanguage = "python";
    in.targetLanguage = target;
    in.annotations.assign(anns.begin(), anns.end());
    return SecurityPreservingTranslation::translate(in);
}

void test_input_validation_is_preserved_in_target_language() {
    TEST(input_validation_is_preserved_in_target_language);
    auto out = run("rust", {{"@InputValidation", "sanitized"}});
    CHECK(out.mappings.size() == 1, "mapping size mismatch");
    CHECK(out.mappings[0].mappedValue.find("input_validation") != std::string::npos,
          "input validation mapping missing");
    CHECK(out.mappings[0].preserved, "annotation should be preserved");
    PASS();
}

void test_trust_boundary_is_maintained() {
    TEST(trust_boundary_is_maintained);
    auto out = run("typescript", {{"@TrustBoundary", "external-api"}});
    CHECK(out.mappings[0].mappedValue == "boundary:external-api", "trust boundary mapping mismatch");
    PASS();
}

void test_auth_annotation_maps_to_target_auth_pattern() {
    TEST(auth_annotation_maps_to_target_auth_pattern);
    auto out = run("python", {{"@Auth", "jwt"}});
    CHECK(out.mappings[0].mappedValue.find("auth_required") != std::string::npos,
          "auth mapping missing");
    PASS();
}

void test_encryption_annotation_maps_to_target_crypto_library() {
    TEST(encryption_annotation_maps_to_target_crypto_library);
    auto out = run("rust", {{"@Encryption", "aes256"}});
    CHECK(out.mappings[0].mappedValue.find("ring::aes256") != std::string::npos,
          "encryption mapping missing");
    PASS();
}

void test_unknown_security_annotation_triggers_review_required() {
    TEST(unknown_security_annotation_triggers_review_required);
    auto out = run("python", {{"@SecurityCustom", "foo"}});
    CHECK(out.mappings[0].reviewRequired, "expected review required");
    CHECK(out.hasBlockingReview, "blocking review should be set");
    PASS();
}

void test_empty_annotation_list_returns_empty_with_note() {
    TEST(empty_annotation_list_returns_empty_with_note);
    auto out = run("python", {});
    CHECK(out.mappings.empty(), "expected empty mappings");
    CHECK(!out.notes.empty(), "expected explanatory note");
    PASS();
}

void test_insecure_crypto_algorithm_requires_human_review() {
    TEST(insecure_crypto_algorithm_requires_human_review);
    auto out = run("python", {{"@Encryption", "md5"}});
    CHECK(out.mappings[0].reviewRequired, "expected review for insecure crypto");
    CHECK(out.mappings[0].reviewReason.find("Insecure crypto") != std::string::npos,
          "missing insecure crypto reason");
    PASS();
}

void test_unmappable_target_language_requires_review_without_dropping_annotation() {
    TEST(unmappable_target_language_requires_review_without_dropping_annotation);
    auto out = run("haskell", {{"@InputValidation", "raw"}});
    CHECK(out.mappings[0].reviewRequired, "expected review required");
    CHECK(out.mappings[0].preserved, "must preserve annotation");
    CHECK(out.preservedCount() == 1, "annotation should remain present");
    PASS();
}

void test_security_annotations_never_auto_removed_count_matches_source() {
    TEST(security_annotations_never_auto_removed_count_matches_source);
    auto out = run("rust", {
        {"@InputValidation", "raw"},
        {"@TrustBoundary", "db"},
        {"@Auth", "session"},
        {"@Encryption", "aes256"}
    });
    CHECK(out.mappings.size() == 4, "all source annotations should be mapped");
    CHECK(out.preservedCount() == 4, "all annotations must remain preserved");
    PASS();
}

void test_blocking_review_flag_only_when_required() {
    TEST(blocking_review_flag_only_when_required);
    auto ok = run("rust", {{"@InputValidation", "sanitized"}, {"@Auth", "jwt"}});
    auto bad = run("rust", {{"@Secrets", "rotation:30d"}});
    CHECK(!ok.hasBlockingReview, "non-problematic set should not block");
    CHECK(bad.hasBlockingReview, "secrets set should block");
    PASS();
}

void test_cors_annotation_maps_for_supported_targets() {
    TEST(cors_annotation_maps_for_supported_targets);
    auto out = run("typescript", {{"@CORS", "public-read"}});
    CHECK(out.mappings[0].mappedValue.find("cors(") != std::string::npos, "missing cors mapping");
    PASS();
}

void test_secrets_annotation_always_requires_review() {
    TEST(secrets_annotation_always_requires_review);
    auto out = run("python", {{"@Secrets", "rotation:7d"}});
    CHECK(out.mappings[0].reviewRequired, "secrets must require review");
    CHECK(out.mappings[0].mappedValue.find("secret_manager") != std::string::npos,
          "expected secret manager mapping");
    PASS();
}

int main() {
    std::cout << "Step 488: Security-Preserving Translation Tests\n";

    test_input_validation_is_preserved_in_target_language();                 // 1
    test_trust_boundary_is_maintained();                                     // 2
    test_auth_annotation_maps_to_target_auth_pattern();                      // 3
    test_encryption_annotation_maps_to_target_crypto_library();              // 4
    test_unknown_security_annotation_triggers_review_required();             // 5
    test_empty_annotation_list_returns_empty_with_note();                    // 6
    test_insecure_crypto_algorithm_requires_human_review();                  // 7
    test_unmappable_target_language_requires_review_without_dropping_annotation(); // 8
    test_security_annotations_never_auto_removed_count_matches_source();     // 9
    test_blocking_review_flag_only_when_required();                          // 10
    test_cors_annotation_maps_for_supported_targets();                       // 11
    test_secrets_annotation_always_requires_review();                        // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
