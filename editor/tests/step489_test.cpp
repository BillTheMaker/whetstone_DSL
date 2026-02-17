// Step 489: Secure-by-Default Code Generation Tests (12 tests)

#include "SecureByDefaultGenerator.h"

#include <iostream>
#include <string>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static SecureGenerationOutput run(const std::string& lang, SecureSnippetKind kind,
                                  const std::string& op = "") {
    SecureGenerationRequest r;
    r.language = lang;
    r.kind = kind;
    r.operationName = op;
    r.securityRequirement = "default";
    return SecureByDefaultGenerator::generate(r);
}

void test_sql_generation_uses_parameterized_queries_not_concat() {
    TEST(sql_generation_uses_parameterized_queries_not_concat);
    auto out = run("python", SecureSnippetKind::SqlQuery, "getUser");
    CHECK(out.code.find("%s") != std::string::npos, "expected parameter placeholder");
    CHECK(out.code.find("+") == std::string::npos, "should not use string concatenation");
    PASS();
}

void test_input_handling_marks_raw_until_sanitized() {
    TEST(input_handling_marks_raw_until_sanitized);
    auto out = run("typescript", SecureSnippetKind::InputHandling, "processInput");
    bool foundRaw = false;
    for (const auto& a : out.annotations) {
        if (a == "@InputValidation(raw)") foundRaw = true;
    }
    CHECK(foundRaw, "expected @InputValidation(raw)");
    CHECK(out.code.find("raw until sanitized") != std::string::npos, "missing raw comment");
    PASS();
}

void test_file_operations_include_path_traversal_guard() {
    TEST(file_operations_include_path_traversal_guard);
    auto out = run("python", SecureSnippetKind::FileOperation, "readSafe");
    CHECK(out.code.find("realpath") != std::string::npos, "missing canonicalization");
    CHECK(out.code.find("path traversal blocked") != std::string::npos, "missing traversal check");
    PASS();
}

void test_network_calls_include_timeout_and_tls_verification() {
    TEST(network_calls_include_timeout_and_tls_verification);
    auto out = run("python", SecureSnippetKind::NetworkCall, "fetchSecure");
    CHECK(out.code.find("timeout=5") != std::string::npos, "missing timeout");
    CHECK(out.code.find("verify=True") != std::string::npos, "missing TLS verify");
    PASS();
}

void test_crypto_generation_avoids_md5_and_sha1_defaults() {
    TEST(crypto_generation_avoids_md5_and_sha1_defaults);
    auto out = run("python", SecureSnippetKind::CryptoOperation, "encrypt");
    CHECK(out.code.find("MD5") == std::string::npos, "must not include md5");
    CHECK(out.code.find("SHA1") == std::string::npos, "must not include sha1");
    CHECK(out.code.find("AESGCM") != std::string::npos, "expected modern crypto");
    PASS();
}

void test_crypto_generation_is_marked_human_review_only() {
    TEST(crypto_generation_is_marked_human_review_only);
    auto out = run("rust", SecureSnippetKind::CryptoOperation, "encryptBlob");
    CHECK(out.requiresHumanReview, "crypto should require human review");
    bool hasAutom = false;
    for (const auto& a : out.annotations) if (a == "@Automatability(human)") hasAutom = true;
    CHECK(hasAutom, "missing @Automatability(human)");
    PASS();
}

void test_all_generated_snippets_include_security_risk_and_review_annotations() {
    TEST(all_generated_snippets_include_security_risk_and_review_annotations);
    auto a = run("python", SecureSnippetKind::SqlQuery);
    auto b = run("python", SecureSnippetKind::InputHandling);
    auto c = run("python", SecureSnippetKind::FileOperation);
    auto d = run("python", SecureSnippetKind::NetworkCall);
    auto e = run("python", SecureSnippetKind::CryptoOperation);
    auto hasBase = [](const SecureGenerationOutput& o) {
        bool risk = false, review = false;
        for (const auto& ann : o.annotations) {
            if (ann == "@Risk(security)") risk = true;
            if (ann == "@Review(required, human)") review = true;
        }
        return risk && review;
    };
    CHECK(hasBase(a) && hasBase(b) && hasBase(c) && hasBase(d) && hasBase(e),
          "missing base security annotations");
    PASS();
}

void test_sql_generation_for_rust_uses_bind_pattern() {
    TEST(sql_generation_for_rust_uses_bind_pattern);
    auto out = run("rust", SecureSnippetKind::SqlQuery, "fetchOne");
    CHECK(out.code.find(".bind(") != std::string::npos, "expected bind usage");
    PASS();
}

void test_network_generation_for_rust_uses_timeout_and_rustls() {
    TEST(network_generation_for_rust_uses_timeout_and_rustls);
    auto out = run("rust", SecureSnippetKind::NetworkCall, "fetch");
    CHECK(out.code.find("use_rustls_tls") != std::string::npos, "missing rustls");
    CHECK(out.code.find("from_secs(5)") != std::string::npos, "missing timeout");
    PASS();
}

void test_file_operation_for_unknown_lang_falls_back_to_secure_stub() {
    TEST(file_operation_for_unknown_lang_falls_back_to_secure_stub);
    auto out = run("lua", SecureSnippetKind::FileOperation, "readSafe");
    CHECK(out.code.find("canonical path prefix check") != std::string::npos,
          "missing secure fallback guidance");
    PASS();
}

void test_operation_name_is_respected_in_generated_code() {
    TEST(operation_name_is_respected_in_generated_code);
    auto out = run("typescript", SecureSnippetKind::InputHandling, "sanitizeLater");
    CHECK(out.code.find("sanitizeLater") != std::string::npos, "operation name missing");
    PASS();
}

void test_security_requirement_annotation_is_emitted() {
    TEST(security_requirement_annotation_is_emitted);
    SecureGenerationRequest r;
    r.language = "python";
    r.kind = SecureSnippetKind::SqlQuery;
    r.operationName = "query";
    r.securityRequirement = "pci-dss";
    auto out = SecureByDefaultGenerator::generate(r);
    bool found = false;
    for (const auto& a : out.annotations) {
        if (a == "@SecurityRequirement(pci-dss)") found = true;
    }
    CHECK(found, "missing security requirement annotation");
    PASS();
}

int main() {
    std::cout << "Step 489: Secure-by-Default Code Generation Tests\n";

    test_sql_generation_uses_parameterized_queries_not_concat();      // 1
    test_input_handling_marks_raw_until_sanitized();                  // 2
    test_file_operations_include_path_traversal_guard();              // 3
    test_network_calls_include_timeout_and_tls_verification();        // 4
    test_crypto_generation_avoids_md5_and_sha1_defaults();            // 5
    test_crypto_generation_is_marked_human_review_only();             // 6
    test_all_generated_snippets_include_security_risk_and_review_annotations(); // 7
    test_sql_generation_for_rust_uses_bind_pattern();                 // 8
    test_network_generation_for_rust_uses_timeout_and_rustls();       // 9
    test_file_operation_for_unknown_lang_falls_back_to_secure_stub(); // 10
    test_operation_name_is_respected_in_generated_code();             // 11
    test_security_requirement_annotation_is_emitted();                // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
