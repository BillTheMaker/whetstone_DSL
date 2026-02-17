// Step 444: Migration Plan Generator Tests (12 tests)

#include "MigrationPlanGenerator.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

// Test project: 3-file C project with dependencies
//   utils.c (leaf) -> parser.c (depends on utils) -> main.c (depends on parser, utils)
static std::vector<FileInfo> testProject() {
    return {
        {"src/utils.c",
         "int clamp(int x, int lo, int hi){ return x<lo?lo:x>hi?hi:x; }\n"
         "int max(int a, int b){ return a>b?a:b; }\n",
         {"clamp", "max"}, {}},
        {"src/parser.c",
         "// parser module\n"
         "#include \"utils.h\"\n"
         "typedef struct { int type; char* text; } Token;\n"
         "Token* parse(const char* input){ int m = max(1,2); return 0; }\n"
         "void free_tokens(Token* t){ }\n"
         "int token_count(Token* t){ return clamp(0,0,100); }\n",
         {"parse", "free_tokens", "token_count"}, {"max", "clamp"}},
        {"src/main.c",
         "// main entry point\n"
         "#include \"parser.h\"\n"
         "#include \"utils.h\"\n"
         "int main(){ Token* t = parse(\"hello\"); free_tokens(t); return 0; }\n",
         {"main"}, {"parse", "free_tokens"}}
    };
}

void test_plan_created_for_project() {
    TEST(plan_created_for_project);
    auto plan = MigrationPlanGenerator::generate("testproj", testProject(), "c", "rust");
    CHECK(plan.projectName == "testproj", "expected project name");
    CHECK(plan.sourceLanguage == "c", "expected source language");
    CHECK(plan.targetLanguage == "rust", "expected target language");
    CHECK(plan.units.size() == 3, "expected 3 units");
    PASS();
}

void test_migration_units_identified() {
    TEST(migration_units_identified);
    auto plan = MigrationPlanGenerator::generate("testproj", testProject(), "c", "rust");
    CHECK(plan.hasUnit("src/utils.c"), "expected utils.c unit");
    CHECK(plan.hasUnit("src/parser.c"), "expected parser.c unit");
    CHECK(plan.hasUnit("src/main.c"), "expected main.c unit");
    PASS();
}

void test_module_names_extracted() {
    TEST(module_names_extracted);
    auto plan = MigrationPlanGenerator::generate("testproj", testProject(), "c", "rust");
    auto u = plan.unitByPath("src/utils.c");
    CHECK(u.moduleName == "utils", "expected module name 'utils'");
    auto p = plan.unitByPath("src/parser.c");
    CHECK(p.moduleName == "parser", "expected module name 'parser'");
    PASS();
}

void test_dependencies_resolved() {
    TEST(dependencies_resolved);
    auto plan = MigrationPlanGenerator::generate("testproj", testProject(), "c", "rust");
    auto parser = plan.unitByPath("src/parser.c");
    CHECK(!parser.dependsOn.empty(), "parser should depend on utils");
    auto main = plan.unitByPath("src/main.c");
    CHECK(!main.dependsOn.empty(), "main should depend on parser");
    PASS();
}

void test_leaf_modules_in_phase_zero() {
    TEST(leaf_modules_in_phase_zero);
    auto plan = MigrationPlanGenerator::generate("testproj", testProject(), "c", "rust");
    auto utils = plan.unitByPath("src/utils.c");
    CHECK(utils.phase == 0, "leaf module (utils) should be in phase 0");
    PASS();
}

void test_dependent_modules_in_later_phases() {
    TEST(dependent_modules_in_later_phases);
    auto plan = MigrationPlanGenerator::generate("testproj", testProject(), "c", "rust");
    auto utils = plan.unitByPath("src/utils.c");
    auto parser = plan.unitByPath("src/parser.c");
    auto main = plan.unitByPath("src/main.c");
    CHECK(parser.phase > utils.phase, "parser should be after utils");
    CHECK(main.phase > utils.phase, "main should be after utils");
    PASS();
}

void test_migration_order_dependencies_first() {
    TEST(migration_order_dependencies_first);
    auto plan = MigrationPlanGenerator::generate("testproj", testProject(), "c", "rust");
    auto ordered = plan.orderedUnits();
    CHECK(ordered.size() == 3, "expected 3 ordered units");
    CHECK(ordered[0].filePath == "src/utils.c", "utils should be first");
    PASS();
}

void test_phase_count() {
    TEST(phase_count);
    auto plan = MigrationPlanGenerator::generate("testproj", testProject(), "c", "rust");
    CHECK(plan.phaseCount >= 2, "expected at least 2 phases");
    auto phase0 = plan.unitsInPhase(0);
    CHECK(!phase0.empty(), "phase 0 should have units");
    PASS();
}

void test_effort_classification() {
    TEST(effort_classification);
    auto plan = MigrationPlanGenerator::generate("testproj", testProject(), "c", "rust");
    auto utils = plan.unitByPath("src/utils.c");
    CHECK(utils.effort >= 1 && utils.effort <= 3, "effort should be 1-3");
    // Small file = low effort
    CHECK(utils.effort == 1, "small file should have low effort");
    PASS();
}

void test_risk_classification() {
    TEST(risk_classification);
    auto plan = MigrationPlanGenerator::generate("testproj", testProject(), "c", "rust");
    auto parser = plan.unitByPath("src/parser.c");
    // Parser has 3 exports and dependencies → medium-high risk
    CHECK(parser.risk >= 2, "parser with exports + deps should be medium-high risk");
    PASS();
}

void test_routing_assigned() {
    TEST(routing_assigned);
    auto plan = MigrationPlanGenerator::generate("testproj", testProject(), "c", "rust");
    bool hasDetOrLlm = false;
    for (const auto& u : plan.units) {
        if (u.routing == MigrationRouting::Deterministic ||
            u.routing == MigrationRouting::LLM)
            hasDetOrLlm = true;
    }
    CHECK(hasDetOrLlm, "expected at least one non-human routing");
    PASS();
}

void test_exports_preserved_in_units() {
    TEST(exports_preserved_in_units);
    auto plan = MigrationPlanGenerator::generate("testproj", testProject(), "c", "rust");
    auto utils = plan.unitByPath("src/utils.c");
    CHECK(utils.exports.size() == 2, "utils should have 2 exports");
    bool foundClamp = false, foundMax = false;
    for (const auto& e : utils.exports) {
        if (e == "clamp") foundClamp = true;
        if (e == "max") foundMax = true;
    }
    CHECK(foundClamp && foundMax, "expected clamp and max exports");
    PASS();
}

int main() {
    std::cout << "Step 444: Migration Plan Generator Tests\n";

    test_plan_created_for_project();          // 1
    test_migration_units_identified();         // 2
    test_module_names_extracted();             // 3
    test_dependencies_resolved();              // 4
    test_leaf_modules_in_phase_zero();         // 5
    test_dependent_modules_in_later_phases();  // 6
    test_migration_order_dependencies_first(); // 7
    test_phase_count();                        // 8
    test_effort_classification();              // 9
    test_risk_classification();                // 10
    test_routing_assigned();                   // 11
    test_exports_preserved_in_units();         // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
