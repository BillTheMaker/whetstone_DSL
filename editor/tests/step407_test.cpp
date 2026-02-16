// Step 407: VB.NET Parser Tests (12 tests)

#include "ast/VBNetParser.h"
#include "Pipeline.h"
#include "ast/Function.h"
#include "ast/Variable.h"
#include "ast/ClassDeclaration.h"
#include "ast/EnumNamespaceNodes.h"
#include "ast/Statement.h"

#include <iostream>
#include <string>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

void test_parse_sub_method() {
    TEST(parse_sub_method);
    auto mod = VBNetParser::parseVBNet(
        "Public Sub DoWork(x As Integer)\n"
        "End Sub\n");
    CHECK(mod != nullptr, "module is null");
    auto fns = mod->getChildren("functions");
    CHECK(fns.size() == 1, "expected 1 function");
    auto* fn = static_cast<Function*>(fns[0]);
    CHECK(fn->name == "DoWork", "expected method DoWork");
    PASS();
}

void test_parse_function_method() {
    TEST(parse_function_method);
    auto mod = VBNetParser::parseVBNet(
        "Public Function Sum(a As Integer, b As Integer) As Integer\n"
        "End Function\n");
    CHECK(mod != nullptr, "module is null");
    auto fns = mod->getChildren("functions");
    CHECK(fns.size() == 1, "expected 1 function");
    auto* fn = static_cast<Function*>(fns[0]);
    CHECK(fn->name == "Sum", "expected method Sum");
    PASS();
}

void test_parse_class() {
    TEST(parse_class);
    auto mod = VBNetParser::parseVBNet(
        "Class Person\n"
        "End Class\n");
    CHECK(mod != nullptr, "module is null");
    auto classes = mod->getChildren("classes");
    CHECK(classes.size() == 1, "expected 1 class");
    auto* cls = static_cast<ClassDeclaration*>(classes[0]);
    CHECK(cls->name == "Person", "expected class Person");
    PASS();
}

void test_parse_interface() {
    TEST(parse_interface);
    auto mod = VBNetParser::parseVBNet(
        "Interface IWorker\n"
        "End Interface\n");
    CHECK(mod != nullptr, "module is null");
    auto ifaces = mod->getChildren("interfaces");
    CHECK(ifaces.size() == 1, "expected 1 interface");
    auto* iface = static_cast<InterfaceDeclaration*>(ifaces[0]);
    CHECK(iface->name == "IWorker", "expected interface IWorker");
    PASS();
}

void test_parse_module_namespace() {
    TEST(parse_module_namespace);
    auto mod = VBNetParser::parseVBNet(
        "Module Utils\n"
        "End Module\n");
    CHECK(mod != nullptr, "module is null");
    auto stmts = mod->getChildren("statements");
    NamespaceDeclaration* ns = nullptr;
    for (auto* s : stmts) {
        if (s->conceptType == "NamespaceDeclaration") ns = static_cast<NamespaceDeclaration*>(s);
    }
    CHECK(ns != nullptr, "expected namespace declaration");
    CHECK(ns->name == "Utils", "expected module name Utils");
    PASS();
}

void test_parse_dim_variable() {
    TEST(parse_dim_variable);
    auto mod = VBNetParser::parseVBNet("Dim x As Integer = 5\n");
    CHECK(mod != nullptr, "module is null");
    auto vars = mod->getChildren("variables");
    CHECK(vars.size() == 1, "expected 1 variable");
    auto* var = static_cast<Variable*>(vars[0]);
    CHECK(var->name == "x", "expected variable x");
    PASS();
}

void test_parse_if_then_block() {
    TEST(parse_if_then_block);
    auto mod = VBNetParser::parseVBNet(
        "If x > 0 Then\n"
        "ElseIf x < 0 Then\n"
        "Else\n"
        "End If\n");
    CHECK(mod != nullptr, "module is null");
    bool foundIf = false;
    for (auto* s : mod->getChildren("statements")) {
        if (s->conceptType == "IfStatement") foundIf = true;
    }
    CHECK(foundIf, "expected IfStatement marker");
    PASS();
}

void test_parse_for_each_block() {
    TEST(parse_for_each_block);
    auto mod = VBNetParser::parseVBNet(
        "For Each item In collection\n"
        "Next\n");
    CHECK(mod != nullptr, "module is null");
    bool foundFor = false;
    for (auto* s : mod->getChildren("statements")) {
        if (s->conceptType == "ForLoop") {
            auto* loop = static_cast<ForLoop*>(s);
            CHECK(loop->iteratorName == "item", "expected iterator item");
            foundFor = true;
        }
    }
    CHECK(foundFor, "expected ForLoop marker");
    PASS();
}

void test_case_insensitive_keywords() {
    TEST(case_insensitive_keywords);
    auto mod = VBNetParser::parseVBNet(
        "public sub Run()\n"
        "end sub\n");
    CHECK(mod != nullptr, "module is null");
    CHECK(mod->getChildren("functions").size() == 1, "expected function with lowercase keyword");
    PASS();
}

void test_parse_with_diagnostics() {
    TEST(parse_with_diagnostics);
    auto result = VBNetParser::parseVBNetWithDiagnostics(
        "Function Echo(value As String) As String\n"
        "End Function\n");
    CHECK(result.module != nullptr, "module is null");
    CHECK(result.module->getChildren("functions").size() == 1, "expected 1 function");
    PASS();
}

void test_parse_mixed_file() {
    TEST(parse_mixed_file);
    auto mod = VBNetParser::parseVBNet(
        "' comment\n"
        "Module App\n"
        "End Module\n"
        "Class Person\n"
        "End Class\n"
        "Public Sub Main()\n"
        "End Sub\n"
        "Dim count As Integer = 1\n");
    CHECK(mod != nullptr, "module is null");
    CHECK(mod->getChildren("statements").size() >= 1, "expected module statement");
    CHECK(mod->getChildren("classes").size() == 1, "expected class");
    CHECK(mod->getChildren("functions").size() == 1, "expected function");
    CHECK(mod->getChildren("variables").size() == 1, "expected variable");
    PASS();
}

void test_pipeline_parse_routing() {
    TEST(pipeline_parse_routing);
    Pipeline p;
    std::vector<ParseDiagnostic> d1, d2, d3;
    auto m1 = p.parse("Sub Main()\nEnd Sub\n", "vbnet", d1);
    auto m2 = p.parse("Sub Main()\nEnd Sub\n", "vb", d2);
    auto m3 = p.parse("Sub Main()\nEnd Sub\n", "vb.net", d3);
    CHECK(m1 != nullptr && m2 != nullptr && m3 != nullptr, "expected parse success for aliases");
    CHECK(m1->targetLanguage == "vbnet", "expected vbnet target language");
    CHECK(m2->targetLanguage == "vbnet", "expected vb alias target language");
    CHECK(m3->targetLanguage == "vbnet", "expected vb.net alias target language");
    PASS();
}

int main() {
    std::cout << "Step 407: VB.NET Parser Tests\n";

    test_parse_sub_method();       // 1
    test_parse_function_method();  // 2
    test_parse_class();            // 3
    test_parse_interface();        // 4
    test_parse_module_namespace(); // 5
    test_parse_dim_variable();     // 6
    test_parse_if_then_block();    // 7
    test_parse_for_each_block();   // 8
    test_case_insensitive_keywords(); // 9
    test_parse_with_diagnostics(); // 10
    test_parse_mixed_file();       // 11
    test_pipeline_parse_routing(); // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
