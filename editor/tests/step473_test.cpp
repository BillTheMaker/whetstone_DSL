// Step 473: Technology Stack Selector Tests (12 tests)

#include "ArchitectTechStackSelector.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static StructuredRequirements reqFrom(const std::string& s) {
    return ArchitectProblemParser::parse(s);
}

static ModuleGraph graphFrom(const std::string& s, DecompositionStrategy strategy = DecompositionStrategy::Layered) {
    auto req = reqFrom(s);
    return ArchitectModuleDecomposer::decompose(req, strategy);
}

void test_selects_frontend_typescript_for_ui_module() {
    TEST(selects_frontend_typescript_for_ui_module);
    auto req = reqFrom("Build a web dashboard.");
    auto g = ArchitectModuleDecomposer::decompose(req);
    auto d = ArchitectTechStackSelector::select(req, g);
    CHECK(d.hasChoiceFor("ui"), "missing ui choice");
    auto c = d.getChoice("ui");
    CHECK(c.language == "typescript", "expected typescript for ui");
    CHECK(c.framework == "react", "expected react framework");
    PASS();
}

void test_selects_rust_for_perf_critical_api() {
    TEST(selects_rust_for_perf_critical_api);
    auto req = reqFrom("Need a secure low-latency REST API.");
    auto g = ArchitectModuleDecomposer::decompose(req);
    auto d = ArchitectTechStackSelector::select(req, g);
    auto c = d.getChoice("api");
    CHECK(c.language == "rust", "expected rust for perf/secure api");
    PASS();
}

void test_selects_python_backend_when_no_perf_constraints() {
    TEST(selects_python_backend_when_no_perf_constraints);
    auto req = reqFrom("Build a REST API for internal admin tooling.");
    auto g = ArchitectModuleDecomposer::decompose(req);
    auto d = ArchitectTechStackSelector::select(req, g);
    auto c = d.getChoice("api");
    CHECK(c.language == "python", "expected python default backend");
    CHECK(c.framework == "fastapi", "expected fastapi framework");
    PASS();
}

void test_selects_postgresql_when_requirement_mentions_postgres() {
    TEST(selects_postgresql_when_requirement_mentions_postgres);
    auto req = reqFrom("API with PostgreSQL storage.");
    auto g = ArchitectModuleDecomposer::decompose(req);
    auto d = ArchitectTechStackSelector::select(req, g);
    auto c = d.getChoice("data");
    CHECK(c.database == "postgresql", "expected postgresql database");
    PASS();
}

void test_selects_mysql_when_requirement_mentions_mysql() {
    TEST(selects_mysql_when_requirement_mentions_mysql);
    auto req = reqFrom("Service with MySQL backend.");
    auto g = ArchitectModuleDecomposer::decompose(req);
    auto d = ArchitectTechStackSelector::select(req, g);
    auto c = d.getChoice("data");
    CHECK(c.database == "mysql", "expected mysql database");
    PASS();
}

void test_applies_backend_language_preference_override() {
    TEST(applies_backend_language_preference_override);
    auto req = reqFrom("Need a secure low-latency API.");
    auto g = ArchitectModuleDecomposer::decompose(req);
    auto d = ArchitectTechStackSelector::select(req, g, {{"backend_language", "go"}});
    auto c = d.getChoice("api");
    CHECK(c.language == "go", "expected backend preference override");
    PASS();
}

void test_applies_database_preference_override() {
    TEST(applies_database_preference_override);
    auto req = reqFrom("API with postgres.");
    auto g = ArchitectModuleDecomposer::decompose(req);
    auto d = ArchitectTechStackSelector::select(req, g, {{"database", "sqlite"}});
    auto c = d.getChoice("data");
    CHECK(c.database == "sqlite", "expected database preference override");
    PASS();
}

void test_applies_frontend_language_preference_override() {
    TEST(applies_frontend_language_preference_override);
    auto req = reqFrom("Build a web dashboard.");
    auto g = ArchitectModuleDecomposer::decompose(req);
    auto d = ArchitectTechStackSelector::select(req, g, {{"frontend_language", "elm"}});
    auto c = d.getChoice("ui");
    CHECK(c.language == "elm", "expected frontend preference override");
    PASS();
}

void test_assigns_integrations_module_to_adapter_stack() {
    TEST(assigns_integrations_module_to_adapter_stack);
    auto req = reqFrom("Integrate with Stripe and Salesforce APIs.");
    auto g = ArchitectModuleDecomposer::decompose(req);
    auto d = ArchitectTechStackSelector::select(req, g);
    auto c = d.getChoice("integrations");
    CHECK(c.language == "python", "expected python for integration adapters");
    CHECK(c.framework == "http-client", "expected adapter framework");
    PASS();
}

void test_confidence_scores_in_reasonable_range() {
    TEST(confidence_scores_in_reasonable_range);
    auto req = reqFrom("Web dashboard with API and postgres.");
    auto g = ArchitectModuleDecomposer::decompose(req);
    auto d = ArchitectTechStackSelector::select(req, g);
    for (const auto& c : d.choices) {
        CHECK(c.confidence >= 0.0f && c.confidence <= 1.0f, "confidence out of range");
    }
    PASS();
}

void test_every_non_crosscutting_module_has_choice() {
    TEST(every_non_crosscutting_module_has_choice);
    auto req = reqFrom("Web API with auth, search, and PostgreSQL.");
    auto g = ArchitectModuleDecomposer::decompose(req);
    auto d = ArchitectTechStackSelector::select(req, g);
    for (const auto& n : g.nodes) {
        if (n.crossCutting) continue;
        CHECK(d.hasChoiceFor(n.name), "missing tech choice for module");
    }
    PASS();
}

void test_fallback_choice_for_empty_graph() {
    TEST(fallback_choice_for_empty_graph);
    StructuredRequirements req;
    ModuleGraph g;
    auto d = ArchitectTechStackSelector::select(req, g);
    CHECK(!d.choices.empty(), "expected fallback choice");
    CHECK(d.choices[0].moduleName == "core", "expected core fallback choice");
    PASS();
}

int main() {
    std::cout << "Step 473: Technology Stack Selector Tests\n";

    test_selects_frontend_typescript_for_ui_module();    // 1
    test_selects_rust_for_perf_critical_api();           // 2
    test_selects_python_backend_when_no_perf_constraints();// 3
    test_selects_postgresql_when_requirement_mentions_postgres(); // 4
    test_selects_mysql_when_requirement_mentions_mysql(); // 5
    test_applies_backend_language_preference_override();  // 6
    test_applies_database_preference_override();          // 7
    test_applies_frontend_language_preference_override(); // 8
    test_assigns_integrations_module_to_adapter_stack();  // 9
    test_confidence_scores_in_reasonable_range();         // 10
    test_every_non_crosscutting_module_has_choice();      // 11
    test_fallback_choice_for_empty_graph();               // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
