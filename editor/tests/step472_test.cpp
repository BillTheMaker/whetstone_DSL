// Step 472: Module Decomposition Engine Tests (12 tests)

#include "ArchitectModuleDecomposer.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static StructuredRequirements reqFromText(const std::string& s) {
    return ArchitectProblemParser::parse(s);
}

void test_decomposes_auth_api_data_modules() {
    TEST(decomposes_auth_api_data_modules);
    auto req = reqFromText("Build a REST API with auth and PostgreSQL.");
    auto g = ArchitectModuleDecomposer::decompose(req);
    CHECK(g.hasModule("auth"), "missing auth module");
    CHECK(g.hasModule("api"), "missing api module");
    CHECK(g.hasModule("data"), "missing data module");
    PASS();
}

void test_decomposes_ui_module_from_web_mobile_constraints() {
    TEST(decomposes_ui_module_from_web_mobile_constraints);
    auto req = reqFromText("Create a web and mobile dashboard.");
    auto g = ArchitectModuleDecomposer::decompose(req);
    CHECK(g.hasModule("ui"), "missing ui module");
    PASS();
}

void test_decomposes_search_reporting_modules() {
    TEST(decomposes_search_reporting_modules);
    auto req = reqFromText("App requires search and reporting features.");
    auto g = ArchitectModuleDecomposer::decompose(req);
    CHECK(g.hasModule("search"), "missing search module");
    CHECK(g.hasModule("reporting"), "missing reporting module");
    PASS();
}

void test_adds_integration_module_for_external_systems() {
    TEST(adds_integration_module_for_external_systems);
    auto req = reqFromText("Integrate Stripe and Salesforce APIs.");
    auto g = ArchitectModuleDecomposer::decompose(req);
    CHECK(g.hasModule("integrations"), "missing integrations module");
    PASS();
}

void test_cross_cutting_modules_are_present() {
    TEST(cross_cutting_modules_are_present);
    auto req = reqFromText("Build a REST API.");
    auto g = ArchitectModuleDecomposer::decompose(req);
    CHECK(g.hasModule("logging"), "missing logging");
    CHECK(g.hasModule("config"), "missing config");
    CHECK(g.hasModule("error_handling"), "missing error_handling");
    PASS();
}

void test_security_nonfunctional_adds_security_module() {
    TEST(security_nonfunctional_adds_security_module);
    auto req = reqFromText("System must be secure and reliable.");
    auto g = ArchitectModuleDecomposer::decompose(req);
    CHECK(g.hasModule("security"), "missing security module");
    PASS();
}

void test_layered_strategy_generates_expected_dependencies() {
    TEST(layered_strategy_generates_expected_dependencies);
    auto req = reqFromText("Web REST API with auth and PostgreSQL.");
    auto g = ArchitectModuleDecomposer::decompose(req, DecompositionStrategy::Layered);
    bool sawUiApi = false;
    bool sawApiData = false;
    for (const auto& e : g.edges) {
        if (e.from == "ui" && e.to == "api") sawUiApi = true;
        if (e.from == "api" && e.to == "data") sawApiData = true;
    }
    CHECK(sawUiApi, "missing ui->api edge");
    CHECK(sawApiData, "missing api->data edge");
    PASS();
}

void test_microservice_strategy_changes_edge_shape() {
    TEST(microservice_strategy_changes_edge_shape);
    auto req = reqFromText("REST API with auth and stripe integration.");
    auto g = ArchitectModuleDecomposer::decompose(req, DecompositionStrategy::Microservice);
    bool sawApiIntegrations = false;
    for (const auto& e : g.edges) {
        if (e.from == "api" && e.to == "integrations") sawApiIntegrations = true;
    }
    CHECK(sawApiIntegrations, "missing api->integrations edge in microservice strategy");
    PASS();
}

void test_monolith_strategy_uses_core_hub() {
    TEST(monolith_strategy_uses_core_hub);
    auto req = reqFromText("simple app with auth and web ui");
    auto g = ArchitectModuleDecomposer::decompose(req, DecompositionStrategy::Monolith);
    bool sawCoreDep = false;
    for (const auto& e : g.edges) {
        if (e.from == "core") sawCoreDep = true;
    }
    CHECK(g.hasModule("core"), "missing core module");
    CHECK(sawCoreDep, "expected core dependencies in monolith strategy");
    PASS();
}

void test_complexity_scores_are_clamped() {
    TEST(complexity_scores_are_clamped);
    auto req = reqFromText("web mobile api auth search reporting notification postgres stripe secure scalable");
    auto g = ArchitectModuleDecomposer::decompose(req);
    for (const auto& n : g.nodes) {
        CHECK(n.complexity >= 1 && n.complexity <= 10, "complexity out of range");
    }
    PASS();
}

void test_dependency_count_helper_reports_nonzero() {
    TEST(dependency_count_helper_reports_nonzero);
    auto req = reqFromText("REST API with auth and PostgreSQL.");
    auto g = ArchitectModuleDecomposer::decompose(req);
    CHECK(g.dependencyCountFor("api") > 0, "expected api dependencies");
    PASS();
}

void test_fallback_core_module_when_sparse_requirements() {
    TEST(fallback_core_module_when_sparse_requirements);
    StructuredRequirements req;
    req.functionalRequirements = {{"core application behavior", 0.55f}};
    auto g = ArchitectModuleDecomposer::decompose(req, DecompositionStrategy::Monolith);
    CHECK(g.hasModule("core"), "missing fallback core module");
    PASS();
}

int main() {
    std::cout << "Step 472: Module Decomposition Engine Tests\n";

    test_decomposes_auth_api_data_modules();              // 1
    test_decomposes_ui_module_from_web_mobile_constraints(); // 2
    test_decomposes_search_reporting_modules();           // 3
    test_adds_integration_module_for_external_systems();  // 4
    test_cross_cutting_modules_are_present();             // 5
    test_security_nonfunctional_adds_security_module();   // 6
    test_layered_strategy_generates_expected_dependencies(); // 7
    test_microservice_strategy_changes_edge_shape();      // 8
    test_monolith_strategy_uses_core_hub();               // 9
    test_complexity_scores_are_clamped();                 // 10
    test_dependency_count_helper_reports_nonzero();       // 11
    test_fallback_core_module_when_sparse_requirements(); // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
