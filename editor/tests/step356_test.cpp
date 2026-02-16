// Step 356: Breadcrumb Navigation (12 tests)

#include <cassert>
#include <iostream>
#include <string>
#include "panels/BreadcrumbBar.h"
#include "ast/ClassDeclaration.h"
#include "ast/Function.h"
#include "ast/Variable.h"
#include "ast/Annotation.h"

int main() {
    int passed = 0;

    auto theme = getDefaultDarkTheme();

    // Build shared AST:
    // module app > class Engine > methods start/stop > variable counter
    Module module("mod1", "app", "python");
    auto* cls = new ClassDeclaration("cls1", "Engine");
    auto* start = new MethodDeclaration("m1", "start");
    auto* stop = new MethodDeclaration("m2", "stop");
    auto* var = new Variable("v1", "counter");
    auto* anno = new IntentAnnotation();
    anno->id = "a1";
    anno->summary = "log";

    start->addChild("body", var);
    start->addChild("annotations", anno);
    cls->addChild("methods", start);
    cls->addChild("methods", stop);
    module.addChild("classes", cls);

    // Test 1: breadcrumb shows module name
    {
        auto model = buildBreadcrumbBar(&module, &module, theme);
        assert(!model.segments.empty());
        assert(model.segments.front().label == "app");
        std::cout << "Test 1 PASSED: module name shown\n";
        passed++;
    }

    // Test 2: nested path updates with cursor
    {
        auto modelA = buildBreadcrumbBar(&module, start, theme);
        auto modelB = buildBreadcrumbBar(&module, var, theme);
        assert(modelA.segments.size() >= 3);
        assert(modelB.segments.size() > modelA.segments.size());
        assert(modelA.segments.back().label == "start");
        assert(modelB.segments.back().label == "counter");
        std::cout << "Test 2 PASSED: nested path updates\n";
        passed++;
    }

    // Test 3: click navigates to node
    {
        auto model = buildBreadcrumbBar(&module, var, theme);
        const ASTNode* clicked = navigateBreadcrumbClick(model, 1); // likely class
        assert(clicked != nullptr);
        std::cout << "Test 3 PASSED: click navigates to node\n";
        passed++;
    }

    // Test 4: dropdown shows siblings
    {
        auto model = buildBreadcrumbBar(&module, start, theme);
        bool foundMethodSegment = false;
        for (const auto& seg : model.segments) {
            if (seg.label == "start") {
                auto names = breadcrumbSiblingNames(seg);
                assert(names.size() == 2);
                bool hasStop = false;
                for (const auto& n : names) {
                    if (n == "stop") hasStop = true;
                }
                assert(hasStop);
                foundMethodSegment = true;
            }
        }
        assert(foundMethodSegment);
        std::cout << "Test 4 PASSED: siblings shown\n";
        passed++;
    }

    // Test 5: empty file shows module only
    {
        Module empty("m2", "empty_mod", "python");
        auto model = buildBreadcrumbBar(&empty, nullptr, theme);
        assert(model.segments.size() == 1);
        assert(model.segments[0].label == "empty_mod");
        std::cout << "Test 5 PASSED: empty file module only\n";
        passed++;
    }

    // Test 6: deep nesting renders with overflow signal
    {
        auto* deep1 = new Function("d1", "d1");
        auto* deep2 = new Function("d2", "d2");
        auto* deep3 = new Function("d3", "d3");
        auto* deep4 = new Function("d4", "d4");
        deep1->addChild("body", deep2);
        deep2->addChild("body", deep3);
        deep3->addChild("body", deep4);
        module.addChild("functions", deep1);
        auto model = buildBreadcrumbBar(&module, deep4, theme, 4);
        assert(model.segments.size() >= 5);
        assert(model.overflow);
        std::cout << "Test 6 PASSED: deep nesting overflow handled\n";
        passed++;
    }

    // Test 7: breadcrumb uses theme colors
    {
        auto model = buildBreadcrumbBar(&module, start, theme);
        assert(model.bgColor.toHex() == theme.bgPanel.toHex());
        assert(model.textColor.toHex() == theme.text.toHex());
        assert(model.borderColor.toHex() == theme.border.toHex());
        std::cout << "Test 7 PASSED: theme colors used\n";
        passed++;
    }

    // Test 8: annotation nodes appear in path
    {
        auto model = buildBreadcrumbBar(&module, anno, theme);
        assert(!model.segments.empty());
        bool foundAnnotation = false;
        for (const auto& seg : model.segments) {
            if (seg.label == "IntentAnnotation" || seg.label == "log") {
                foundAnnotation = true;
            }
        }
        assert(foundAnnotation);
        std::cout << "Test 8 PASSED: annotation appears in path\n";
        passed++;
    }

    // Test 9: breadcrumb labels use compact names for class/function
    {
        auto model = buildBreadcrumbBar(&module, start, theme);
        bool hasClass = false, hasMethod = false;
        for (const auto& seg : model.segments) {
            if (seg.label == "Engine") hasClass = true;
            if (seg.label == "start") hasMethod = true;
        }
        assert(hasClass && hasMethod);
        std::cout << "Test 9 PASSED: compact node names\n";
        passed++;
    }

    // Test 10: invalid click index returns null
    {
        auto model = buildBreadcrumbBar(&module, start, theme);
        assert(navigateBreadcrumbClick(model, 999) == nullptr);
        std::cout << "Test 10 PASSED: invalid click safe\n";
        passed++;
    }

    // Test 11: sibling list empty for root segment
    {
        auto model = buildBreadcrumbBar(&module, start, theme);
        assert(!model.segments.empty());
        auto rootNames = breadcrumbSiblingNames(model.segments[0]);
        assert(rootNames.empty());
        std::cout << "Test 11 PASSED: root has no siblings\n";
        passed++;
    }

    // Test 12: cursor outside module falls back to module only
    {
        Function foreign("f1", "foreign");
        auto model = buildBreadcrumbBar(&module, &foreign, theme);
        assert(model.segments.size() == 1);
        assert(model.segments[0].label == "app");
        std::cout << "Test 12 PASSED: foreign cursor fallback\n";
        passed++;
    }

    std::cout << "\nResults: " << passed << "/12\n";
    assert(passed == 12);
    return 0;
}
