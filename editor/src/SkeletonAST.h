#pragma once
// Step 316: Skeleton AST Support
//
// Modules and functions with annotations but no implementation — the
// architect's project specification. A skeleton AST is the project
// model before code exists.

#include "ast/ASTNode.h"
#include "ast/Module.h"
#include "ast/Function.h"
#include "ast/Variable.h"
#include "ast/Parameter.h"
#include "ast/Annotation.h"
#include "ast/ClassDeclaration.h"
#include <string>
#include <vector>
#include <memory>

// --- SkeletonTask: a flat task entry derived from a skeleton node ---

struct SkeletonTask {
    std::string nodeId;
    std::string nodeName;
    std::string nodeType;           // "Function", "ClassDeclaration", etc.
    std::string contextWidth;       // from @ContextWidth or "local"
    std::string automatability;     // from @Automatability or "llm"
    std::string priority;           // from @Priority or "medium"
    bool reviewRequired = false;    // from @Review or inferred from @Ambiguity
    std::string status;             // from @ImplementationStatus or "skeleton"
    std::vector<std::string> dependencies;  // from @Priority.blockedBy
};

// --- Skeleton creation helpers ---

inline Module* createSkeletonModule(const std::string& name,
                                    const std::string& language) {
    auto* mod = new Module("skel_" + name, name, language);
    return mod;
}

inline Function* addSkeletonFunction(Module* mod,
                                     const std::string& name,
                                     const std::vector<std::string>& paramNames,
                                     const std::string& returnType,
                                     const std::vector<ASTNode*>& annotations = {}) {
    static int fnCounter = 0;
    auto* fn = new Function("skfn_" + std::to_string(++fnCounter), name);
    // returnType stored as annotation or metadata (Function has no returnType field)

    for (const auto& pname : paramNames) {
        static int pCounter = 0;
        auto* p = new Parameter();
        p->id = "skp_" + std::to_string(++pCounter);
        p->name = pname;
        fn->addChild("parameters", p);
    }

    for (auto* anno : annotations) {
        fn->addChild("annotations", anno);
    }

    // Add ImplementationStatus(skeleton) if not already present
    bool hasStatus = false;
    for (auto* a : fn->getChildren("annotations")) {
        if (a->conceptType == "ImplementationStatusAnnotation") {
            hasStatus = true;
            break;
        }
    }
    if (!hasStatus) {
        auto* implStatus = new ImplementationStatusAnnotation();
        static int isCounter = 0;
        implStatus->id = "skis_" + std::to_string(++isCounter);
        implStatus->status = "skeleton";
        fn->addChild("annotations", implStatus);
    }

    mod->addChild("functions", fn);
    return fn;
}

inline ClassDeclaration* addSkeletonClass(Module* mod,
                                          const std::string& name,
                                          const std::vector<std::string>& methodNames,
                                          const std::vector<std::string>& fieldNames = {}) {
    static int clsCounter = 0;
    auto* cls = new ClassDeclaration();
    cls->id = "skcls_" + std::to_string(++clsCounter);
    cls->name = name;
    cls->conceptType = "ClassDeclaration";

    for (const auto& mname : methodNames) {
        static int mdCounter = 0;
        auto* method = new MethodDeclaration("skmd_" + std::to_string(++mdCounter), mname);
        method->className = name;
        // Mark as skeleton
        auto* implStatus = new ImplementationStatusAnnotation();
        static int misCounter = 0;
        implStatus->id = "skmis_" + std::to_string(++misCounter);
        implStatus->status = "skeleton";
        method->addChild("annotations", implStatus);
        cls->addChild("methods", method);
    }

    for (const auto& fname : fieldNames) {
        auto* field = new Variable("skfld_" + fname, fname);
        cls->addChild("fields", field);
    }

    mod->addChild("functions", cls);  // Module stores top-level items as "functions"
    return cls;
}

// --- Skeleton detection ---

inline bool isSkeletonNode(const ASTNode* node) {
    if (!node) return false;
    auto annos = node->getChildren("annotations");
    for (const auto* a : annos) {
        if (a->conceptType == "ImplementationStatusAnnotation") {
            auto* is = static_cast<const ImplementationStatusAnnotation*>(a);
            return is->status == "skeleton";
        }
    }
    // Also skeleton if function has empty body
    if (node->conceptType == "Function" || node->conceptType == "AsyncFunction" ||
        node->conceptType == "MethodDeclaration") {
        auto body = node->getChildren("body");
        return body.empty();
    }
    return false;
}

// --- Skeleton summary ---

struct SkeletonSummary {
    int totalNodes = 0;
    int skeletonNodes = 0;
    int implementedNodes = 0;
    std::map<std::string, int> byAnnotationType;  // conceptType → count
};

inline void countSkeletonNodes(const ASTNode* node, SkeletonSummary& summary) {
    if (!node) return;

    bool isFnLike = (node->conceptType == "Function" ||
                     node->conceptType == "AsyncFunction" ||
                     node->conceptType == "MethodDeclaration" ||
                     node->conceptType == "ClassDeclaration");

    if (isFnLike) {
        summary.totalNodes++;
        if (isSkeletonNode(node)) {
            summary.skeletonNodes++;
        } else {
            summary.implementedNodes++;
        }
    }

    // Count annotation types
    auto annos = node->getChildren("annotations");
    for (const auto* a : annos) {
        summary.byAnnotationType[a->conceptType]++;
    }

    for (const auto* child : node->allChildren()) {
        countSkeletonNodes(child, summary);
    }
}

inline SkeletonSummary getSkeletonSummary(const ASTNode* module) {
    SkeletonSummary summary;
    countSkeletonNodes(module, summary);
    return summary;
}

// --- Task list generation ---

inline void extractRoutingAnnotations(const ASTNode* node, SkeletonTask& task) {
    auto annos = node->getChildren("annotations");
    for (const auto* a : annos) {
        if (a->conceptType == "ContextWidthAnnotation") {
            task.contextWidth = static_cast<const ContextWidthAnnotation*>(a)->width;
        }
        else if (a->conceptType == "AutomatabilityAnnotation") {
            task.automatability = static_cast<const AutomatabilityAnnotation*>(a)->strategy;
        }
        else if (a->conceptType == "PriorityAnnotation") {
            auto* pa = static_cast<const PriorityAnnotation*>(a);
            task.priority = pa->level;
            task.dependencies = pa->blockedBy;
        }
        else if (a->conceptType == "ReviewAnnotation") {
            task.reviewRequired = static_cast<const ReviewAnnotation*>(a)->required;
        }
        else if (a->conceptType == "ImplementationStatusAnnotation") {
            task.status = static_cast<const ImplementationStatusAnnotation*>(a)->status;
        }
        else if (a->conceptType == "AmbiguityAnnotation") {
            auto* amb = static_cast<const AmbiguityAnnotation*>(a);
            if (amb->level == "high" || amb->level == "medium") {
                task.reviewRequired = true;
            }
        }
    }

    // Defaults
    if (task.contextWidth.empty()) task.contextWidth = "local";
    if (task.automatability.empty()) task.automatability = "llm";
    if (task.priority.empty()) task.priority = "medium";
    if (task.status.empty()) task.status = "skeleton";
}

inline std::string getTaskNodeName(const ASTNode* node) {
    if (node->conceptType == "Function" || node->conceptType == "AsyncFunction" ||
        node->conceptType == "MethodDeclaration") {
        return static_cast<const Function*>(node)->name;
    }
    if (node->conceptType == "ClassDeclaration") {
        return static_cast<const ClassDeclaration*>(node)->name;
    }
    return node->id;
}

inline void collectSkeletonTasks(const ASTNode* node,
                                 std::vector<SkeletonTask>& tasks) {
    if (!node) return;

    bool isFnLike = (node->conceptType == "Function" ||
                     node->conceptType == "AsyncFunction" ||
                     node->conceptType == "MethodDeclaration" ||
                     node->conceptType == "ClassDeclaration");

    if (isFnLike) {
        SkeletonTask task;
        task.nodeId = node->id;
        task.nodeName = getTaskNodeName(node);
        task.nodeType = node->conceptType;
        extractRoutingAnnotations(node, task);
        tasks.push_back(task);
    }

    for (const auto* child : node->allChildren()) {
        collectSkeletonTasks(child, tasks);
    }
}

inline std::vector<SkeletonTask> skeletonToTaskList(const ASTNode* module) {
    std::vector<SkeletonTask> tasks;
    collectSkeletonTasks(module, tasks);
    return tasks;
}
