#pragma once
// Step 66: Cross-language memory projection
//
// CrossLanguageProjector: deep-copies an AST Module, changing its
// targetLanguage while preserving (and optionally adapting) memory
// annotations so that code generators can emit language-appropriate
// constructs (e.g. shared_ptr for @Reclaim(Tracing) in C++).

#include <memory>
#include <string>
#include <vector>
#include "ast/ASTNode.h"
#include "ast/Module.h"
#include "ast/Function.h"
#include "ast/Variable.h"
#include "ast/Parameter.h"
#include "ast/Type.h"
#include "ast/Annotation.h"

class CrossLanguageProjector {
public:
    // Project an AST from source language to target language.
    // Returns a new Module with annotations adapted for the target.
    std::unique_ptr<Module> project(const Module* source,
                                    const std::string& targetLanguage) const {
        if (!source) return nullptr;

        auto mod = std::make_unique<Module>(
            source->id + "_proj", source->name, targetLanguage);

        // Copy functions with annotations and body
        for (auto* child : source->getChildren("functions")) {
            if (child->conceptType == "Function") {
                auto* srcFn = static_cast<const Function*>(child);
                Function* newFn = cloneFunction(srcFn);
                mod->addChild("functions", newFn);
            }
        }

        // Copy module-level variables
        for (auto* child : source->getChildren("variables")) {
            if (child->conceptType == "Variable") {
                auto* srcVar = static_cast<const Variable*>(child);
                Variable* newVar = new Variable(srcVar->id + "_proj", srcVar->name);
                mod->addChild("variables", newVar);
            }
        }

        // Copy module-level annotations
        for (auto* anno : source->getChildren("annotations")) {
            ASTNode* newAnno = cloneAnnotation(anno);
            if (newAnno) mod->addChild("annotations", newAnno);
        }

        return mod;
    }

    // Check if all annotation types from the original survive in the projected AST.
    bool annotationsPreserved(const Module* original,
                              const Module* projected) const {
        auto origTypes = collectAnnotationTypes(original);
        auto projTypes = collectAnnotationTypes(projected);

        for (const auto& type : origTypes) {
            bool found = false;
            for (const auto& pt : projTypes) {
                if (pt == type) { found = true; break; }
            }
            if (!found) return false;
        }
        return true;
    }

private:
    Function* cloneFunction(const Function* src) const {
        Function* fn = new Function(src->id + "_proj", src->name);

        // Copy annotations (preserve all, even non-native to target)
        for (auto* anno : src->getChildren("annotations")) {
            ASTNode* newAnno = cloneAnnotation(anno);
            if (newAnno) fn->addChild("annotations", newAnno);
        }

        // Copy body nodes
        for (auto* child : src->getChildren("body")) {
            ASTNode* newChild = cloneNode(child);
            if (newChild) fn->addChild("body", newChild);
        }

        // Copy parameters
        for (auto* child : src->getChildren("parameters")) {
            ASTNode* newChild = cloneNode(child);
            if (newChild) fn->addChild("parameters", newChild);
        }

        // Copy return type
        auto* retType = src->getChild("returnType");
        if (retType) {
            ASTNode* newRet = cloneNode(retType);
            if (newRet) fn->setChild("returnType", newRet);
        }

        return fn;
    }

    ASTNode* cloneAnnotation(const ASTNode* anno) const {
        if (!anno) return nullptr;

        const auto& ct = anno->conceptType;

        if (ct == "ReclaimAnnotation") {
            auto* src = static_cast<const ReclaimAnnotation*>(anno);
            auto* a = new ReclaimAnnotation(src->id + "_proj", src->strategy);
            a->reclaimPattern = src->reclaimPattern;
            return a;
        }
        if (ct == "DeallocateAnnotation") {
            auto* src = static_cast<const DeallocateAnnotation*>(anno);
            auto* a = new DeallocateAnnotation(src->id + "_proj", src->strategy);
            a->deallocateLocation = src->deallocateLocation;
            a->owner = src->owner;
            return a;
        }
        if (ct == "LifetimeAnnotation") {
            auto* src = static_cast<const LifetimeAnnotation*>(anno);
            auto* a = new LifetimeAnnotation(src->id + "_proj", src->strategy);
            a->lifetimeScope = src->lifetimeScope;
            return a;
        }
        if (ct == "OwnerAnnotation") {
            auto* src = static_cast<const OwnerAnnotation*>(anno);
            auto* a = new OwnerAnnotation(src->id + "_proj", src->strategy);
            a->ownerType = src->ownerType;
            return a;
        }
        if (ct == "AllocateAnnotation") {
            auto* src = static_cast<const AllocateAnnotation*>(anno);
            auto* a = new AllocateAnnotation(src->id + "_proj", src->strategy);
            a->allocationPattern = src->allocationPattern;
            return a;
        }
        if (ct == "DerefStrategy") {
            auto* src = static_cast<const DerefStrategy*>(anno);
            auto* a = new DerefStrategy(src->id + "_proj", src->strategy);
            a->derefLocation = src->derefLocation;
            a->owner = src->owner;
            return a;
        }
        if (ct == "OptimizationLock") {
            auto* a = new OptimizationLock();
            auto* src = static_cast<const OptimizationLock*>(anno);
            a->id = src->id + "_proj";
            a->lockedBy = src->lockedBy;
            a->lockReason = src->lockReason;
            a->lockLevel = src->lockLevel;
            return a;
        }
        if (ct == "LangSpecific") {
            auto* a = new LangSpecific();
            auto* src = static_cast<const LangSpecific*>(anno);
            a->id = src->id + "_proj";
            a->language = src->language;
            a->idiomType = src->idiomType;
            a->rawSyntax = src->rawSyntax;
            return a;
        }

        return nullptr;
    }

    ASTNode* cloneNode(const ASTNode* node) const {
        if (!node) return nullptr;

        const auto& ct = node->conceptType;

        if (ct == "Variable") {
            auto* src = static_cast<const Variable*>(node);
            auto* v = new Variable(src->id + "_proj", src->name);
            // Clone variable annotations
            for (auto* anno : src->getChildren("annotations")) {
                ASTNode* a = cloneAnnotation(anno);
                if (a) v->addChild("annotations", a);
            }
            // Clone type child
            auto* typeChild = src->getChild("type");
            if (typeChild) {
                ASTNode* t = cloneNode(typeChild);
                if (t) v->setChild("type", t);
            }
            // Clone initializer
            auto* init = src->getChild("initializer");
            if (init) {
                ASTNode* i = cloneNode(init);
                if (i) v->setChild("initializer", i);
            }
            return v;
        }
        if (ct == "Parameter") {
            auto* src = static_cast<const Parameter*>(node);
            auto* p = new Parameter(src->id + "_proj", src->name);
            auto* typeChild = src->getChild("type");
            if (typeChild) {
                ASTNode* t = cloneNode(typeChild);
                if (t) p->setChild("type", t);
            }
            return p;
        }
        if (ct == "PrimitiveType") {
            auto* src = static_cast<const PrimitiveType*>(node);
            return new PrimitiveType(src->id + "_proj", src->kind);
        }

        // Fallback: annotations
        if (ct.find("Annotation") != std::string::npos ||
            ct == "DerefStrategy" || ct == "OptimizationLock" ||
            ct == "LangSpecific") {
            return cloneAnnotation(node);
        }

        return nullptr;
    }

    // Recursively collect all annotation conceptTypes from a tree
    std::vector<std::string> collectAnnotationTypes(const ASTNode* node) const {
        std::vector<std::string> types;
        if (!node) return types;
        collectAnnotationTypesImpl(node, types);
        return types;
    }

    void collectAnnotationTypesImpl(const ASTNode* node,
                                    std::vector<std::string>& types) const {
        if (!node) return;

        // If this is an annotation node, record its type
        const auto& ct = node->conceptType;
        if (ct == "ReclaimAnnotation" || ct == "DeallocateAnnotation" ||
            ct == "LifetimeAnnotation" || ct == "OwnerAnnotation" ||
            ct == "AllocateAnnotation" || ct == "DerefStrategy" ||
            ct == "OptimizationLock" || ct == "LangSpecific") {
            types.push_back(ct);
        }

        // Recurse into all children
        for (auto* child : node->allChildren()) {
            collectAnnotationTypesImpl(child, types);
        }
    }
};
