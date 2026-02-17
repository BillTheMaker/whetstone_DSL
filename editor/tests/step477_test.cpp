// Step 477: Architecture Templates Tests (12 tests)

#include "ArchitectTemplates.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static TemplateInput sampleInput() {
    TemplateInput in;
    in.projectName = "bookstore";
    in.primaryEntity = "Book";
    in.fields = {"id", "title", "author"};
    in.authMethod = "jwt";
    return in;
}

void test_rest_api_template_contains_expected_modules() {
    TEST(rest_api_template_contains_expected_modules);
    auto out = ArchitectTemplates::instantiate(ArchitectureTemplateKind::RestApi, sampleInput());
    CHECK(out.skeleton.hasModule("routes"), "missing routes");
    CHECK(out.skeleton.hasModule("handlers"), "missing handlers");
    CHECK(out.skeleton.hasModule("models"), "missing models");
    CHECK(out.skeleton.hasModule("middleware"), "missing middleware");
    CHECK(out.skeleton.hasModule("database"), "missing database");
    PASS();
}

void test_cli_template_contains_cli_command_output_modules() {
    TEST(cli_template_contains_cli_command_output_modules);
    auto out = ArchitectTemplates::instantiate(ArchitectureTemplateKind::CliTool, sampleInput());
    CHECK(out.skeleton.hasModule("cli"), "missing cli");
    CHECK(out.skeleton.hasModule("commands"), "missing commands");
    CHECK(out.skeleton.hasModule("output"), "missing output");
    PASS();
}

void test_library_template_contains_api_internal_tests_docs() {
    TEST(library_template_contains_api_internal_tests_docs);
    auto out = ArchitectTemplates::instantiate(ArchitectureTemplateKind::Library, sampleInput());
    CHECK(out.skeleton.hasModule("public_api"), "missing public_api");
    CHECK(out.skeleton.hasModule("internal"), "missing internal");
    CHECK(out.skeleton.hasModule("tests"), "missing tests");
    CHECK(out.skeleton.hasModule("docs"), "missing docs");
    PASS();
}

void test_microservice_template_contains_service_transport_storage_health() {
    TEST(microservice_template_contains_service_transport_storage_health);
    auto out = ArchitectTemplates::instantiate(ArchitectureTemplateKind::Microservice, sampleInput());
    CHECK(out.skeleton.hasModule("service"), "missing service");
    CHECK(out.skeleton.hasModule("transport"), "missing transport");
    CHECK(out.skeleton.hasModule("storage"), "missing storage");
    CHECK(out.skeleton.hasModule("health"), "missing health");
    PASS();
}

void test_fullstack_template_contains_frontend_backend_database_deploy() {
    TEST(fullstack_template_contains_frontend_backend_database_deploy);
    auto out = ArchitectTemplates::instantiate(ArchitectureTemplateKind::FullStack, sampleInput());
    CHECK(out.skeleton.hasModule("frontend"), "missing frontend");
    CHECK(out.skeleton.hasModule("backend"), "missing backend");
    CHECK(out.skeleton.hasModule("database"), "missing database");
    CHECK(out.skeleton.hasModule("deploy"), "missing deploy");
    PASS();
}

void test_template_functions_are_annotated() {
    TEST(template_functions_are_annotated);
    auto out = ArchitectTemplates::instantiate(ArchitectureTemplateKind::RestApi, sampleInput());
    auto m = out.skeleton.getModule("handlers");
    CHECK(!m.functions.empty(), "missing handler functions");
    bool hasIntent = false, hasContract = false;
    for (const auto& a : m.functions[0].annotations) {
        if (a.key == "@Intent") hasIntent = true;
        if (a.key == "@Contract") hasContract = true;
    }
    CHECK(hasIntent && hasContract, "missing expected annotations");
    PASS();
}

void test_template_is_parameterized_by_primary_entity() {
    TEST(template_is_parameterized_by_primary_entity);
    auto out = ArchitectTemplates::instantiate(ArchitectureTemplateKind::RestApi, sampleInput());
    auto handlers = out.skeleton.getModule("handlers");
    bool hasCreateBook = false;
    for (const auto& f : handlers.functions) if (f.name == "createBook") hasCreateBook = true;
    CHECK(hasCreateBook, "expected entity-parameterized function name");
    PASS();
}

void test_template_module_dependencies_present() {
    TEST(template_module_dependencies_present);
    auto out = ArchitectTemplates::instantiate(ArchitectureTemplateKind::RestApi, sampleInput());
    auto routes = out.skeleton.getModule("routes");
    bool hasHandlersDep = false;
    for (const auto& d : routes.dependencies) if (d == "handlers") hasHandlersDep = true;
    CHECK(hasHandlersDep, "expected routes->handlers dependency");
    PASS();
}

void test_template_output_contains_template_note() {
    TEST(template_output_contains_template_note);
    auto out = ArchitectTemplates::instantiate(ArchitectureTemplateKind::CliTool, sampleInput());
    CHECK(!out.notes.empty(), "expected template note");
    PASS();
}

void test_template_module_languages_match_expected_defaults() {
    TEST(template_module_languages_match_expected_defaults);
    auto rest = ArchitectTemplates::instantiate(ArchitectureTemplateKind::RestApi, sampleInput());
    auto micro = ArchitectTemplates::instantiate(ArchitectureTemplateKind::Microservice, sampleInput());
    auto full = ArchitectTemplates::instantiate(ArchitectureTemplateKind::FullStack, sampleInput());
    CHECK(rest.skeleton.getModule("handlers").language == "typescript", "rest handlers language mismatch");
    CHECK(micro.skeleton.getModule("service").language == "go", "microservice language mismatch");
    CHECK(full.skeleton.getModule("backend").language == "python", "fullstack backend language mismatch");
    PASS();
}

void test_template_outputs_are_nonempty_across_all_kinds() {
    TEST(template_outputs_are_nonempty_across_all_kinds);
    auto in = sampleInput();
    auto a = ArchitectTemplates::instantiate(ArchitectureTemplateKind::RestApi, in);
    auto b = ArchitectTemplates::instantiate(ArchitectureTemplateKind::CliTool, in);
    auto c = ArchitectTemplates::instantiate(ArchitectureTemplateKind::Library, in);
    auto d = ArchitectTemplates::instantiate(ArchitectureTemplateKind::Microservice, in);
    auto e = ArchitectTemplates::instantiate(ArchitectureTemplateKind::FullStack, in);
    CHECK(!a.skeleton.modules.empty(), "rest template empty");
    CHECK(!b.skeleton.modules.empty(), "cli template empty");
    CHECK(!c.skeleton.modules.empty(), "library template empty");
    CHECK(!d.skeleton.modules.empty(), "microservice template empty");
    CHECK(!e.skeleton.modules.empty(), "fullstack template empty");
    PASS();
}

void test_templates_include_auth_module_for_rest_api() {
    TEST(templates_include_auth_module_for_rest_api);
    auto out = ArchitectTemplates::instantiate(ArchitectureTemplateKind::RestApi, sampleInput());
    CHECK(out.skeleton.hasModule("auth"), "expected auth module in REST template");
    PASS();
}

int main() {
    std::cout << "Step 477: Architecture Templates Tests\n";

    test_rest_api_template_contains_expected_modules();                // 1
    test_cli_template_contains_cli_command_output_modules();           // 2
    test_library_template_contains_api_internal_tests_docs();          // 3
    test_microservice_template_contains_service_transport_storage_health(); // 4
    test_fullstack_template_contains_frontend_backend_database_deploy(); // 5
    test_template_functions_are_annotated();                           // 6
    test_template_is_parameterized_by_primary_entity();                // 7
    test_template_module_dependencies_present();                       // 8
    test_template_output_contains_template_note();                     // 9
    test_template_module_languages_match_expected_defaults();          // 10
    test_template_outputs_are_nonempty_across_all_kinds();            // 11
    test_templates_include_auth_module_for_rest_api();                // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
