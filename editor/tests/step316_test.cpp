// Step 316: Skeleton AST Support (12 tests)
// createSkeletonModule, addSkeletonFunction, addSkeletonClass,
// isSkeletonNode, getSkeletonSummary, skeletonToTaskList.

#include "SkeletonAST.h"
#include "ast/Module.h"
#include "ast/Function.h"
#include "ast/Annotation.h"
#include "ast/ClassDeclaration.h"
#include "ast/Statement.h"
#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <algorithm>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

// 1. Create skeleton module
void test_create_skeleton_module() {
    TEST(create_skeleton_module);
    auto* mod = createSkeletonModule("myapp", "python");
    CHECK(mod != nullptr, "Module created");
    CHECK(mod->name == "myapp", "Module name");
    CHECK(mod->targetLanguage == "python", "Module language");
    CHECK(mod->conceptType == "Module", "Module conceptType");
    delete mod;
    PASS();
}

// 2. Add skeleton function with parameters
void test_add_skeleton_function() {
    TEST(add_skeleton_function);
    auto* mod = createSkeletonModule("myapp", "python");
    auto* fn = addSkeletonFunction(mod, "process", {"data", "config"}, "Result");
    CHECK(fn != nullptr, "Function created");
    CHECK(fn->name == "process", "Function name");
    CHECK(fn->name == "process", "Function name matches");
    auto params = fn->getChildren("parameters");
    CHECK(params.size() == 2, "2 parameters, got: " + std::to_string(params.size()));
    // Should auto-add ImplementationStatus(skeleton)
    auto annos = fn->getChildren("annotations");
    bool hasImplStatus = false;
    for (auto* a : annos) {
        if (a->conceptType == "ImplementationStatusAnnotation") {
            auto* is = static_cast<ImplementationStatusAnnotation*>(a);
            CHECK(is->status == "skeleton", "Auto-added skeleton status");
            hasImplStatus = true;
        }
    }
    CHECK(hasImplStatus, "ImplementationStatus auto-added");
    delete mod;
    PASS();
}

// 3. Add skeleton function with explicit annotations
void test_skeleton_function_with_annotations() {
    TEST(skeleton_function_with_annotations);
    auto* mod = createSkeletonModule("myapp", "python");

    auto* cw = new ContextWidthAnnotation();
    cw->id = "cw1"; cw->width = "project";
    auto* aa = new AutomatabilityAnnotation();
    aa->id = "aa1"; aa->strategy = "llm"; aa->confidence = 0.9;

    auto* fn = addSkeletonFunction(mod, "analyze", {"input"}, "Analysis",
                                   {cw, aa});
    auto annos = fn->getChildren("annotations");
    // Should have: ContextWidth, Automatability, ImplementationStatus(skeleton)
    CHECK(annos.size() == 3, "3 annotations, got: " + std::to_string(annos.size()));
    delete mod;
    PASS();
}

// 4. Add skeleton class
void test_add_skeleton_class() {
    TEST(add_skeleton_class);
    auto* mod = createSkeletonModule("myapp", "python");
    auto* cls = addSkeletonClass(mod, "UserService",
                                 {"create", "delete", "find"},
                                 {"db", "cache"});
    CHECK(cls != nullptr, "Class created");
    CHECK(cls->name == "UserService", "Class name");
    auto methods = cls->getChildren("methods");
    CHECK(methods.size() == 3, "3 methods");
    auto fields = cls->getChildren("fields");
    CHECK(fields.size() == 2, "2 fields");
    // Each method should have ImplementationStatus(skeleton)
    for (auto* m : methods) {
        CHECK(isSkeletonNode(m), "Method " + m->id + " is skeleton");
    }
    delete mod;
    PASS();
}

// 5. isSkeletonNode detection
void test_is_skeleton_node() {
    TEST(is_skeleton_node);
    // Skeleton: has ImplementationStatus(skeleton)
    auto fn1 = std::make_unique<Function>("fn1", "empty_func");
    auto* is = new ImplementationStatusAnnotation();
    is->id = "is1"; is->status = "skeleton";
    fn1->addChild("annotations", is);
    CHECK(isSkeletonNode(fn1.get()), "Explicit skeleton annotation");

    // Skeleton: empty body
    auto fn2 = std::make_unique<Function>("fn2", "no_body");
    CHECK(isSkeletonNode(fn2.get()), "Empty body = skeleton");

    // Not skeleton: has body
    auto fn3 = std::make_unique<Function>("fn3", "has_body");
    auto* ret = new Return();
    ret->id = "r1";
    fn3->addChild("body", ret);
    CHECK(!isSkeletonNode(fn3.get()), "Has body = not skeleton");

    // Not skeleton: status = "complete"
    auto fn4 = std::make_unique<Function>("fn4", "complete_func");
    auto* is2 = new ImplementationStatusAnnotation();
    is2->id = "is2"; is2->status = "complete";
    fn4->addChild("annotations", is2);
    auto* ret2 = new Return();
    ret2->id = "r2";
    fn4->addChild("body", ret2);
    CHECK(!isSkeletonNode(fn4.get()), "Complete status = not skeleton");
    PASS();
}

// 6. Skeleton summary counts
void test_skeleton_summary() {
    TEST(skeleton_summary);
    auto* mod = createSkeletonModule("myapp", "python");
    addSkeletonFunction(mod, "task_a", {}, "void");
    addSkeletonFunction(mod, "task_b", {}, "void");

    // Add an implemented function
    auto* implFn = new Function("fn3", "implemented");
    auto* ret = new Return();
    ret->id = "r1";
    implFn->addChild("body", ret);
    auto* completeStatus = new ImplementationStatusAnnotation();
    completeStatus->id = "cis1"; completeStatus->status = "complete";
    implFn->addChild("annotations", completeStatus);
    mod->addChild("functions", implFn);

    auto summary = getSkeletonSummary(mod);
    CHECK(summary.totalNodes == 3, "3 total, got: " + std::to_string(summary.totalNodes));
    CHECK(summary.skeletonNodes == 2, "2 skeleton, got: " + std::to_string(summary.skeletonNodes));
    CHECK(summary.implementedNodes == 1, "1 implemented, got: " + std::to_string(summary.implementedNodes));
    CHECK(summary.byAnnotationType.count("ImplementationStatusAnnotation") > 0, "Has impl status count");
    delete mod;
    PASS();
}

// 7. Task list generation
void test_task_list_generation() {
    TEST(task_list_generation);
    auto* mod = createSkeletonModule("myapp", "python");

    auto* cw = new ContextWidthAnnotation();
    cw->id = "cw1"; cw->width = "file";
    auto* aa = new AutomatabilityAnnotation();
    aa->id = "aa1"; aa->strategy = "template";
    addSkeletonFunction(mod, "simple_getter", {}, "int", {cw, aa});

    auto* cw2 = new ContextWidthAnnotation();
    cw2->id = "cw2"; cw2->width = "project";
    auto* aa2 = new AutomatabilityAnnotation();
    aa2->id = "aa2"; aa2->strategy = "llm";
    auto* pa = new PriorityAnnotation();
    pa->id = "pa1"; pa->level = "critical";
    pa->blockedBy.push_back("simple_getter");
    addSkeletonFunction(mod, "complex_analysis", {"data"}, "Report", {cw2, aa2, pa});

    auto tasks = skeletonToTaskList(mod);
    CHECK(tasks.size() == 2, "2 tasks, got: " + std::to_string(tasks.size()));

    auto& t1 = tasks[0];
    CHECK(t1.nodeName == "simple_getter", "Task 1 name");
    CHECK(t1.contextWidth == "file", "Task 1 context width");
    CHECK(t1.automatability == "template", "Task 1 automatability");

    auto& t2 = tasks[1];
    CHECK(t2.nodeName == "complex_analysis", "Task 2 name");
    CHECK(t2.contextWidth == "project", "Task 2 context width");
    CHECK(t2.automatability == "llm", "Task 2 automatability");
    CHECK(t2.priority == "critical", "Task 2 priority");
    CHECK(t2.dependencies.size() == 1, "Task 2 has 1 dependency");
    CHECK(t2.dependencies[0] == "simple_getter", "Task 2 blocked by simple_getter");

    delete mod;
    PASS();
}

// 8. Annotation extraction — defaults
void test_annotation_defaults() {
    TEST(annotation_defaults);
    auto* mod = createSkeletonModule("myapp", "python");
    addSkeletonFunction(mod, "bare_function", {}, "void");

    auto tasks = skeletonToTaskList(mod);
    CHECK(tasks.size() == 1, "1 task");
    auto& t = tasks[0];
    CHECK(t.contextWidth == "local", "Default contextWidth = local");
    CHECK(t.automatability == "llm", "Default automatability = llm");
    CHECK(t.priority == "medium", "Default priority = medium");
    CHECK(t.reviewRequired == false, "Default reviewRequired = false");
    CHECK(t.status == "skeleton", "Default status = skeleton");
    delete mod;
    PASS();
}

// 9. Empty module handling
void test_empty_module() {
    TEST(empty_module);
    auto* mod = createSkeletonModule("empty", "python");
    auto summary = getSkeletonSummary(mod);
    CHECK(summary.totalNodes == 0, "0 total nodes");
    CHECK(summary.skeletonNodes == 0, "0 skeleton nodes");

    auto tasks = skeletonToTaskList(mod);
    CHECK(tasks.empty(), "0 tasks");
    delete mod;
    PASS();
}

// 10. Dependency ordering from @Priority.blockedBy
void test_dependency_ordering() {
    TEST(dependency_ordering);
    auto* mod = createSkeletonModule("myapp", "python");

    // Task C depends on A and B
    auto* paC = new PriorityAnnotation();
    paC->id = "paC"; paC->level = "low";
    paC->blockedBy.push_back("task_a");
    paC->blockedBy.push_back("task_b");
    addSkeletonFunction(mod, "task_c", {}, "void", {paC});
    addSkeletonFunction(mod, "task_a", {}, "void");
    addSkeletonFunction(mod, "task_b", {}, "void");

    auto tasks = skeletonToTaskList(mod);
    CHECK(tasks.size() == 3, "3 tasks");

    // Find task_c and verify dependencies
    for (const auto& t : tasks) {
        if (t.nodeName == "task_c") {
            CHECK(t.dependencies.size() == 2, "task_c has 2 deps");
            CHECK(t.dependencies[0] == "task_a", "dep[0] = task_a");
            CHECK(t.dependencies[1] == "task_b", "dep[1] = task_b");
        }
    }
    delete mod;
    PASS();
}

// 11. Review inferred from high ambiguity
void test_review_from_ambiguity() {
    TEST(review_from_ambiguity);
    auto* mod = createSkeletonModule("myapp", "python");

    auto* amb = new AmbiguityAnnotation();
    amb->id = "amb1"; amb->level = "high";
    amb->description = "Requirements are unclear";
    addSkeletonFunction(mod, "unclear_task", {}, "void", {amb});

    auto tasks = skeletonToTaskList(mod);
    CHECK(tasks.size() == 1, "1 task");
    CHECK(tasks[0].reviewRequired == true, "High ambiguity → review required");
    delete mod;
    PASS();
}

// 12. Class methods appear in task list
void test_class_methods_in_tasks() {
    TEST(class_methods_in_tasks);
    auto* mod = createSkeletonModule("myapp", "python");
    addSkeletonClass(mod, "Service", {"init", "start", "stop"}, {});

    auto tasks = skeletonToTaskList(mod);
    // Should have: ClassDeclaration + 3 MethodDeclarations = 4 tasks
    CHECK(tasks.size() == 4, "4 tasks (1 class + 3 methods), got: " + std::to_string(tasks.size()));

    bool hasClass = false;
    int methodCount = 0;
    for (const auto& t : tasks) {
        if (t.nodeType == "ClassDeclaration") hasClass = true;
        if (t.nodeType == "MethodDeclaration") methodCount++;
    }
    CHECK(hasClass, "Has class task");
    CHECK(methodCount == 3, "3 method tasks");
    delete mod;
    PASS();
}

int main() {
    std::cout << "Step 316: Skeleton AST Support\n";
    test_create_skeleton_module();
    test_add_skeleton_function();
    test_skeleton_function_with_annotations();
    test_add_skeleton_class();
    test_is_skeleton_node();
    test_skeleton_summary();
    test_task_list_generation();
    test_annotation_defaults();
    test_empty_module();
    test_dependency_ordering();
    test_review_from_ambiguity();
    test_class_methods_in_tasks();
    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed > 0 ? 1 : 0;
}
