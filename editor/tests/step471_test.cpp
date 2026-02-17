// Step 471: Problem Description Parser Tests (12 tests)

#include "ArchitectProblemParser.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

void test_extracts_functional_requirements_api_auth() {
    TEST(extracts_functional_requirements_api_auth);
    auto r = ArchitectProblemParser::parse("Build a REST API with user auth and CRUD endpoints.");
    CHECK(!r.functionalRequirements.empty(), "expected functional requirements");
    CHECK(r.hasFunctionalKeyword("api"), "expected api functional requirement");
    CHECK(r.hasFunctionalKeyword("auth"), "expected auth functional requirement");
    PASS();
}

void test_extracts_nonfunctional_performance_security() {
    TEST(extracts_nonfunctional_performance_security);
    auto r = ArchitectProblemParser::parse("System must be fast, scalable, and secure.");
    CHECK(!r.nonFunctionalRequirements.empty(), "expected non-functional requirements");
    bool hasPerformance = false, hasSecurity = false;
    for (const auto& it : r.nonFunctionalRequirements) {
        if (it.text == "fast" || it.text == "performance" || it.text == "scalable") hasPerformance = true;
        if (it.text == "secure" || it.text == "security") hasSecurity = true;
    }
    CHECK(hasPerformance, "missing performance/scalability extraction");
    CHECK(hasSecurity, "missing security extraction");
    PASS();
}

void test_extracts_platform_web_mobile() {
    TEST(extracts_platform_web_mobile);
    auto r = ArchitectProblemParser::parse("Need a web and mobile app for iOS and Android.");
    CHECK(r.hasPlatform("web"), "missing web platform");
    CHECK(r.hasPlatform("mobile"), "missing mobile platform");
    CHECK(r.hasPlatform("ios"), "missing ios platform");
    CHECK(r.hasPlatform("android"), "missing android platform");
    PASS();
}

void test_extracts_integration_postgres_redis() {
    TEST(extracts_integration_postgres_redis);
    auto r = ArchitectProblemParser::parse("Integrate with PostgreSQL and Redis for data/cache.");
    bool hasPg = false, hasRedis = false;
    for (const auto& it : r.integrationRequirements) {
        if (it.text == "postgres" || it.text == "postgresql") hasPg = true;
        if (it.text == "redis") hasRedis = true;
    }
    CHECK(hasPg, "missing postgres integration");
    CHECK(hasRedis, "missing redis integration");
    PASS();
}

void test_confidence_scores_in_expected_range() {
    TEST(confidence_scores_in_expected_range);
    auto r = ArchitectProblemParser::parse("Build secure API with PostgreSQL.");
    auto checkBucket = [](const std::vector<ExtractedItem>& items) {
        for (const auto& i : items) {
            if (i.confidence < 0.0f || i.confidence > 1.0f) return false;
        }
        return true;
    };
    CHECK(checkBucket(r.functionalRequirements), "functional confidence out of range");
    CHECK(checkBucket(r.nonFunctionalRequirements), "non-functional confidence out of range");
    CHECK(checkBucket(r.integrationRequirements), "integration confidence out of range");
    PASS();
}

void test_deduplicates_keywords() {
    TEST(deduplicates_keywords);
    auto r = ArchitectProblemParser::parse("API API API with auth auth auth.");
    int apiCount = 0;
    for (const auto& it : r.functionalRequirements) if (it.text == "api") ++apiCount;
    CHECK(apiCount == 1, "keyword should be deduplicated");
    PASS();
}

void test_fallback_functional_requirement_when_none_found() {
    TEST(fallback_functional_requirement_when_none_found);
    auto r = ArchitectProblemParser::parse("We need something nice and maintainable.");
    CHECK(!r.functionalRequirements.empty(), "expected fallback functional requirement");
    CHECK(r.functionalRequirements[0].confidence <= 0.60f, "fallback should have low confidence");
    PASS();
}

void test_detects_cli_platform_constraint() {
    TEST(detects_cli_platform_constraint);
    auto r = ArchitectProblemParser::parse("Build a CLI tool for log analysis.");
    CHECK(r.hasPlatform("cli"), "missing CLI platform constraint");
    PASS();
}

void test_detects_external_api_integration() {
    TEST(detects_external_api_integration);
    auto r = ArchitectProblemParser::parse("Service must call Stripe API and store results.");
    bool hasStripe = false;
    for (const auto& it : r.integrationRequirements) if (it.text == "stripe") hasStripe = true;
    CHECK(hasStripe, "missing stripe integration extraction");
    PASS();
}

void test_case_insensitive_extraction() {
    TEST(case_insensitive_extraction);
    auto r = ArchitectProblemParser::parse("Build REST Api with AUTH and PostgreSQL backend.");
    CHECK(r.hasFunctionalKeyword("api"), "missing case-insensitive api extraction");
    CHECK(r.hasFunctionalKeyword("auth"), "missing case-insensitive auth extraction");
    PASS();
}

void test_extracts_multiple_categories_from_single_prompt() {
    TEST(extracts_multiple_categories_from_single_prompt);
    auto r = ArchitectProblemParser::parse(
        "Create a web dashboard API, secure and scalable, with PostgreSQL integration.");
    CHECK(!r.functionalRequirements.empty(), "missing functional extraction");
    CHECK(!r.nonFunctionalRequirements.empty(), "missing non-functional extraction");
    CHECK(!r.platformConstraints.empty(), "missing platform extraction");
    CHECK(!r.integrationRequirements.empty(), "missing integration extraction");
    PASS();
}

void test_longer_keywords_receive_higher_confidence() {
    TEST(longer_keywords_receive_higher_confidence);
    auto r = ArchitectProblemParser::parse("Need scalable performance.");
    float scalableConf = 0.0f, fastConf = 0.0f;
    for (const auto& it : r.nonFunctionalRequirements) {
        if (it.text == "scalable") scalableConf = it.confidence;
        if (it.text == "fast") fastConf = it.confidence;
    }
    CHECK(scalableConf >= fastConf, "longer keyword should not have lower confidence");
    PASS();
}

int main() {
    std::cout << "Step 471: Problem Description Parser Tests\n";

    test_extracts_functional_requirements_api_auth();    // 1
    test_extracts_nonfunctional_performance_security();  // 2
    test_extracts_platform_web_mobile();                 // 3
    test_extracts_integration_postgres_redis();          // 4
    test_confidence_scores_in_expected_range();          // 5
    test_deduplicates_keywords();                        // 6
    test_fallback_functional_requirement_when_none_found(); // 7
    test_detects_cli_platform_constraint();              // 8
    test_detects_external_api_integration();             // 9
    test_case_insensitive_extraction();                  // 10
    test_extracts_multiple_categories_from_single_prompt(); // 11
    test_longer_keywords_receive_higher_confidence();    // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
